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
        std::string msg = queue.front();
        queue.pop();
        pthread_mutex_unlock(&mutex);
        
        if(parser.getTipo(msg) == 02) {
            std::cout << "[SERVIDOR PIEZAS] Recibida señal de cierre" << std::endl;
            break;
        }
        if(parser.getTipo(msg) == 14 || parser.getTipo(msg) == 10) {
            procesarSolicitud(msg);
        }
    }
}
void ServidorPiezas::procesarSolicitud(std::string msg) {
    std::string piezas;
    std::string figura;
    std::cout << "[SERVIDOR PIEZAS] Procesando solicitud..." << std::endl;
    bool error = false;
    bool errorF = false;
    bool list = false;
    if (parser.getTipo(msg) == 10) {
        piezas =
        "Listado de Figuras en el servidor :)\n"
        "- Perro \n"
        "- Gato \n"
        "- Ballena \n"
        "- Oveja \n"
        "- Carro \n";
        list = true;
    } else if (parser.getTipo(msg) == 14) {
        
        figura = parser.getFigura(msg);
        std::cout << "[SERVIDOR PIEZAS] Buscando piezas de " << figura << std::endl;
        
        if (figura == "Perro") {
            if (parser.getMitad(msg) == 1) {
                piezas =
                "lego 1x2 : 4\n"
                "lego 2x2 : 2\n"
                "lego 1x4 : 3\n";
            } else if (parser.getMitad(msg) == 2) {
                piezas =
                "lego 3x2 : 4\n"
                "lego 3x3 : 2\n"
                "lego 3x4 : 3\n";
            } else {
                piezas = "No existe la mitad indicada:\n";
                error = true;
            }
        } else if (figura == "Gato") {
            if (parser.getMitad(msg) == 1) {
                piezas =
                "lego 5x2 : 4\n"
                "lego 4x2 : 2\n";
            } else if (parser.getMitad(msg) == 2) {
                piezas =
                "lego 6x2 : 4\n"
                "lego 2x2 : 2\n";
            } else {
                piezas = "No existe la mitad indicada:\n";
                error = true;
            }

        } else if (figura == "Ballena") {
            if (parser.getMitad(msg) == 1) {
                piezas =
                "lego 3x2 : 4\n"
                "lego 2x2 : 2\n"
                "lego 1x5 : 3\n";
            } else if (parser.getMitad(msg) == 2) {
                piezas = 
                "lego 1x2 : 4\n"
                "lego 4x2 : 2\n"
                "lego 3x4 : 3\n";
            } else {
                piezas = "No existe la mitad indicada:\n";
                error = true;
            }

        } else if (figura == "Oveja") {
            if (parser.getMitad(msg) == 1) {
                piezas =
                "lego 2x2 : 2\n"
                "lego 1x5 : 3\n"
                "lego 1x2 : 4\n";
            } else if (parser.getMitad(msg) == 2) {
                piezas =
                "lego 1x2 : 4\n"
                "lego 4x2 : 2\n"
                "lego 3x4 : 3\n";
            } else {
                piezas = "No existe la mitad indicada:\n";
                error = true;
            }

        } else if (figura == "Carro") {
            if (parser.getMitad(msg) == 1) {
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
            } else if (parser.getMitad(msg) == 2) {
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
            piezas = "La figura ingresada no es válida!\n";
            errorF = true;
        }

    } else {
        piezas = "Comando Inválido!\n";
        error = true;
    }

    std::string resp;
    if (error) {
        resp = "102|";
    } else if (errorF) {
        resp = "16|";
    } else if (list){
        resp = "11|";
    } else {
        resp = "15|";
    }
    error = false;
    list = false;
    resp += "figura=" + figura + ";mensaje=" + piezas;
    std::cout << "[SERVIDOR PIEZAS] " << resp << std::endl;

    if (router) {
        pthread_mutex_lock(router->getMutex());
        router->getQueue().push(resp);
        pthread_cond_signal(router->getVC());
        pthread_mutex_unlock(router->getMutex());

        std::cout << "[SERVIDOR PIEZAS] Enviando información al router" << std::endl;
    } else {
        std::string msgError = "ERROR: router no conectado";
        resp = "102|";
        pthread_mutex_lock(router->getMutex());
        router->getQueue().push(resp);
        pthread_cond_signal(router->getVC());
        pthread_mutex_unlock(router->getMutex());
        std::cout << "[SERVIDOR PIEZAS]" << msgError << std::endl;
    }
}

pthread_mutex_t* ServidorPiezas::getMutex() {
    return &mutex;
}

pthread_cond_t* ServidorPiezas::getVC() {
    return &cond;
}

std::queue<std::string>& ServidorPiezas::getQueue() {
    return queue;
}
