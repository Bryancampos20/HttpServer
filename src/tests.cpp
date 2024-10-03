#include <gtest/gtest.h>
#include "request_handler.h" 
#include <thread>

// Inicialización de las entradas antes de cada prueba
class HttpServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        entries.clear(); 
    }
};

//------Pruebas con GET

//Get cuando no hay entradas y retorno un json vacío 
TEST_F(HttpServerTest, GetEntries_WhenNoEntries_ReturnsEmptyJsonArray) {
    std::string session_id = generate_session_id(); 
    std::string request = "GET /entries HTTP/1.1\r\n\r\n"; // Simulamos la solicitud GET

    std::string response = process_request(request, session_id); 

    // Verificamos la respuesta, que sea un arreglo json vacío
    EXPECT_NE(response.find("[]"), std::string::npos); 
    EXPECT_NE(response.find("Total de entradas guardadas: 0"), std::string::npos); 
}

//Get luego de agregar una entrada, retorno un json con la entrada
TEST_F(HttpServerTest, GetEntries_AfterAddingEntry_ReturnsJsonWithEntry) {

    // Simulamos una solicitud POST para agregar una entrada
    UserData userData;
    userData.name = "Pamela González";
    userData.email = "Pamela@gmail.com";

    std::string post_request = "POST /entries HTTP/1.1\r\n\r\n"
                                "{\"name\":\"" + userData.name + "\", \"email\":\"" + userData.email + "\"}";
    std::string post_response = process_request(post_request, "1"); 

    // Verificamos que la entrada fue guardada
    EXPECT_NE(post_response.find("Entrada guardada."), std::string::npos);

    // Realizamos la solicitud GET
    std::string session_id = "1";
    std::string request = "GET /entries HTTP/1.1\r\n\r\n";

    std::string response = process_request(request, session_id); 

    // Verificamos que la respuesta del GET contenga la entrada que añadimos
    EXPECT_NE(response.find("Pamela González"), std::string::npos);
    EXPECT_NE(response.find("Pamela@gmail.com"), std::string::npos);
    EXPECT_NE(response.find("Total de entradas guardadas: 1"), std::string::npos); 
}

//------Pruebas con POST

// Añadir una entrada correctamente
TEST_F(HttpServerTest, AddEntry_ReturnsSuccessMessage) {
    UserData userData;
    userData.name = "Yendry González";
    userData.email = "yendry@example.com";

    // Simulamos la solicitud POST 
    std::string post_request = "POST /entries HTTP/1.1\r\n\r\n"
                                "{\"name\":\"" + userData.name + "\", \"email\":\"" + userData.email + "\"}";
    std::string response = process_request(post_request, generate_session_id());

    // Verificamos que la entrada se guardó
    EXPECT_NE(response.find("Entrada guardada."), std::string::npos);
}

// Post cuando el formato de la entrada es incorrecto
TEST_F(HttpServerTest, AddEntryWithInvalidFormat_ReturnsFormatErrorMessage) {
    //Datos para ingresar, faltaría campo email. Formato estaría incompleto
    UserData userData;
    userData.name = "Pamela González";

    // Simulamos la solicitud POST
    std::string invalid_post_request = "POST /entries HTTP/1.1\r\n\r\n"
                                       "{\"name\":\"" + userData.name + "\"}";
    std::string response = process_request(invalid_post_request, generate_session_id());

    // Verificamos que la respuesta 
    EXPECT_NE(response.find("Formato de datos inválido."), std::string::npos);
}

// Post cuando la entrada es duplicada
TEST_F(HttpServerTest, AddDuplicateEntry_ReturnsDuplicateMessage) {
    UserData userData;
    userData.name = "Pamela González";
    userData.email = "Pamela@gmail.com";

    // Simulamos la solicitud POST
    std::string post_request = "POST /entries HTTP/1.1\r\n\r\n"
                                "{\"name\":\"" + userData.name + "\", \"email\":\"" + userData.email + "\"}";
    process_request(post_request, "1");

    // Intentamos añadir la misma entrada 
    std::string duplicate_post_request = "POST /entries HTTP/1.1\r\n\r\n"
                                          "{\"name\":\"" + userData.name + "\", \"email\":\"" + userData.email + "\"}";
    std::string response = process_request(duplicate_post_request, "1");

    // Verifica que la respuesta indique que hay un duplicado
    EXPECT_NE(response.find("Entrada duplicada."), std::string::npos);
}
//------Pruebas con DELETE

TEST_F(HttpServerTest, DeleteEntry_ReturnsSuccessMessage) {
    // Datos para ingresar (ahora con el campo email incluido)
    UserData userData;
    userData.name = "Pamela González";
    userData.email = "pamela@example.com";

    // Simulamos la solicitud POST para agregar una entrada completa
    std::string post_request = "POST /entries HTTP/1.1\r\n\r\n"
                               "{\"name\":\"" + userData.name + "\", \"email\":\"" + userData.email + "\"}";
    std::string post_response = process_request(post_request, generate_session_id());

    // Simulamos la solicitud DELETE para eliminar la misma entrada
    std::string delete_request = "DELETE /entries HTTP/1.1\r\n\r\n"
                                 "{\"name\":\"" + userData.name + "\", \"email\":\"" + userData.email + "\"}";
    std::string delete_response = process_request(delete_request, generate_session_id());

    // Verificar que la respuesta DELETE contenga el mensaje esperado
    EXPECT_NE(delete_response.find("Entrada eliminada."), std::string::npos);
}

