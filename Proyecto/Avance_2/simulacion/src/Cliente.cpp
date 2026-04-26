/**
 * @file Cliente.cpp
 * @brief Implementación del Cliente
 */
#include "Cliente.hpp"
#include "ServidorIntermedio.hpp"

Cliente::Cliente() {
    this->server = nullptr;
    pthread_mutex_init(&queueMutex, nullptr);
    pthread_cond_init(&queueCond, nullptr);
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
    if (parser.getTipo(msg) != 02) {
        std::cout << "[CLIENTE] Solicitud enviada" << std::endl;
    } else {
        std::cout << "[CLIENTE] Solicitud enviada -> Exit "<< std::endl;
    }
}

bool Cliente::receive_from_server() {
    pthread_mutex_t* mutex_server = server->getMutex();
    pthread_cond_t* cond_server = server->getVC();
    pthread_mutex_lock(mutex_server);
    while (server->getQueue().empty()) {
        pthread_cond_wait(cond_server, mutex_server);
    }

    std::string msg = server->getQueue().front();
    server->getQueue().pop();
    pthread_mutex_unlock(mutex_server);
    if (parser.getTipo(msg) == 02) {
        return false;
    }

    if (parser.getTipo(msg) == 15) {
        std::cout << "\n[CLIENTE] Piezas necesarias:" << std::endl;
    } else if (parser.getTipo(msg) == 16) {
        std::cout << "\n[CLIENTE] ERROR:" << std::endl;
    } else if (parser.getTipo(msg) == 11) {
        std::cout << "\n[CLIENTE]: Respuesta del servidor" << std::endl;
    }

    std::cout << "\n" << parser.getMensaje(msg) << std::endl;

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
