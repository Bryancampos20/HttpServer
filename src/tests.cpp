#include <gtest/gtest.h>
#include "request_handler.h" 

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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