TEST_F(HttpServerTest, DeleteEntry_ReturnsFailMessage) {
    // Datos para ingresar (ahora con el campo email incluido)
    UserData userData;
    userData.name = "Pamela González";
    userData.email = "pamela@example.com";

    // Simulamos la solicitud DELETE para eliminar la misma entrada
    std::string delete_request = "DELETE /entries HTTP/1.1\r\n\r\n"
                                 "{\"name\":\"" + userData.name + "\", \"email\":\"" + userData.email + "\"}";
    std::string delete_response = process_request(delete_request, generate_session_id());

    // Verificar que la respuesta DELETE contenga el mensaje esperado
    EXPECT_NE(delete_response.find("Entrada no encontrada."), std::string::npos);
}
//------Pruebas con gestión de cookies 

TEST_F(HttpServerTest, CreatesAndUsesSessionCookie) {
    UserData userData;
    userData.name = "Pamela González";
    userData.email = "Pamela@gmail.com";

    // Simulamos una solicitud POST sin una session_id (nueva sesión)
    std::string post_request = "POST /entries HTTP/1.1\r\n\r\n"
                                "{\"name\":\"" + userData.name + "\", \"email\":\"" + userData.email + "\"}";
    std::string response = process_request(post_request, "1"); //Se envía 1 como session_id

    // Simulamos que la cookie de sesión ha sido generada
    std::string session_id = "1"; 

    // Simulamos una solicitud GET usando la cookie de sesión establecida
    std::string get_request = "GET /entries HTTP/1.1\r\n\r\n";
    std::string get_response = process_request(get_request, session_id);

    // Verificamos que la sesión se haya usado correctamente y devuelva la entrada
    EXPECT_NE(get_response.find("Session ID actual: 1"), std::string::npos);
    EXPECT_NE(get_response.find("Pamela González"), std::string::npos);
    EXPECT_NE(get_response.find("Pamela@gmail.com"), std::string::npos);
}

// Prueba con cookie de sesión inválida (vacía)
TEST_F(HttpServerTest, InvalidSessionCookie) {

    // Simulamos una cookie inválida (session_id vacío)
    std::string get_request2 = "GET /entries HTTP/1.1\r\n\r\n";
    std::string get_response = process_request(get_request2, ""); //Se envía un session_id vacío

    // Verificamos que en la respuesta no exista un session_id definido
    EXPECT_NE(get_response.find("Session ID actual:"), std::string::npos);
}

//------Pruebas de concurrencia

// Función auxiliar para simular el POST, para las pruebas de concurrencia
void simulate_post_request(UserData userData) {
    std::string post_request = "POST /entries HTTP/1.1\r\n\r\n"
                                "{\"name\":\"" + userData.name + "\", \"email\":\"" + userData.email + "\"}";
    process_request(post_request, generate_session_id());
}

// Prueba de manejo concurrente de múltiples solicitudes
TEST_F(HttpServerTest, ModifiesEntriesSafely) {
    UserData user1;
    user1.name = "Pamela González";
    user1.email = "Pamela@gmail.com";

    UserData user2;
    user2.name = "Jeison Duarte";
    user2.email = "jeison@example.com";

    UserData user3;
    user3.name = "Alicia Leiva";
    user3.email = "alicia@example.com";

    // Simulamos múltiples solicitudes concurrentes utilizando hilos
    std::thread t1(simulate_post_request, user1);
    std::thread t2(simulate_post_request, user2);
    std::thread t3(simulate_post_request, user3);

    // Esperamos a que todos los hilos terminen
    t1.join();
    t2.join();
    t3.join();

    // Realizamos una solicitud GET para verificar que todas las entradas fueron agregadas correctamente
    std::string get_request = "GET /entries HTTP/1.1\r\n\r\n";
    std::string get_response = process_request(get_request, "1");

    // Verificamos que las tres entradas estén presentes
    EXPECT_NE(get_response.find("Pamela González"), std::string::npos);
    EXPECT_NE(get_response.find("jeison@example.com"), std::string::npos);
    EXPECT_NE(get_response.find("Alicia Leiva"), std::string::npos);
}

// Simula múltiples solicitudes POST concurrentes con la misma entrada
TEST_F(HttpServerTest, SameEntry) {
    UserData userData;
    userData.name = "Pamela González";
    userData.email = "Pamela@gmail.com";

    // Simulamos múltiples solicitudes POST concurrentes utilizando la misma entrada
    std::thread t1(simulate_post_request, userData);
    std::thread t2(simulate_post_request, userData);
    std::thread t3(simulate_post_request, userData);

    // Esperamos a que todos los hilos terminen
    t1.join();
    t2.join();
    t3.join();

    // Verificamos que la entrada no se haya agregado más de una vez
    std::string get_request = "GET /entries HTTP/1.1\r\n\r\n";
    std::string get_response = process_request(get_request, "1");

    // Verificamos que solo una entrada de "Pamela González" esté presente
    size_t first_occurrence = get_response.find("Pamela González");
    EXPECT_NE(first_occurrence, std::string::npos);  // Debe estar presente
    size_t second_occurrence = get_response.find("Pamela González", first_occurrence + 1);
    EXPECT_EQ(second_occurrence, std::string::npos);  // No debe haber una segunda entrada de Pamela
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

