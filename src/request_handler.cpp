#include "request_handler.h"   // Incluir el encabezado para manejar solicitudes
#include <cstring>             // Incluir para operaciones de manipulación de cadenas
#include <cstdlib>            // Incluir para funciones de C estándar
#include <ctime>              // Incluir para obtener la hora y fecha actual
#include <iostream>           // Incluir para operaciones de entrada/salida
#include <vector>             // Incluir para usar el contenedor vector
#include <string>             // Incluir para usar la clase string
#include <sstream>            // Incluir para manipulación de cadenas de flujo

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

std::string current_session_id;

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
    std::string response_body; // Cuerpo de la respuesta

    // Manejo de métodos HTTP
    if (request.find("POST") != std::string::npos) { 
        std::string::size_type body_start = request.find("\r\n\r\n"); // Buscar el inicio del cuerpo de la solicitud
        if (body_start != std::string::npos) {
            body_start += 4; // Ajustar la posición al inicio del cuerpo
            std::string body = request.substr(body_start); // Obtener el cuerpo de la solicitud

            // Manejo del JSON
            std::string name, email;
            std::string::size_type name_pos = body.find("\"name\": \"");
            std::string::size_type email_pos = body.find("\"email\": \"");
            
            if (name_pos != std::string::npos && email_pos != std::string::npos) {
                name_pos += 9; // Ajustar posición al inicio del valor del nombre
                email_pos += 10; // Ajustar posición al inicio del valor del email
                auto name_end = body.find("\"", name_pos); // Buscar fin del nombre
                auto email_end = body.find("\"", email_pos); // Buscar fin del email
                
                if (name_end != std::string::npos && email_end != std::string::npos) {
                    name = body.substr(name_pos, name_end - name_pos); // Extraer el nombre
                    email = body.substr(email_pos, email_end - email_pos); // Extraer el email

                    // Almacenar la nueva entrada, evitando duplicados
                    bool duplicate = false;
                    for (const auto& entry : entries) {
                        if (strcmp(entry.name, name.c_str()) == 0 && strcmp(entry.email, email.c_str()) == 0) {
                            duplicate = true; // Marcar como duplicado
                            break;
                        }
                    }

                    if (!duplicate && entries.size() < MAX_ENTRIES) {
                        Entry new_entry; // Crear una nueva entrada
                        strncpy(new_entry.name, name.c_str(), NAME_LENGTH - 1); // Copiar el nombre
                        new_entry.name[NAME_LENGTH - 1] = '\0'; // Asegurarse de que la cadena esté terminada en nulo
                        strncpy(new_entry.email, email.c_str(), EMAIL_LENGTH - 1); // Copiar el correo
                        new_entry.email[EMAIL_LENGTH - 1] = '\0'; // Asegurarse de que la cadena esté terminada en nulo
                        entries.push_back(new_entry); // Almacenar la entrada

                        std::cout << "Entrada guardada: " << new_entry.name << ", " << new_entry.email << std::endl; // Mensaje de depuración
                        response_body = "Entrada guardada: " + std::string(new_entry.name) + ", " + new_entry.email;
                        response_stream << "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n" << response_body;
                    } else if (duplicate) {
                        response_stream << "HTTP/1.1 409 Conflict\r\nContent-Type: text/plain\r\n\r\nEntrada duplicada."; // Respuesta de conflicto por entrada duplicada
                    } else {
                        response_stream << "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nNo se pueden almacenar más entradas."; // Respuesta de error si se alcanza el límite
                    }
                }
            } else {
                response_stream << "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nInvalid data format."; // Respuesta de error si el formato es inválido
            }
        } else {
            response_stream << "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nInvalid Request"; // Respuesta por defecto para solicitudes inválidas
        }
        return response_stream.str(); // Retornar la respuesta
    } else if (request.find("GET") != std::string::npos) { // Si es una solicitud GET
        response_stream << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n["; // Iniciar la respuesta JSON
        for (size_t i = 0; i < entries.size(); i++) {
            if (i > 0) response_stream << ","; // Separar entradas con comas
            response_stream << "{\"name\":\"" << entries[i].name << "\", \"email\":\"" << entries[i].email << "\"}"; // Añadir entrada en formato JSON
        }
        response_stream << "]"; // Cerrar el arreglo JSON
        current_session_id = session_id;
        // Añadir el total de entradas a la respuesta
        std::cout << "Solicitando entradas guardadas. Total: " << entries.size() << std::endl; // Mensaje de depuración
        response_stream << "\nTotal de entradas guardadas: " << entries.size(); // Añadir total de entradas
        response_stream << "\nSession ID actual: " << session_id;
        return response_stream.str(); // Retornar la respuesta
    }

    // Si hay un ID de sesión en la cookie
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

    // Respuesta por defecto para solicitudes inválidas
    response_stream << "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nInvalid Request";
    return response_stream.str(); // Retornar la respuesta de error
}

