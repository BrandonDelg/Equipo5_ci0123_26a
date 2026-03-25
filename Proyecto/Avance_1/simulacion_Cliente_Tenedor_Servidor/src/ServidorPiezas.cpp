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
        "- Oveja \n"
        "- Carro \n";
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

        } else if (figura == "Figure_Carro") {
            if (msg->mitad == 1) {
                piezas =
                "Medium Stone Gray Bar 1 : 2\n"
                "Black Bracket 1x1 with 1x1 Plate Down : 2\n"
                "Medium Stone Gray Bracket 1x2 with 12 Up : 1\n"
                "Dark Red Brick 1x2 with Bottom Tube : 1\n"
                "Red Brick 1x2 with Grille : 2\n"
                "Transparent Brick 1x2 without Bottom Tube : 2\n"
                "Red Brick 1x4 : 2\n"
                "Transparent Diamond : 2\n"
                "Black Mudguard Plate 2x4 with Arches with Hole : 2\n"
                "Red Panel 1x2x1 with Rounded Corners : 1\n"
                "Pearl Gold Plate 1x1 Round : 3\n"
                "Medium Stone Gray Plate 1x1 with Clip (Thick Ring) : 2\n"
                "Pearl Gold Plate 1x1 with Horizontal Clip (Thick Open O Clip) : 1\n"
                "Red PLate 1x2 : 1\n"
                "Red Plate 1x2 with 1 Stud (with Groove and Bottom Stud Holder) : 1\n"
                "Medium Stone Gray Plate 1x2 with Horizontal Clips (Open O Clips) : 2\n";
            } else if (msg->mitad == 2) {
                piezas =
                "Medium Stone Gray Plate 1x6 : 2\n"
                "Red Plate 1x8 : 2\n"
                "Black Plate 2x2 Corner : 2\n"
                "Medium Stone Gray Plate 2x2 with Two Wheel Holder Pins : 2\n"
                "Black Plate 2x4 : 1\n"
                "Black Slope 1x2 Curved : 2\n"
                "Black Slope 1x3 Curved : 2\n"
                "Reddish Brown Suitcase : 1\n"
                "Transparent Red Tile 1x1 Round : 2\n"
                "Pearl Gold Tile 1x1 Round with Crown Coin : 1\n"
                "Medium Stone Gray TIle 1x2 (with Bottom Groove) : 1\n"
                "Black Tile 2x2 with Studs on Edge : 4\n"
                "Dark Red Tile 2x4 : 1\n"
                "Red Tile 2x4 : 1\n"
                "Black Tire 14x6 : 4\n"
                "Flat Silver Wheel Rim 11x6 with Hub Cap : 4\n";
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
