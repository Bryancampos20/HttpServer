#include "request_handler.h" 
#include <cstring>           
#include <cstdlib>          
#include <ctime>            
#include <iostream>         
#include <vector>           
#include <string>           
#include <sstream>          
#include <mutex> 

#define SESSION_ID_LENGTH 32 // Define la longitud del ID de sesión.
#define MAX_ENTRIES 200      // Define el número máximo de entradas que se pueden almacenar.

// Vector que almacena todas las entradas del usuario.
std::vector<Entry> entries;

std::mutex entries_mutex; // Referenciamos el mutex

// Función para generar un ID de sesión aleatorio.
std::string generate_session_id() {
    std::string session_id(SESSION_ID_LENGTH, '\0'); 
    srand(static_cast<unsigned int>(time(NULL))); 
    
    snprintf(&session_id[0], SESSION_ID_LENGTH + 1, "%lx%04x", time(NULL), rand() % 0xFFFF);
    
    return session_id; 
}

// Función para extraer el nombre, correo, updated_name y updated_email del cuerpo de la solicitud
UserData extract_name_and_email(const std::string& request) {
    UserData userData;

    // Encuentra el inicio del cuerpo de la solicitud
    std::string::size_type body_start = request.find("\r\n\r\n");
    if (body_start != std::string::npos) {
        body_start += 4; // Salta los saltos de línea
        std::string body = request.substr(body_start);

        // Función lambda para extraer valor por clave
        auto extract_value = [&](const std::string& key) -> std::string {
            std::string::size_type key_pos = body.find(key);
            if (key_pos == std::string::npos) {
                return "";
            }
            // Encuentra la primera comilla después de la clave
            key_pos = body.find("\"", key_pos + key.length());
            if (key_pos == std::string::npos) {
                return "";
            }
            key_pos += 1; // Salta la comilla
            // Encuentra la comilla de cierre
            std::string::size_type value_end = body.find("\"", key_pos);
            if (value_end == std::string::npos) {
                return "";
            }
            return body.substr(key_pos, value_end - key_pos);
        };

        // Extrae los valores
        userData.name = extract_value("\"name\":");
        userData.email = extract_value("\"email\":");
        userData.updated_name = extract_value("\"updated_name\":");
        userData.updated_email = extract_value("\"updated_email\":");
    }

    return userData;
}

