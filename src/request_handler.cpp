#include "request_handler.h"  // Incluir el encabezado para manejar solicitudes
#include <cstring>            // Incluir para operaciones de manipulación de cadenas
#include <cstdlib>           // Incluir para funciones de C estándar
#include <ctime>             // Incluir para obtener la hora y fecha actual
#include <iostream>          // Incluir para operaciones de entrada/salida
#include <vector>            // Incluir para usar el contenedor vector
#include <string>            // Incluir para usar la clase string
#include <sstream>           // Incluir para manipulación de cadenas de flujo

// Definiciones de constantes
#define SESSION_ID_LENGTH 32 // Longitud del ID de sesión
#define MAX_ENTRIES 200      // Máximo número de entradas que se pueden almacenar
#define NAME_LENGTH 50       // Longitud máxima para el nombre
#define EMAIL_LENGTH 50      // Longitud máxima para el correo electrónico

// Estructura para almacenar una entrada de nombre y correo electrónico
struct Entry {
    char name[NAME_LENGTH];  // Nombre del usuario
    char email[EMAIL_LENGTH]; // Correo electrónico del usuario
};

// Vector para almacenar las entradas
std::vector<Entry> entries;

// Función para generar un ID de sesión único
std::string generate_session_id() {
    std::string session_id(SESSION_ID_LENGTH, '\0'); // Crear un string de longitud SESSION_ID_LENGTH
    srand(static_cast<unsigned int>(time(NULL))); // Inicializar la semilla del generador de números aleatorios
    snprintf(&session_id[0], SESSION_ID_LENGTH + 1, "%lx%04x", time(NULL), rand() % 0xFFFF); // Formatear el ID de sesión
    return session_id; // Retornar el ID de sesión generado
}

// Función para procesar las solicitudes HTTP
std::string process_request(const std::string &request, const std::string &session_id) {
    std::ostringstream response_stream; // Flujo para construir la respuesta

    // Buscar la cabecera de la cookie en la solicitud
    auto cookie_header = request.find("Cookie:");
    if (cookie_header != std::string::npos) {
        auto session_cookie_pos = request.find("session_id=", cookie_header); // Buscar el ID de sesión en la cookie
        if (session_cookie_pos != std::string::npos) {
            session_cookie_pos += strlen("session_id="); // Ajustar la posición al inicio del valor del ID de sesión
            auto end = request.find(';', session_cookie_pos); // Buscar el fin de la cookie
            std::string session_cookie = request.substr(session_cookie_pos, end - session_cookie_pos); // Extraer el valor del ID de sesión
            response_stream << "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nFunciona bien - con cookie. Session ID: " << session_cookie;
            return response_stream.str(); // Retornar la respuesta con la cookie
        }
    }

    // Si no hay ID de sesión en la solicitud, se genera uno nuevo
    if (session_id.empty()) {
        std::string new_session_id = generate_session_id(); // Generar un nuevo ID de sesión
        response_stream << "HTTP/1.1 200 OK\r\nSet-Cookie: session_id=" << new_session_id << "\r\nContent-Type: text/plain\r\n\r\nNueva cookie establecida. Session ID: " << new_session_id;
        return response_stream.str(); // Retornar la respuesta con el nuevo ID de sesión
    } else {
        response_stream << "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nFunciona bien - sin cookie. Session ID: " << session_id;
    }

    // Manejo de métodos HTTP
    if (request.find("POST") != std::string::npos) { // Si es una solicitud POST
        std::string::size_type body_start = request.find("\r\n\r\n"); // Buscar el inicio del cuerpo de la solicitud
        if (body_start != std::string::npos) {
            body_start += 4; // Ajustar la posición al inicio del cuerpo
            char name[NAME_LENGTH] = {0}; // Almacenar el nombre
            char email[EMAIL_LENGTH] = {0}; // Almacenar el correo electrónico

            // Leer el nombre y el correo del cuerpo de la solicitud
            int num_matched = sscanf(request.substr(body_start).c_str(), "{\"name\": \"%[^\"]\", \"email\": \"%[^\"]\"}", name, email);
            if (num_matched != 2) {
                response_stream << "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nInvalid data format."; // Respuesta de error si el formato es inválido
                return response_stream.str();
            }

            // Si hay espacio para más entradas, se añade la nueva entrada
            if (entries.size() < MAX_ENTRIES) {
                Entry new_entry; // Crear una nueva entrada
                strncpy(new_entry.name, name, NAME_LENGTH - 1); // Copiar el nombre
                new_entry.name[NAME_LENGTH - 1] = '\0'; // Asegurarse de que la cadena esté terminada en nulo
                strncpy(new_entry.email, email, EMAIL_LENGTH - 1); // Copiar el correo
                new_entry.email[EMAIL_LENGTH - 1] = '\0'; // Asegurarse de que la cadena esté terminada en nulo
                entries.push_back(new_entry); // Almacenar la entrada
                
                std::cout << "Entrada guardada: " << new_entry.name << ", " << new_entry.email << std::endl; // Mensaje de depuración
                response_stream << "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nEntrada guardada: " << new_entry.name << ", " << new_entry.email;
            } else {
                response_stream << "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nNo se pueden almacenar más entradas."; // Respuesta de error si se alcanza el límite
            }
            return response_stream.str(); // Retornar la respuesta
        }
    } else if (request.find("GET") != std::string::npos) { // Si es una solicitud GET
        if (!entries.empty()) { // Si hay entradas almacenadas
            response_stream << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n["; // Iniciar la respuesta JSON
            for (size_t i = 0; i < entries.size(); i++) {
                if (i > 0) response_stream << ","; // Separar entradas con comas
                response_stream << "{\"name\":\"" << entries[i].name << "\", \"email\":\"" << entries[i].email << "\"}"; // Añadir entrada en formato JSON
            }
            response_stream << "]"; // Cerrar el arreglo JSON
            std::cout << "Solicitando entradas guardadas. Total: " << entries.size() << std::endl; // Mensaje de depuración
        } else {
            response_stream << "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nNo hay entradas guardadas."; // Mensaje si no hay entradas
        }
        return response_stream.str(); // Retornar la respuesta
    }

    // Respuesta por defecto para solicitudes inválidas
    response_stream << "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nInvalid Request";
    return response_stream.str(); // Retornar la respuesta de error
}
