#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>
#include <vector>
#include <mutex> 

#define NAME_LENGTH 50 //Longitud máxima para los nombres.
#define EMAIL_LENGTH 50 //Longitud máxima para los correos electrónicos.

//Estructura para almacenar una entrada de usuario
struct Entry {
    char name[NAME_LENGTH]; 
    char email[EMAIL_LENGTH]; 
};

//Estructura para devolver los resultados
struct UserData {
    std::string name;
    std::string email;
    std::string updated_name;
    std::string updated_email;
};

//Para los tests
extern std::vector<Entry> entries; 

//Se declarar el mutex
extern std::mutex entries_mutex; 

std::string generate_session_id();
std::string process_request(const std::string &request, const std::string &session_id);

#endif 
