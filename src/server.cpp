#include <iostream>            
#include <cstring>              
#include <cstdlib>             // Incluye funciones para la conversión de datos y la gestión de la memoria.
#include <unistd.h>            // Incluye funciones para el manejo de llamadas al sistema.
#include <arpa/inet.h>         // Incluye funciones para manejar direcciones de red.
#include <pthread.h>           // Incluye funciones para manejo de hilos (threads).
#include "request_handler.h"    

#define PORT 1708              // Define el puerto en el que el servidor escuchará.
#define BACKLOG 10             // Define el número máximo de conexiones en espera.
#define BUF_SIZE 1024          // Define el tamaño del búfer para leer solicitudes.

void *handle_request(void *client_socket) {
    int sock = *(int *)client_socket; // Cast de puntero a entero para obtener el socket del cliente.
    free(client_socket); // Libera la memoria del puntero al socket del cliente.
    
    char buffer[BUF_SIZE]; // Crea un búfer para almacenar la solicitud del cliente.
    read(sock, buffer, sizeof(buffer)); // Lee la solicitud en el búfer.
    std::cout << "Request received:\n" << buffer << std::endl; // Muestra la solicitud recibida en la consola.

    std::string session_id; // Variable para almacenar el ID de sesión.
    char *cookie_header = strstr(buffer, "Cookie: "); // Busca el encabezado de la cookie en la solicitud.
    
    // Verifica si hay una cookie en la solicitud.
    if (cookie_header) {
        session_id = strstr(cookie_header, "=") + 1; // Extrae el valor de la cookie.
        char *end = strstr(const_cast<char*>(session_id.c_str()), ";"); // Busca el final del valor de la cookie.
        if (end) {
            *end = '\0'; // Si se encuentra un final, termina la cadena.
        }
    } else {
        session_id = "new_session_id"; // ID de sesión por defecto si no hay cookie.
    }

    // Procesa la solicitud y genera una respuesta.
    std::string response = process_request(buffer, session_id.c_str());

    // Si no se recibió un session_id, genera uno nuevo y envía el encabezado de la cookie.
    if (session_id == "new_session_id") {
        std::string new_session_id = generate_session_id(); // Genera un nuevo ID de sesión.
        std::string cookie_response = "HTTP/1.1 200 OK\r\nSet-Cookie: session_id=" + new_session_id + "\r\n"; // Crea la respuesta con la cookie.
        cookie_response += "Content-Type: text/plain\r\n\r\n"; // Establece el tipo de contenido.
        cookie_response += response;  // Agrega el cuerpo de la respuesta.
        write(sock, cookie_response.c_str(), cookie_response.size()); // Envía la respuesta al cliente.
    } else {
        // Si ya existe el session_id, responde normalmente.
        write(sock, response.c_str(), response.size()); // Envía la respuesta al cliente.
    }

    close(sock); // Cierra el socket del cliente.
    return nullptr; // Retorna nulo al finalizar el manejo de la solicitud.
}

int main() {
    int server_fd, *client_socket; // Declara variables para el socket del servidor y del cliente.
    struct sockaddr_in address; // Estructura para almacenar la dirección del servidor.
    int addrlen = sizeof(address); // Variable para la longitud de la dirección.

    // Crea un socket.
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed"); // Muestra un error si el socket no se puede crear.
        exit(EXIT_FAILURE); // Finaliza el programa si hay un error.
    }

    // Configura la estructura de la dirección del servidor.
    address.sin_family = AF_INET; // Usa la familia de direcciones IPv4.
    address.sin_addr.s_addr = INADDR_ANY; // Permite conexiones desde cualquier dirección.
    address.sin_port = htons(PORT); // Establece el puerto del servidor.

    // Asocia el socket a la dirección y puerto.
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed"); // Muestra un error si la asociación falla.
        exit(EXIT_FAILURE); // Finaliza el programa si hay un error.
    }

    // Escucha las conexiones entrantes.
    if (listen(server_fd, BACKLOG) < 0) {
        perror("Listen failed"); // Muestra un error si la escucha falla.
        exit(EXIT_FAILURE); // Finaliza el programa si hay un error.
    }

    std::cout << "Server listening on port " << PORT << std::endl; // Muestra un mensaje indicando que el servidor está escuchando.

    // Bucle principal para aceptar conexiones entrantes.
    while (true) {
        client_socket = (int *)malloc(sizeof(int)); // Reserva memoria para el socket del cliente.
        *client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen); // Acepta una conexión entrante.
        if (*client_socket < 0) {
            perror("Accept failed"); // Muestra un error si la aceptación de la conexión falla.
            free(client_socket); // Libera la memoria del puntero del socket del cliente.
            continue; // Continúa al siguiente ciclo para aceptar más conexiones.
        }

        pthread_t thread_id; // Declara un identificador de hilo.
        pthread_create(&thread_id, nullptr, handle_request, client_socket); // Crea un nuevo hilo para manejar la solicitud del cliente.
        pthread_detach(thread_id); // Desvincula el hilo, permitiendo que se limpie automáticamente al finalizar.
    }

    return 0; 
}




