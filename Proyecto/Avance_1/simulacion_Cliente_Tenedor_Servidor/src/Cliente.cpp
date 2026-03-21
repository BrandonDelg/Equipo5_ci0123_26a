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

void Cliente::send_to_server(void* package) {
    Message* msg = (Message*)package;
    pthread_mutex_lock(&queueMutex);
    messageQueue.push(msg);
    pthread_cond_signal(&queueCond);
    pthread_mutex_unlock(&queueMutex);
    if (msg->type != CLOSE) {
        std::cout << "[CLIENTE] Solicitud enviada -> Figura: " << msg->figura << " mitad: " << msg->mitad << std::endl;
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

    Message* msg = server->getQueue().front();
    server->getQueue().pop();
    pthread_mutex_unlock(mutex_server);
    if (msg->type == CLOSE) {
        delete msg;
        return false;
    }

    std::cout << "\n[CLIENTE] Piezas necesarias:" << std::endl;
    std::cout << msg->message << std::endl;

    delete msg;
    return true;
}

std::queue<Message*>& Cliente::getQueue() {
    return messageQueue;
}

pthread_mutex_t* Cliente::getMutex() {
    return &queueMutex;
}

pthread_cond_t* Cliente::getVC() {
    return &queueCond;
}