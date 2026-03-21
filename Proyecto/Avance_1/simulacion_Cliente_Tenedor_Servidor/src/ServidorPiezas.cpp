#include "ServidorPiezas.hpp"
#include "ServidorIntermedio.hpp"
#include "Cliente.hpp"
#include <cstring>
#include <iostream>

ServidorPiezas::ServidorPiezas() {
    running = true;
    pthread_mutex_init(&mutex,nullptr);
    pthread_cond_init(&cond,nullptr);
}

ServidorPiezas::~ServidorPiezas() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

void ServidorPiezas::Connect(ServidorIntermedio* r) {
    router = r;
}

void ServidorPiezas::listen() {
    while(running) {
        pthread_mutex_lock(&mutex);
        while(queue.empty()) {
            pthread_cond_wait(&cond,&mutex);
        }
        Message* msg = queue.front();
        queue.pop();
        pthread_mutex_unlock(&mutex);
        
        if(msg->type == CLOSE) {
            std::cout << "[SERVIDOR PIEZAS] Recibida señal de cierre" << std::endl;
            delete msg;
            break;
        }
        if(msg->type == REQUEST_FIGURE) {
            procesarSolicitud(msg);
        }
        delete msg;
    }
}
void ServidorPiezas::procesarSolicitud(Message* msg) {
    std::cout << "[SERVIDOR PIEZAS] Buscando inventario de " << msg->figura << " mitad " << msg->mitad << std::endl;

    Message* resp = new Message();
    resp->type = RESPONSE_PIECES;
    strcpy(resp->figura, msg->figura);
    resp->mitad = msg->mitad;

    std::string piezas;
    if (msg->mitad == 1) {
        piezas = "lego 1x2 : 4\nlego 2x2 : 2";
    } else {
        piezas = "lego 1x4 : 3\nlego 2x4 : 1";
    }
    strncpy(resp->message, piezas.c_str(), sizeof(resp->message) - 1);
    resp->message[sizeof(resp->message) - 1] = '\0';

    if (router) {
        pthread_mutex_lock(router->getMutex());
        router->getQueue().push(resp);
        pthread_cond_signal(router->getVC());
        pthread_mutex_unlock(router->getMutex());
        std::cout << "[SERVIDOR PIEZAS] Enviando piezas al router" << std::endl;
    } else {
        std::cout << "[SERVIDOR PIEZAS] ERROR: router no conectado" << std::endl;
        delete resp;
    }
}


pthread_mutex_t* ServidorPiezas::getMutex() {
    return &mutex;
}

pthread_cond_t* ServidorPiezas::getVC() {
    return &cond;
}

std::queue<Message*>& ServidorPiezas::getQueue() {
    return queue;
}