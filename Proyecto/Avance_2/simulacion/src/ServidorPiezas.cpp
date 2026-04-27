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
    islaId = "isla-default";
    router = nullptr;
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);
}

ServidorPiezas::ServidorPiezas(const std::string& islaId) {
    running = true;
    this->islaId = islaId;
    router = nullptr;
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);
}

ServidorPiezas::~ServidorPiezas() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

void ServidorPiezas::Connect(ServidorIntermedio* r) {
    router = r;
    sendToRouter("01|");
}

bool ServidorPiezas::sendRespuesta(const std::string& msg) {
    if (!router) {
        return false;
    }
    pthread_mutex_lock(router->getMutex());
    router->getQueue().push(msg);
    pthread_cond_signal(router->getVC());
    pthread_mutex_unlock(router->getMutex());
    return true;
}

bool ServidorPiezas::sendToRouter(const std::string& msg) {
    if (!router || !router->getCliente()) {
        return false;
    }
    pthread_mutex_lock(router->getCliente()->getMutex());
    router->getCliente()->getQueue().push(msg);
    pthread_cond_signal(router->getCliente()->getVC());
    pthread_mutex_unlock(router->getCliente()->getMutex());
    return true;
}

std::string ServidorPiezas::getIslaId() const {
    return islaId;
}

int ServidorPiezas::getPuerto() const {
    return puerto;
}

bool ServidorPiezas::responderHeartbeat() {
    if (!running) {
        // std::cout << "[SERVIDOR PIEZAS] HEARTBEAT recibido pero servidor inactivo, no se responde ALIVE" << std::endl;
        sendToRouter("102|");
        return false;
    }
    sendToRouter("90|");
    return true;
}

void ServidorPiezas::listen() {
    while (running) {
        pthread_mutex_lock(&mutex);
        while (queue.empty()) {
            pthread_cond_wait(&cond, &mutex);
        }
        std::string msg = queue.front();
        queue.pop();
        pthread_mutex_unlock(&mutex);
        int tipo = parser.getTipo(msg);
        if (tipo == 01){
            std::cout << "[SERVIDOR PIEZAS] Recibida señal de registro del router" << std::endl;
        } else if (tipo == 02) {
            std::cout << "[SERVIDOR PIEZAS] Recibida señal de unregister" << std::endl;
            break;
        } else if (tipo == 03){
            // std::cout << "[SERVIDOR PIEZAS] Recibida solicitud de heartbeat" << std::endl;
            responderHeartbeat();
        } else if (tipo == 14 || tipo == 10) {
            std::cout << "[SERVIDOR PIEZAS] Recibida solicitud recurso" << std::endl;
            procesarSolicitud(msg);
        } else {
            std::cout << "[SERVIDOR PIEZAS] Tipo de mensaje no soportado: " << parser.getTipo(msg) << std::endl;
            if (router) {
                sendRespuesta("102|");
            }
        }
    }
}

void ServidorPiezas::Stop() {
    running = false;
    pthread_mutex_lock(&mutex);
    queue.push("02|");
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void ServidorPiezas::procesarSolicitud(std::string msg) {
    std::string codigo = "00";
    std::string cuerpo = "";
    std::cout << "[SERVIDOR PIEZAS] Procesando solicitud..." << std::endl;
    int tipo = parser.getTipo(msg);
    if ( tipo == 10) {
        codigo = "11";
        std::string figuras = "Perro,Gato,Ballena,Oveja,Carro";
        cuerpo = "figura="+figuras+";";
    } else if (tipo == 03){
        codigo = "90";
    } else if (tipo == 14) {
        codigo = "15";
        std::string figura = parser.getFigura(msg);
        int mitad = parser.getMitad(msg);
        std::cout << "[SERVIDOR PIEZAS] Buscando piezas de " << figura << std::endl;
        if(mitad < 1 || mitad > 2){
            codigo = "105";
        } else if (figura == "Perro") {
            if (mitad == 1) {
                cuerpo = "figura=" + figura + ";piezas=" 
                "lego 1x2 : 4\n"
                "lego 2x2 : 2\n"
                "lego 1x4 : 3\n;";
            } else if (mitad == 2) {
                cuerpo = "figura=" + figura + ";piezas="
                "lego 3x2 : 4\n"
                "lego 3x3 : 2\n"
                "lego 3x4 : 3\n;";
            } else {
                codigo = "105";
            }
        } else if (figura == "Gato") {
            if (mitad == 1) {
                cuerpo = "figura=" + figura + ";piezas="
                "lego 5x2 : 4\n"
                "lego 4x2 : 2\n;";
            } else if (mitad == 2) {
                cuerpo = "figura=" + figura + ";piezas="
                "lego 6x2 : 4\n"
                "lego 2x2 : 2\n;";
            } else {
                codigo = "105";
            }

        } else if (figura == "Ballena") {
            if (mitad == 1) {
                cuerpo = "figura=" + figura + ";piezas="
                "lego 3x2 : 4\n"
                "lego 2x2 : 2\n"
                "lego 1x5 : 3\n;";
            } else if (mitad == 2) {
                cuerpo = "figura=" + figura + ";piezas="
                "lego 1x2 : 4\n"
                "lego 4x2 : 2\n"
                "lego 3x4 : 3\n;";
            } else {
                codigo = "105";
            }

        } else if (figura == "Oveja") {
            if (mitad == 1) {
                cuerpo = "figura=" + figura + ";piezas="
                "lego 2x2 : 2\n"
                "lego 1x5 : 3\n"
                "lego 1x2 : 4\n;";
            } else if (mitad == 2) {
                cuerpo = "figura=" + figura + ";piezas="
                "lego 1x2 : 4\n"
                "lego 4x2 : 2\n"
                "lego 3x4 : 3\n;";
            } else {
                codigo = "105";
            }

        } else if (figura == "Carro") {
            if (mitad == 1) {
                cuerpo = "figura=" + figura + ";piezas="
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
                "Medium Stone Gray Plate 1x2 with Horizontal Clips (Open O Clips) : 2\n;";
            } else if (mitad == 2) {
                cuerpo = "figura=" + figura + ";piezas="
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
                "Flat Silver Wheel Rim 11x6 with Hub Cap : 4\n;";
            } else {
                codigo = "105";
            }

        } else {
            codigo = "104";
        }

    } else {
        codigo="107";
    }
    std::string resp = codigo + "|" + cuerpo;

    std::cout << "[SERVIDOR PIEZAS] " << resp << std::endl;

    if (router) {
        sendRespuesta(resp);
        std::cout << "[SERVIDOR PIEZAS] Enviando información al router " << codigo << std::endl;
    } else {
        std::string msgError = "ERROR: router no conectado";
        sendRespuesta("102|");
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
