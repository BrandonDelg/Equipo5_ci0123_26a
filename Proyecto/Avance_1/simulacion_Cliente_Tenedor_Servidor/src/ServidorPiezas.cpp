/**
 * @file ServidorPiezas.cpp
 * @brief Implementación de la clase Servidor de Piezas
 */
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
        if(msg->type == REQUEST_FIGURE || msg->type == REQUEST_LIST) {
            procesarSolicitud(msg);
        }
        delete msg;
    }
}
void ServidorPiezas::procesarSolicitud(Message* msg) {
    std::string piezas;
    std::string figura = std::string(msg->figura);
    std::cout << "[SERVIDOR PIEZAS] Procesando solicitud: " << figura << std::endl;
    bool error = false;
    bool errorF = false;
    bool list = false;
    if (figura == "GET_FIGURES" && msg->type == REQUEST_LIST) {
        piezas =
        "Listado de Figuras en el servidor :)\n"
        "- Perro \n"
        "- Gato \n"
        "- Ballena \n"
        "- Oveja \n";
        list = true;
    } else if (figura.find("Figure_") == 0 && msg->type == REQUEST_FIGURE) {

        std::cout << "[SERVIDOR PIEZAS] Buscando piezas de " << figura << std::endl;
        
        if (figura == "Figure_Perro") {
            if (msg->mitad == 1) {
                piezas =
                "lego 1x2 : 4\n"
                "lego 2x2 : 2\n"
                "lego 1x4 : 3\n";
            } else if (msg->mitad == 2) {
                piezas =
                "lego 3x2 : 4\n"
                "lego 3x3 : 2\n"
                "lego 3x4 : 3\n";
            } else {
                piezas = "No existe la mitad indicada:\n";
                error = true;
            }
        } else if (figura == "Figure_Gato") {
            if (msg->mitad == 1) {
                piezas =
                "lego 5x2 : 4\n"
                "lego 4x2 : 2\n";
            } else if (msg->mitad == 2) {
                piezas =
                "lego 6x2 : 4\n"
                "lego 2x2 : 2\n";
            } else {
                piezas = "No existe la mitad indicada:\n";
                error = true;
            }

        } else if (figura == "Figure_Ballena") {
            if (msg->mitad == 1) {
                piezas =
                "lego 3x2 : 4\n"
                "lego 2x2 : 2\n"
                "lego 1x5 : 3\n";
            } else if (msg->mitad == 2) {
                piezas = 
                "lego 1x2 : 4\n"
                "lego 4x2 : 2\n"
                "lego 3x4 : 3\n";
            } else {
                piezas = "No existe la mitad indicada:\n";
                error = true;
            }

        } else if (figura == "Figure_Oveja") {
            if (msg->mitad == 1) {
                piezas =
                "lego 2x2 : 2\n"
                "lego 1x5 : 3\n"
                "lego 1x2 : 4\n";
            } else if (msg->mitad == 2) {
                piezas =
                "lego 1x2 : 4\n"
                "lego 4x2 : 2\n"
                "lego 3x4 : 3\n";
            } else {
                piezas = "No existe la mitad indicada:\n";
                error = true;
            }

        } else {
            piezas = "La figura ingresada no es valida!\n";
            errorF = true;
        }

    } else {
        piezas = "Comando Invalido!\n";
        error = true;
    }

    Message* resp = new Message();
    if (error) {
        resp->type = ERROR;
    } else if (errorF) {
        msg->type = ERROR_NOT_FOUND;
    } else if (list){
        resp->type = RESPONSE_FIGURES;
    } else {
        resp->type = RESPONSE_PIECES;
    }
    error = false;
    list = false;
    strcpy(resp->figura, msg->figura);
    resp->mitad = msg->mitad;

    strncpy(resp->message, piezas.c_str(), sizeof(resp->message) - 1);
    resp->message[sizeof(resp->message) - 1] = '\0';

    if (router) {
        pthread_mutex_lock(router->getMutex());
        router->getQueue().push(resp);
        pthread_cond_signal(router->getVC());
        pthread_mutex_unlock(router->getMutex());

        std::cout << "[SERVIDOR PIEZAS] Enviando informacion al router" << std::endl;
    } else {
        std::string msgError = "ERROR: router no conectado";
        resp->type = ERROR_NOT_CONECTION;
        strncpy(resp->message, msgError.c_str(), sizeof(resp->message) - 1);
        pthread_mutex_lock(router->getMutex());
        router->getQueue().push(resp);
        pthread_cond_signal(router->getVC());
        pthread_mutex_unlock(router->getMutex());
        std::cout << "[SERVIDOR PIEZAS]" << msgError << std::endl;
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
