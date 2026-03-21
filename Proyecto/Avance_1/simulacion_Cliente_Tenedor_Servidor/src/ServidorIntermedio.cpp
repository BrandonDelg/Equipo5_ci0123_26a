#include "ServidorIntermedio.hpp"
#include "ServidorPiezas.hpp"
#include "Cliente.hpp"
#include <iostream>
#include <cstring>

ServidorIntermedio::ServidorIntermedio() {
    cliente = nullptr;
    servidor = nullptr;
    pthread_mutex_init(&mutex,nullptr);
    pthread_cond_init(&cond,nullptr);
}

ServidorIntermedio::~ServidorIntermedio() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

void ServidorIntermedio::Connect(Cliente* c) {
    cliente = c;
}

void ServidorIntermedio::ConnectServidor(ServidorPiezas* s) {
    servidor = s;
}

void ServidorIntermedio::listen() {
    pthread_mutex_t* mutex_client = cliente->getMutex();
    pthread_cond_t* cond_client = cliente->getVC();

    while(true) {
        pthread_mutex_lock(mutex_client);
        while(cliente->getQueue().empty()) {
            pthread_cond_wait(cond_client, mutex_client);
        }

        Message* msg = cliente->getQueue().front();
        cliente->getQueue().pop();
        pthread_mutex_unlock(mutex_client);

        if(msg->type == CLOSE) {
            std::cout << "[ROUTER] Solicitud para cerrar el servidor procesada" << std::endl;
            if(servidor) {
                Message* cierre = new Message();
                cierre->type = CLOSE;
                pthread_mutex_lock(servidor->getMutex());
                servidor->getQueue().push(cierre);
                pthread_cond_signal(servidor->getVC());
                pthread_mutex_unlock(servidor->getMutex());
            }
            delete msg;
            break;
        }
        std::cout << "[ROUTER] Recibe solicitud del cliente" << std::endl;
        std::cout << "[ROUTER] Consultando servidor de piezas :)" << std::endl;

        if(servidor) {
            Message* copia_msg = new Message();
            copia_msg->type = msg->type;
            strcpy(copia_msg->figura, msg->figura);
            copia_msg->mitad = msg->mitad;

            pthread_mutex_lock(servidor->getMutex());
            servidor->getQueue().push(copia_msg);
            pthread_cond_signal(servidor->getVC());
            pthread_mutex_unlock(servidor->getMutex());
        } else {
            std::cout << "[ROUTER] ERROR: servidor de piezas no conectado" << std::endl;
        }
    }
}

pthread_mutex_t* ServidorIntermedio::getMutex() {
    return &mutex;
}

pthread_cond_t* ServidorIntermedio::getVC() {
    return &cond;
}

std::queue<Message*>& ServidorIntermedio::getQueue() {
    return respuestas;
}