/**
 * @file Cliente.cpp
 * @brief Implementación del Cliente — protocolo v3.0
 */
#include "Cliente.hpp"
#include "ServidorIntermedio.hpp"

Cliente::Cliente() {
    this->server = nullptr;
    pthread_mutex_init(&queueMutex, nullptr);
    pthread_cond_init(&queueCond, nullptr);
    running = true;
}

Cliente::~Cliente() {
    pthread_mutex_destroy(&queueMutex);
    pthread_cond_destroy(&queueCond);
}

void Cliente::Connect(ServidorIntermedio* serv) {
    this->server = serv;
    this->server->Connect(this);
    std::cout << "[CLIENTE] Conectado al servidor intermedio" << std::endl;
}

void Cliente::send_to_server(std::string msg) {
    pthread_mutex_lock(&queueMutex);
    messageQueue.push(msg);
    pthread_cond_signal(&queueCond);
    pthread_mutex_unlock(&queueMutex);

    if (msg == "P/Q/") {
        std::cout << "[CLIENTE] Solicitud enviada -> Exit" << std::endl;
    } else {
        std::cout << "[CLIENTE] Solicitud enviada: " << msg << std::endl;
    }
}

bool Cliente::receive_from_server() {
    pthread_mutex_t* mutex_server = server->getMutex();
    pthread_cond_t* cond_server   = server->getVC();

    pthread_mutex_lock(mutex_server);
    while (server->getQueue().empty()) {
        pthread_cond_wait(cond_server, mutex_server);
    }
    std::string msg = server->getQueue().front();
    server->getQueue().pop();
    pthread_mutex_unlock(mutex_server);

    if (msg.size() >= 4 && msg[0] == 'P' && msg[1] == '/' && msg[2] == 'D' && msg[3] == '/') {
        std::string data = msg.substr(4);

        if (data == "400") {
            std::cout << "\n[CLIENTE] Error: solicitud inválida (400)\n";
        } else if (data == "404") {
            std::cout << "\n[CLIENTE] Error: figura no encontrada (404)\n";
        } else if (data.find('\n') != std::string::npos) {
            std::cout << "\n[CLIENTE] Piezas necesarias:\n" << data;
        } else {
            std::cout << "\n[CLIENTE] Lista de figuras disponibles:\n";
            std::stringstream ss(data);
            std::string figura;
            while (std::getline(ss, figura, ',')) {
                std::cout << "  - " << figura << "\n";
            }
        }
    } else {
        std::cout << "\n[CLIENTE] Mensaje inesperado: " << msg << std::endl;
    }

    return true;
}

std::queue<std::string>& Cliente::getQueue() {
    return messageQueue;
}

pthread_mutex_t* Cliente::getMutex() {
    return &queueMutex;
}

pthread_cond_t* Cliente::getVC() {
    return &queueCond;
}
