#include "request_handler.h" 
#include <cstring>           
#include <cstdlib>          
#include <ctime>            
#include <iostream>         
#include <vector>           
#include <string>           
#include <sstream>          

#define SESSION_ID_LENGTH 32 // Define la longitud del ID de sesión.
#define MAX_ENTRIES 200      // Define el número máximo de entradas que se pueden almacenar.
#define NAME_LENGTH 50       // Define la longitud máxima para los nombres.
#define EMAIL_LENGTH 50      // Define la longitud máxima para los correos electrónicos.

// Estructura para almacenar una entrada de usuario que contiene un nombre y un correo electrónico.
struct Entry {
    char name[NAME_LENGTH]; 
    char email[EMAIL_LENGTH]; 
};

// Vector que almacena todas las entradas del usuario.
std::vector<Entry> entries;

// Función para generar un ID de sesión aleatorio.
std::string generate_session_id() {
    std::string session_id(SESSION_ID_LENGTH, '\0'); 
    srand(static_cast<unsigned int>(time(NULL))); 
    
    snprintf(&session_id[0], SESSION_ID_LENGTH + 1, "%lx%04x", time(NULL), rand() % 0xFFFF);
    
    return session_id; 
}

// Función para procesar las solicitudes HTTP.
std::string process_request(const std::string &request, const std::string &session_id) {
    std::ostringstream response_stream; // Flujo de salida para construir la respuesta.
    std::string response_body; // Cuerpo de la respuesta.

    // Verifica si la solicitud es de tipo POST.
    if (request.find("POST") != std::string::npos) {
        std::string::size_type body_start = request.find("\r\n\r\n"); // Encuentra el inicio del cuerpo de la solicitud.
        if (body_start != std::string::npos) { // Asegura que se encontró el cuerpo.
            body_start += 4; // Avanza el puntero para saltar los saltos de línea.
            std::string body = request.substr(body_start); // Extrae el cuerpo de la solicitud.

            std::string name, email; // Variables para almacenar el nombre y el correo electrónico.
            std::string::size_type name_pos = body.find("\"name\": \""); // Busca la posición del nombre en el cuerpo.
            std::string::size_type email_pos = body.find("\"email\": \""); // Busca la posición del correo electrónico en el cuerpo.

            // Verifica que se hayan encontrado ambas posiciones.
            if (name_pos != std::string::npos && email_pos != std::string::npos) {
                name_pos += 9; // Ajusta la posición para obtener el valor del nombre.
                email_pos += 10; // Ajusta la posición para obtener el valor del correo electrónico.
                auto name_end = body.find("\"", name_pos); // Encuentra el final del nombre.
                auto email_end = body.find("\"", email_pos); // Encuentra el final del correo electrónico.

                // Verifica que se hayan encontrado ambos finales.
                if (name_end != std::string::npos && email_end != std::string::npos) {
                    name = body.substr(name_pos, name_end - name_pos); // Extrae el nombre.
                    email = body.substr(email_pos, email_end - email_pos); // Extrae el correo electrónico.

                    bool duplicate = false; // Bandera para verificar si hay entradas duplicadas.
                    
                    // Verifica si ya existe una entrada con el mismo nombre y correo electrónico.
                    for (const auto& entry : entries) {
                        if (strcmp(entry.name, name.c_str()) == 0 && strcmp(entry.email, email.c_str()) == 0) {
                            duplicate = true; // Marca como duplicado si se encuentra.
                            break;
                        }
                    }

                    // Si no es duplicado y hay espacio para más entradas.
                    if (!duplicate && entries.size() < MAX_ENTRIES) {
                        Entry new_entry; // Crea una nueva entrada.
                        strncpy(new_entry.name, name.c_str(), NAME_LENGTH - 1); // Copia el nombre.
                        new_entry.name[NAME_LENGTH - 1] = '\0'; // Asegura la terminación nula.
                        strncpy(new_entry.email, email.c_str(), EMAIL_LENGTH - 1); // Copia el correo electrónico.
                        new_entry.email[EMAIL_LENGTH - 1] = '\0'; // Asegura la terminación nula.
                        entries.push_back(new_entry); // Agrega la nueva entrada al vector.

                        // Construye el cuerpo de la respuesta para una entrada guardada.
                        response_body = "Entrada guardada: " + std::string(new_entry.name) + ", " + new_entry.email;
                        response_stream << response_body; // Agrega el cuerpo a la respuesta.
                    } else if (duplicate) {
                        response_stream << "Entrada duplicada."; // Mensaje para entrada duplicada.
                    } else {
                        response_stream << "No se pueden almacenar más entradas."; // Mensaje si no hay espacio para más entradas.
                    }
                }
            } else {
                response_stream << "Formato de datos inválido."; // Mensaje si el formato es incorrecto.
            }
        }
    // Manejo de solicitudes GET.
    } else if (request.find("GET") != std::string::npos) {
        response_stream << "["; // Inicia la respuesta en formato JSON.
        
        // Recorre todas las entradas y las agrega al flujo de respuesta.
        for (size_t i = 0; i < entries.size(); i++) {
            if (i > 0) response_stream << ","; // Agrega una coma entre entradas.
            response_stream << "{\"name\":\"" << entries[i].name << "\", \"email\":\"" << entries[i].email << "\"}";
        }
        response_stream << "]"; // Cierra el arreglo JSON.
        response_stream << "\nTotal de entradas guardadas: " << entries.size(); // Agrega el conteo de entradas.
        response_stream << "\nSession ID actual: " << session_id; // Agrega el ID de sesión actual.

    } else if (request.find("DELETE") != std::string::npos) {

        // Encuentra el cuerpo de la solicitud
        std::string::size_type body_start = request.find("\r\n\r\n");
        if (body_start != std::string::npos) {
            body_start += 4;
            std::string body = request.substr(body_start);

            std::string name, email;
            std::string::size_type name_pos = body.find("\"name\": \"");
            std::string::size_type email_pos = body.find("\"email\": \"");

            // Extrae nombre y correo si se encuentran
            if (name_pos != std::string::npos && email_pos != std::string::npos) {
                name_pos += 9;
                email_pos += 10;
                auto name_end = body.find("\"", name_pos);
                auto email_end = body.find("\"", email_pos);

                if (name_end != std::string::npos && email_end != std::string::npos) {
                    name = body.substr(name_pos, name_end - name_pos); // Extrae el nombre.
                    email = body.substr(email_pos, email_end - email_pos); // Extrae el correo electrónico.

                    // Depuración para verificar los datos extraídos.
                    response_stream << "Debug - Nombre extraído: " << name << "\n";
                    response_stream << "Debug - Correo extraído: " << email << "\n";

                    bool found = false;

                    response_stream << "Debug - Comenzando la búsqueda de la entrada...\n";

                    // Busca la entrada con el nombre y correo especificados
                    for (auto it = entries.begin(); it != entries.end(); ++it) {
                        response_stream << "Debug - Comparando con: " << it->name << ", " << it->email << "\n";
                        if (strcmp(it->name, name.c_str()) == 0 && strcmp(it->email, email.c_str()) == 0) {
                            entries.erase(it);
                            response_stream << "Debug - Entrada encontrada y eliminada.\n";
                            found = true;
                            break;
                        }
                    }

                    // Construye el cuerpo de la respuesta para una entrada eliminada.
                    if (found) {
                        response_stream << "Entrada eliminada: " << name << ", " << email;
                    } else {
                        response_stream << "Entrada no encontrada.";
                    }
                }
            }
        }
    } else {
        response_stream << "Solicitud no válida.";
    }

    // Construcción del encabezado de respuesta HTTP
    std::string http_response = "HTTP/1.1 200 OK\r\n"; // Define el estado HTTP.
    http_response += "Content-Type: application/json\r\n"; // Define el tipo de contenido de la respuesta.
    http_response += "Content-Length: " + std::to_string(response_stream.str().size()) + "\r\n"; // Define la longitud del contenido.
    http_response += "\r\n";  // Línea en blanco separando encabezado y cuerpo.
    http_response += response_stream.str(); // Agrega el cuerpo de la respuesta.

    return http_response;  
}

