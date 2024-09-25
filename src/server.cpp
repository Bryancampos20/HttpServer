#include <iostream>            // Biblioteca estándar para entrada y salida
#include <cstring>            // Biblioteca para manipulación de cadenas
#include <cstdlib>            // Biblioteca para funciones de conversión y gestión de memoria
#include <unistd.h>           // Biblioteca para funciones POSIX (como read y write)
#include <arpa/inet.h>        // Biblioteca para funciones de red (como socket y connect)
#include <pthread.h>          // Biblioteca para el manejo de hilos
#include "request_handler.h"   // Archivo de encabezado para el manejo de solicitudes HTTP

// Definiciones de constantes
#define PORT 8080               // Puerto en el que el servidor escucha
#define BACKLOG 10              // Número máximo de conexiones en espera
#define BUF_SIZE 1024           // Tamaño del búfer para la lectura de datos

/**
 * Función que maneja la solicitud de un cliente HTTP.
 * 
 * Esta función es ejecutada en un hilo separado para cada cliente. 
 * Lee la solicitud HTTP del cliente, procesa la solicitud y envía la respuesta.
 * 
 * @param client_socket Puntero al socket del cliente.
 * @return nullptr
 */
void *handle_request(void *client_socket) {
    int sock = *(int *)client_socket;  // Desreferencia el puntero al socket del cliente
    free(client_socket);                // Libera el puntero al socket
    char buffer[BUF_SIZE];              // Búfer para almacenar la solicitud
    read(sock, buffer, sizeof(buffer)); // Lee la solicitud del cliente
    std::cout << "Request received:\n" << buffer << std::endl; // Muestra la solicitud recibida

    std::string session_id;             // Variable para almacenar el ID de sesión
    char *cookie_header = strstr(buffer, "Cookie: "); // Busca el encabezado de cookies
    if (cookie_header) {
        session_id = strstr(cookie_header, "=") + 1; // Obtiene el ID de sesión
        char *end = strstr(const_cast<char*>(session_id.c_str()), ";");
        if (end) {
            *end = '\0';  // Finaliza la cadena del ID de sesión
        }
    } else {
        session_id = "new_session_id"; // ID de sesión por defecto
    }

    // Cambia de char* a std::string y procesa la solicitud
    std::string response = process_request(buffer, session_id.c_str());
    
    // Si se genera un nuevo ID de sesión, establece una cookie
    if (session_id == "new_session_id") {
        std::string new_session_id = generate_session_id(); 
        std::string cookie_response = "Set-Cookie: session_id=" + new_session_id + "\r\n";
        write(sock, cookie_response.c_str(), cookie_response.size()); // Envía la cookie al cliente
    }

    write(sock, response.c_str(), response.size()); // Envía la respuesta al cliente
    close(sock); // Cierra el socket del cliente
    return nullptr; // Termina el hilo
}

/**
 * Función principal del servidor HTTP.
 * 
 * Crea un socket, lo vincula a una dirección y puerto, y comienza a escuchar solicitudes de clientes.
 * Para cada solicitud de cliente, se crea un nuevo hilo que maneja la solicitud.
 * 
 * @return 0 en caso de éxito.
 */
int main() {
    int server_fd, *client_socket;  // Descriptores de archivo para el servidor y el cliente
    struct sockaddr_in address;      // Estructura para almacenar la dirección del servidor
    int addrlen = sizeof(address);   // Longitud de la dirección

    // Crea el socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");      // Manejo de errores al crear el socket
        exit(EXIT_FAILURE);
    }

    // Configura la dirección del servidor
    address.sin_family = AF_INET;     // Usar IPv4
    address.sin_addr.s_addr = INADDR_ANY; // Aceptar conexiones en cualquier dirección
    address.sin_port = htons(PORT);    // Asignar el puerto

    // Vincula el socket a la dirección y puerto
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");         // Manejo de errores al vincular
        exit(EXIT_FAILURE);
    }

    // Escucha solicitudes de conexión
    if (listen(server_fd, BACKLOG) < 0) {
        perror("Listen failed");       // Manejo de errores al escuchar
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << std::endl; // Indica que el servidor está escuchando

    // Bucle principal del servidor
    while (true) {
        client_socket = (int *)malloc(sizeof(int)); // Asigna memoria para el socket del cliente
        *client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen); // Acepta una nueva conexión
        if (*client_socket < 0) {
            perror("Accept failed"); // Manejo de errores al aceptar
            free(client_socket);      // Libera memoria en caso de error
            continue; // Continúa con la siguiente iteración
        }
        pthread_t thread_id; // Crea un identificador de hilo
        pthread_create(&thread_id, nullptr, handle_request, client_socket); // Crea un nuevo hilo para manejar la solicitud
        pthread_detach(thread_id); // Separa el hilo para liberar recursos automáticamente al terminar
    }

    return 0; // Finaliza el programa
}

