/**
 * @file Client.cpp
 * @brief Implementación del client
 */
#include <cstring>
#include "Client.hpp"

Client::Client(int id) {
    this->id = id;
}

Client::~Client() {

}
void Client::ClientRequestList(VSocket* client, const char* service, Logger& log, const char* host) {
    std::string request =
    std::string("GET /figuras HTTP/1.1\r\n")
    + "Host: " + host + "\r\n"
    + "\r\n";

    client->Connect(host, service);

    log.log("Conexion establecida al servidor local");
    log.log(request.c_str());

    client->Write(request.c_str(), request.length());
}
void Client::ClientRequestFigure(VSocket* client, std::string figura, int parte, const char* service, Logger& log, const char* host) {
    std::string p = std::to_string(parte);
    std::string request =
        "GET /figura/" + figura + "/" + p + " HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "\r\n";

    client->Connect(host, service);
    log.log("Conexion establecida");
    log.log(request.c_str());

    client->Write(request.c_str(), request.length());
}
void Client::CloseConnection(VSocket* client, const char* service, const char* host) {
    std::string request =
    std::string("GET /shutdown HTTP/1.1\r\n")
    + "Host: " + host + "\r\n"
    + "\r\n";
    client->Connect(host, service);
    client->Write(request.c_str(), request.length());
}
int Client::getId() {
    return this->id;
}
