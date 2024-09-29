#include <iostream>
#include <cstring>
#include <cstdlib>             // Para conversiones y gestión de memoria.
#include <unistd.h>            // Para llamadas al sistema (e.g., close).
#include <arpa/inet.h>         // Para manejar direcciones de red.
#include <pthread.h>           // Para manejo de hilos.
#include "request_handler.h"   // Incluye las funciones que procesan las solicitudes.

#define PORT 1708              // Define el puerto en el que el servidor escuchará.
#define BACKLOG 10             // Número máximo de conexiones en espera.
#define BUF_SIZE 1024          // Tamaño del búfer para leer solicitudes.

// Función para manejar la solicitud de un cliente en un hilo separado.
void *handle_request(void *client_socket) {
    int sock = *(int *)client_socket; // Cast de puntero a entero para obtener el socket del cliente.
    free(client_socket);              // Libera la memoria del puntero al socket del cliente.

    char buffer[BUF_SIZE];            // Búfer para almacenar la solicitud del cliente.
    read(sock, buffer, sizeof(buffer)); // Lee la solicitud del cliente en el búfer.
    std::cout << "Request received:\n" << buffer << std::endl; // Muestra la solicitud recibida.

    std::string session_id;           // Variable para almacenar el ID de sesión.
    char *cookie_header = strstr(buffer, "Cookie: "); // Busca el encabezado de la cookie en la solicitud.

    // Verifica si hay una cookie en la solicitud.
    if (cookie_header) {
        session_id = strstr(cookie_header, "=") + 1;  // Extrae el valor de la cookie.
        char *end = strstr(const_cast<char*>(session_id.c_str()), ";"); // Busca el final del valor de la cookie.
        if (end) {
            *end = '\0'; // Termina la cadena en el punto final de la cookie.
        }
    } else {
        session_id = "new_session_id"; // Si no hay cookie, se define un nuevo session_id por defecto.
    }

    // Procesa la solicitud y genera una respuesta.
    std::string response = process_request(buffer, session_id.c_str());

    // Si no se recibió un session_id, genera uno nuevo y envía el encabezado de la cookie.
    if (session_id == "new_session_id") {
        std::string new_session_id = generate_session_id(); // Genera un nuevo ID de sesión.
        std::string cookie_response = "HTTP/1.1 200 OK\r\nSet-Cookie: session_id=" + new_session_id + "; Path=/; Max-Age=3600\r\n"; // Respuesta con cookie.
        cookie_response += "Content-Type: text/plain\r\n\r\n"; // Tipo de contenido de la respuesta.
        cookie_response += response;  // Agrega el cuerpo de la respuesta.
        write(sock, cookie_response.c_str(), cookie_response.size()); // Envía la respuesta con cookie al cliente.
    } else {
        // Si ya existe el session_id, responde normalmente.
        write(sock, response.c_str(), response.size()); // Envía la respuesta al cliente.
    }

    close(sock); // Cierra el socket del cliente.
    return nullptr; // Retorna nullptr al finalizar.
}

int main() {
    int server_fd, *client_socket;             // Variables para el socket del servidor y cliente.
    struct sockaddr_in address;                // Estructura para la dirección del servidor.
    int addrlen = sizeof(address);             // Longitud de la dirección.

    // Crea el socket del servidor.
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed"); // Muestra un error si la creación del socket falla.
        exit(EXIT_FAILURE);      // Finaliza el programa en caso de error.
    }

    // Configura la estructura de la dirección del servidor.
    address.sin_family = AF_INET;               // Familia de direcciones IPv4.
    address.sin_addr.s_addr = INADDR_ANY;       // Acepta conexiones desde cualquier dirección.
    address.sin_port = htons(PORT);             // Establece el puerto en el que el servidor escuchará.

    // Asocia el socket con la dirección y puerto.
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed"); // Error en caso de fallo en bind.
        exit(EXIT_FAILURE);    // Finaliza el programa.
    }

    // Configura el servidor para escuchar conexiones.
    if (listen(server_fd, BACKLOG) < 0) {
        perror("Listen failed"); // Error en caso de fallo en listen.
        exit(EXIT_FAILURE);      // Finaliza el programa.
    }

    std::cout << "Server listening on port " << PORT << std::endl; // Informa que el servidor está escuchando.

    // Bucle para aceptar conexiones entrantes.
    while (true) {
        client_socket = (int *)malloc(sizeof(int)); // Reserva memoria para el socket del cliente.
        *client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen); // Acepta una conexión entrante.

        if (*client_socket < 0) {
            perror("Accept failed"); // Error si falla la aceptación de la conexión.
            free(client_socket);     // Libera la memoria del socket en caso de error.
            continue;                // Continúa para aceptar la siguiente conexión.
        }

        pthread_t thread_id; // Declara el identificador del hilo.
        if (pthread_create(&thread_id, nullptr, handle_request, client_socket) != 0) {
            perror("Thread creation failed"); // Error si falla la creación del hilo.
            free(client_socket);              // Libera la memoria del socket del cliente.
        } else {
            pthread_detach(thread_id); // Desvincula el hilo para que se limpie automáticamente al terminar.
        }
    }

    return 0; // Finaliza el programa.
}
