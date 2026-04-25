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

void Client::ClientRequestList(VSocket* client, const char* service, Logger& log) {
    char * os = (char *) "os.ecci.ucr.ac.cr";
    char * request = (char *)
    "GET /lego/index.php HTTP/1.1\r\n"
    "Host: os.ecci.ucr.ac.cr\r\n"
    "Connection: close\r\n"
    "\r\n";
    client->Connect(os, service);
    std::string mlog =  "Conexion establecida: "; 
    mlog += os;
    log.log(mlog.c_str());
    log.log(request);
    client->Write(request, strlen(request));
}

void Client::ClientRequestFigure(VSocket* client, std::string figuraElegida, int parteElegida, const char* service, Logger& log) {
    char * os = (char *) "os.ecci.ucr.ac.cr";
    std::string path = "/lego/list.php?figure=" + figuraElegida + "&part=" + std::to_string(parteElegida);      
    std::string requestFigurePiezas = 
    "GET " + path + " HTTP/1.1\r\n"
    "Host: os.ecci.ucr.ac.cr\r\n"
    "Connection: close\r\n"
    "\r\n";
    //std::cout << "[Servidor][RESPONSE_FIGURE/Figure=" <<  figuraElegida << "/PART=" << parteElegida << "]" << std::endl;
    client->Connect(os, service);
    std::string mlog =  "Conexion establecida: "; 
    mlog += os;
    log.log(mlog.c_str());
    log.log(requestFigurePiezas );
    client->Write((char*)requestFigurePiezas.c_str(), requestFigurePiezas.length());
   
}
std::vector<std::pair<std::string, int>> Client::pedirFigura(std::string figura, int parte,
    int argc, const char* service,Parser& parser, Logger& log) {
    char buffer[MAXBUF];
    int st;
    VSocket* client;
    if (argc > 1) {
    client = new SSLSocket();
    } else {
    client = new Socket('s');
    }
    ClientRequestFigure(client, figura, parte, service, log);

    std::string html;
    while ((st = client->Read(buffer, MAXBUF-1)) > 0) {
        buffer[st] = 0;
        html += buffer;
    }
    parser.procesarPiezas(html);
    auto piezas = parser.getPiezas();
    delete client;
    return piezas;
}


int Client::getId() {
    return this->id;
}
