#include <iostream>
#include <thread>
#include <string>
#include "Socket.hpp"

#define PORT 8080
#define BUFSIZE 512

char* SERVER_HOST = " ";
#define SERVER_PORT "1234"

void task(VSocket* client, bool ipv6) {
    char buffer[BUFSIZE] = {0};
    int st;
    std::string request;

    while ((st = client->Read(buffer, BUFSIZE - 1)) > 0) {
        buffer[st] = 0;
        request += buffer;
        if (request.find("\r\n\r\n") != std::string::npos) break;
    }

    std::cout << "HTTP recibido:\n" << request << std::endl;

    if (request.find("GET /shutdown") != std::string::npos) {
        std::cout << "[INTERMEDIARIO] Shutdown solicitado\n";

        VSocket* server = new Socket('s', ipv6);
        server->Connect(SERVER_HOST, SERVER_PORT);

        std::string internal = "99|";
        server->Write(internal.c_str(), internal.length());

        server->Close();
        delete server;

        std::string http = "HTTP/1.1 200 OK\r\n\r\nApagando servidor e intermediario";
        client->Write(http.c_str(), http.length());

        client->Close();
        delete client;

        exit(0);
    }

    if (request.find("GET /favicon.ico") != std::string::npos) {
        std::string http = "HTTP/1.1 204 No Content\r\n\r\n";
        client->Write(http.c_str(), http.length());
        client->Close();
        delete client;
        return;
    }

    std::string internal;
    bool valid = false;
    if (request.find("GET /figuras") != std::string::npos) {
        internal = "10|";
        valid = true;
    } else if (request.find("GET /figura/") != std::string::npos) {
        size_t start = request.find("/figura/") + 8;
        size_t end = request.find(" ", start);
        if (end == std::string::npos) {
            std::string http = "HTTP/1.1 400 Bad Request\r\n\r\nFormato invalido";
            client->Write(http.c_str(), http.length());
            client->Close();
            delete client;
            return;
        }

        std::string ruta = request.substr(start, end - start);
        size_t slash = ruta.find("/");

        if (slash == std::string::npos) {
            std::string http = "HTTP/1.1 400 Bad Request\r\n\r\nFalta mitad";
            client->Write(http.c_str(), http.length());
            client->Close();
            delete client;
            return;
        }

        std::string figura = ruta.substr(0, slash);
        std::string parteStr = ruta.substr(slash + 1);

        if (figura.empty()) {
            std::string http = "HTTP/1.1 400 Bad Request\r\n\r\nFigura vacia";
            client->Write(http.c_str(), http.length());
            client->Close();
            delete client;
            return;
        }

        int parte = 0;
        try {
            parte = std::stoi(parteStr);
        } catch (...) {
            std::string http = "HTTP/1.1 400 Bad Request\r\n\r\nMitad invalida";
            client->Write(http.c_str(), http.length());
            client->Close();
            delete client;
            return;
        }

        // if (parte != 1 && parte != 2) {
        //     std::string http = "HTTP/1.1 400 Bad Request\r\n\r\nMitad fuera de rango";
        //     client->Write(http.c_str(), http.length());
        //     client->Close();
        //     delete client;
        //     return;
        // }

        internal = "14|figura=" + figura + ";mitad=" + std::to_string(parte);
        valid = true;
    }
    if (!valid) {
        std::string http = "HTTP/1.1 404 Not Found\r\n\r\nRuta no valida";
        client->Write(http.c_str(), http.length());
        client->Close();
        delete client;
        return;
    }

    std::cout << "[INTERMEDIARIO] Enviando: " << internal << std::endl;

    VSocket* server = new Socket('s', ipv6);
    server->Connect(SERVER_HOST, SERVER_PORT);
    server->Write(internal.c_str(), internal.length());

    std::string response;
    st = server->Read(buffer, BUFSIZE - 1);
    if (st > 0) {
        buffer[st] = 0;
        response = buffer;
    }

    std::cout << "[INTERMEDIARIO] Recibido: " << response << std::endl;
    std::string http;
    if (response.find("11|") == 0) {
        size_t pos = response.find("figuras=");
        std::string data = (pos != std::string::npos) ? response.substr(pos + 8) : "";
        http = "HTTP/1.1 200 OK\r\n\r\n" + data;
    } else if (response.find("105|") == 0) {
        http = "HTTP/1.1 400 Bad Request\r\n\r\nMitad invalida";
    } else if (response.find("15|") == 0) {
        size_t pos = response.find("piezas=");
        std::string piezas = (pos != std::string::npos) ? response.substr(pos + 7) : "";
        http = "HTTP/1.1 200 OK\r\n\r\n" + piezas;
    } else if (response.find("16|") == 0) {
        http = "HTTP/1.1 404 Not Found\r\n\r\nFigura no encontrada";
    } else if (response.find("100") == 0) {
        http = "HTTP/1.1 400 Bad Request\r\n\r\nError de protocolo";
    } else {
        http = "HTTP/1.1 500 Internal Server Error\r\n\r\nRespuesta desconocida";
    }
    client->Write(http.c_str(), http.length());

    server->Close();
    delete server;

    client->Close();
    delete client;
}

int main(int argc, char* argv[]) {
    VSocket* intermediario;
    std::thread* worker;
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <host> <ipv6 0|1>\n";
        return 1;
    }

    SERVER_HOST = argv[1];
    bool ipv6 = std::stoi(argv[2]);
    intermediario = new Socket('s', ipv6);

    intermediario->Bind(PORT);
    intermediario->MarkPassive(5);

    std::cout << "Intermediario escuchando en puerto " << PORT << "...\n";

    while (true) {
        VSocket* client = intermediario->AcceptConnection();

        worker = new std::thread(task, client, ipv6);
        worker->detach();
    }

    return 0;
}