// Procesar las solicitudes HTTP.
std::string process_request(const std::string &request, const std::string &session_id) {
    std::ostringstream response_stream; // Flujo de salida para construir la respuesta.
    std::string response_body; // Cuerpo de la respuesta.

    // Verifica si la solicitud es de tipo POST.
    if (request.find("POST") != std::string::npos) {
        // Extrae los datos del JSON en el cuerpo de la solicitud
        UserData userData = extract_name_and_email(request);

        // Verificar si el nombre o el correo están vacíos
        if (userData.name.empty() || userData.email.empty()) {
            response_stream << "Formato de datos inválido.";
        } else {
            bool duplicate = false; // Bandera para verificar si hay entradas duplicadas.
            
            std::lock_guard<std::mutex> lock(entries_mutex); // Bloquea el mutex antes de acceder a 'entries'
            
            // Verifica si ya existe una entrada con el mismo nombre y correo electrónico.
            for (const auto& entry : entries) {
                if (strcmp(entry.name, userData.name.c_str()) == 0 && strcmp(entry.email, userData.email.c_str()) == 0) {
                    duplicate = true; 
                    break;
                }
            }  
            // Si no es duplicado y hay espacio para más entradas.
            if (!duplicate && entries.size() < MAX_ENTRIES) {
                Entry new_entry; // Crea una nueva entrada.
                strncpy(new_entry.name, userData.name.c_str(), NAME_LENGTH - 1); // Copia el nombre.
                new_entry.name[NAME_LENGTH - 1] = '\0'; // Asegura la terminación nula.
                strncpy(new_entry.email, userData.email.c_str(), EMAIL_LENGTH - 1); // Copia el correo electrónico.
                new_entry.email[EMAIL_LENGTH - 1] = '\0'; // Asegura la terminación nula.
                entries.push_back(new_entry); // Agrega la nueva entrada al vector.

                response_stream << "Entrada guardada."; // Agrega el cuerpo a la respuesta.
            } else if (duplicate) {
                response_stream << "Entrada duplicada."; // Mensaje para entrada duplicada.
            } else {
                response_stream << "No se pueden almacenar más entradas."; // Mensaje si no hay espacio para más entradas.
            }
        }

    // Manejo de solicitudes GET.
    } else if (request.find("GET") != std::string::npos) {
        response_stream << "["; // Inicia la respuesta en formato JSON.
        
        std::lock_guard<std::mutex> lock(entries_mutex); // Bloquea el mutex antes de acceder a 'entries'

        // Recorre todas las entradas y las agrega al flujo de respuesta.
        for (size_t i = 0; i < entries.size(); i++) {
            if (i > 0) response_stream << ","; // Agrega una coma entre entradas.
            response_stream << "{\"name\":\"" << entries[i].name << "\", \"email\":\"" << entries[i].email << "\"}";
        }
        response_stream << "]"; // Cierra el arreglo JSON.
        response_stream << "\nTotal de entradas guardadas: " << entries.size(); // Agrega el conteo de entradas.
        response_stream << "\nSession ID actual: " << session_id; // Agrega el ID de sesión actual.

    } else if (request.find("DELETE") != std::string::npos) {
        // Llamada a la función para extraer nombre y correo
        UserData userData = extract_name_and_email(request);

        // Si los datos de usuario fueron extraídos
        if (!userData.name.empty() && !userData.email.empty()) {

            bool found = false;

            std::lock_guard<std::mutex> lock(entries_mutex); // Bloquea el mutex antes de acceder a 'entries'

            // Busca la entrada con el nombre y correo especificados
            for (auto it = entries.begin(); it != entries.end(); ++it) {
                if (strcmp(it->name, userData.name.c_str()) == 0 && strcmp(it->email, userData.email.c_str()) == 0) {
                    entries.erase(it);  // Elimina la entrada
                    found = true;
                    break;
                }
            }

            // Respuesta final
            if (found) {
                response_stream << "Entrada eliminada.";
            } else {
                response_stream << "Entrada no encontrada.";
            }
        }

    } else if (request.find("UPDATE") != std::string::npos) {
        // Llamada a la función para extraer nombre, correo, updated_name y updated_email
        UserData userData = extract_name_and_email(request);
        
        // Depuración
        response_stream << "Debug - Nombre extraído: " << userData.name << "\n";
        response_stream << "Debug - Correo extraído: " << userData.email << "\n";
        response_stream << "Debug - updated_name extraído: " << userData.updated_name << "\n";
        response_stream << "Debug - updated_email extraído: " << userData.updated_email << "\n";

        // Si los datos de usuario fueron extraídos
        if (!userData.name.empty() && !userData.email.empty() &&
            !userData.updated_name.empty() && !userData.updated_email.empty()) {
            
            bool found = false;

            std::lock_guard<std::mutex> lock(entries_mutex); // Bloquea el mutex antes de acceder a 'entries'

            response_stream << "Debug - Comenzando la búsqueda de la entrada...\n";

            // Busca la entrada con el nombre y correo especificados
            for (auto it = entries.begin(); it != entries.end(); ++it) {
                response_stream << "Debug - Comparando con: " << it->name << ", " << it->email << "\n";
                if (strcmp(it->name, userData.name.c_str()) == 0 &&
                    strcmp(it->email, userData.email.c_str()) == 0) {
                    
                    // Actualiza los campos con los nuevos valores
                    strncpy(it->name, userData.updated_name.c_str(), NAME_LENGTH - 1);
                    it->name[NAME_LENGTH - 1] = '\0';
                    strncpy(it->email, userData.updated_email.c_str(), EMAIL_LENGTH - 1);
                    it->email[EMAIL_LENGTH - 1] = '\0';
                    
                    response_stream << "Debug - Entrada encontrada y actualizada.\n";
                    found = true;
                    break;
                }
            }

            // Respuesta final
            if (found) {
                response_stream << "Entrada actualizada: " << userData.name << ", " 
                                << userData.email << " -> " 
                                << userData.updated_name << ", " 
                                << userData.updated_email;
            } else {
                response_stream << "Entrada no encontrada.";
            }
        } else {
            response_stream << "Formato de datos inválido.";
        }
    } else if (request.find("PUT") != std::string::npos) {
        // Llamada a la función para extraer nombre, correo, updated_name y updated_email
        UserData userData = extract_name_and_email(request);
        
        // Depuración
        response_stream << "Debug - Nombre extraído: " << userData.name << "\n";
        response_stream << "Debug - Correo extraído: " << userData.email << "\n";
        response_stream << "Debug - updated_name extraído: " << userData.updated_name << "\n";
        response_stream << "Debug - updated_email extraído: " << userData.updated_email << "\n";

        // Si los datos de usuario fueron extraídos
        if (!userData.name.empty() && !userData.email.empty() &&
            !userData.updated_name.empty() && !userData.updated_email.empty()) {
            
            bool found = false;

            std::lock_guard<std::mutex> lock(entries_mutex); // Bloquea el mutex antes de acceder a 'entries'

            response_stream << "Debug - Comenzando la búsqueda de la entrada...\n";

            // Busca la entrada con el nombre y correo especificados
            for (auto it = entries.begin(); it != entries.end(); ++it) {
                response_stream << "Debug - Comparando con: " << it->name << ", " << it->email << "\n";
                if (strcmp(it->name, userData.name.c_str()) == 0 &&
                    strcmp(it->email, userData.email.c_str()) == 0) {
                    
                    // Actualiza los campos con los nuevos valores
                    strncpy(it->name, userData.updated_name.c_str(), NAME_LENGTH - 1);
                    it->name[NAME_LENGTH - 1] = '\0';
                    strncpy(it->email, userData.updated_email.c_str(), EMAIL_LENGTH - 1);
                    it->email[EMAIL_LENGTH - 1] = '\0';
                    
                    response_stream << "Debug - Entrada encontrada y actualizada.\n";
                    found = true;
                    break;
                }
            }
 
            // Si no se encuentra la entrada, la crea
            if (!found) {
                if (entries.size() < MAX_ENTRIES) {
                    Entry new_entry;
                    strncpy(new_entry.name, userData.updated_name.c_str(), NAME_LENGTH - 1);
                    new_entry.name[NAME_LENGTH - 1] = '\0';  // Asegura la terminación nula
                    strncpy(new_entry.email, userData.updated_email.c_str(), EMAIL_LENGTH - 1);
                    new_entry.email[EMAIL_LENGTH - 1] = '\0';  // Asegura la terminación nula
                    
                    // Agrega la nueva entrada al vector
                    entries.push_back(new_entry);

                    response_stream << "Nueva entrada creada: " 
                                    << new_entry.name << ", " 
                                    << new_entry.email;
                } else {
                    response_stream << "No se pueden almacenar más entradas.";
                }
            } else {
                // Respuesta final si la entrada fue actualizada
                response_stream << "Entrada actualizada: " 
                                << userData.name << ", " 
                                << userData.email << " -> " 
                                << userData.updated_name << ", " 
                                << userData.updated_email;
            }
        } else {
            response_stream << "Formato de datos inválido.";
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


