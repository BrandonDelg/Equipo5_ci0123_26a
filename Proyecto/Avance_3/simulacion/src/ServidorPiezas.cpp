/**
 * @file ServidorPiezas.cpp
 * @brief Implementación de la clase Servidor de Piezas — protocolo v3.0
 *
 * Todo el tráfico SI↔SP usa protocolo v3.0:
 *   P/C/        registro (Connect)
 *   P/Q/        cierre   (Quit)
 *   P/H/        heartbeat
 *   P/A/        ALIVE (respuesta al heartbeat)
 *   P/R/        solicitud lista (reenviado desde cliente)
 *   P/G/fig/n   solicitud figura (reenviado desde cliente)
 *   P/D/datos   respuesta con datos
 *   P/D/400     error de solicitud
 *   P/D/404     figura no encontrada
 */
#include "ServidorPiezas.hpp"
#include "ServidorIntermedio.hpp"
#include "Cliente.hpp"
#include <iostream>
#include <stdexcept>

ServidorPiezas::ServidorPiezas() {
    running = true;
    islaId  = "isla-default";
    router  = nullptr;
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);
}

ServidorPiezas::ServidorPiezas(const std::string& islaId) {
    running      = true;
    this->islaId = islaId;
    router       = nullptr;
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);
}

ServidorPiezas::~ServidorPiezas() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

void ServidorPiezas::Connect(ServidorIntermedio* r) {
    router = r;
    sendToRouter("P/C/");  // v3.0: registro
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
        return false;
    }
    sendToRouter("P/A/");
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

        if (msg.size() < 3 || msg[0] != 'P' || msg[1] != '/') {
            std::cout << "[SP] Mensaje no reconocido: " << msg << std::endl;
            sendRespuesta("P/D/400");
            continue;
        }

        char cmd = msg[2];

        switch (cmd) {
            case 'C':
                std::cout << "[SP] Registro del router recibido (P/C/)" << std::endl;
                break;

            case 'Q':
                std::cout << "[SP] Señal de cierre recibida (P/Q/)" << std::endl;
                return;

            case 'H':
                responderHeartbeat();
                break;

            case 'R':
            case 'G':
                procesarSolicitud(msg);
                break;

            default:
                std::cout << "[SP] Comando no soportado: " << cmd << std::endl;
                sendRespuesta("P/D/400");
                break;
        }
    }
}

void ServidorPiezas::Stop() {
    running = false;
    pthread_mutex_lock(&mutex);
    queue.push("P/Q/");
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void ServidorPiezas::procesarSolicitud(std::string msg) {
    std::string resp;
    char cmd = msg[2];

    if (cmd == 'R') {
        resp = "P/D/Perro,Gato,Ballena,Oveja,Carro";

    } else if (cmd == 'G') {
        size_t slash = msg.find('/', 4);

        if (slash == std::string::npos) {
            resp = "P/D/400";
        } else {
            std::string figura   = msg.substr(4, slash - 4);
            std::string mitadStr = msg.substr(slash + 1);
            int mitad = 0;

            try {
                mitad = std::stoi(mitadStr);
            } catch (const std::exception&) {
                sendRespuesta("P/D/400");
                return;
            }

            if (mitad < 1 || mitad > 2) {
                resp = "P/D/400";
            } else if (figura == "Perro") {
                if (mitad == 1) {
                    resp = "P/D/"
                           "lego 1x2 : 4\n"
                           "lego 2x2 : 2\n"
                           "lego 1x4 : 3\n";
                } else {
                    resp = "P/D/"
                           "lego 3x2 : 4\n"
                           "lego 3x3 : 2\n"
                           "lego 3x4 : 3\n";
                }
            } else if (figura == "Gato") {
                if (mitad == 1) {
                    resp = "P/D/"
                           "lego 5x2 : 4\n"
                           "lego 4x2 : 2\n";
                } else {
                    resp = "P/D/"
                           "lego 6x2 : 4\n"
                           "lego 2x2 : 2\n";
                }
            } else if (figura == "Ballena") {
                if (mitad == 1) {
                    resp = "P/D/"
                           "lego 3x2 : 4\n"
                           "lego 2x2 : 2\n"
                           "lego 1x5 : 3\n";
                } else {
                    resp = "P/D/"
                           "lego 1x2 : 4\n"
                           "lego 4x2 : 2\n"
                           "lego 3x4 : 3\n";
                }
            } else if (figura == "Oveja") {
                if (mitad == 1) {
                    resp = "P/D/"
                           "lego 2x2 : 2\n"
                           "lego 1x5 : 3\n"
                           "lego 1x2 : 4\n";
                } else {
                    resp = "P/D/"
                           "lego 1x2 : 4\n"
                           "lego 4x2 : 2\n"
                           "lego 3x4 : 3\n";
                }
            } else if (figura == "Carro") {
                if (mitad == 1) {
                    resp = "P/D/"
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
                           "Red Plate 1x2 : 1\n"
                           "Red Plate 1x2 with 1 Stud (with Groove and Bottom Stud Holder) : 1\n"
                           "Medium Stone Gray Plate 1x2 with Horizontal Clips (Open O Clips) : 2\n";
                } else {
                    resp = "P/D/"
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
                           "Medium Stone Gray Tile 1x2 (with Bottom Groove) : 1\n"
                           "Black Tile 2x2 with Studs on Edge : 4\n"
                           "Dark Red Tile 2x4 : 1\n"
                           "Red Tile 2x4 : 1\n"
                           "Black Tire 14x6 : 4\n"
                           "Flat Silver Wheel Rim 11x6 with Hub Cap : 4\n";
                }
            } else {
                resp = "P/D/404";
            }
        }
    } else {
        resp = "P/D/400";
    }

    std::cout << "[SP] Respuesta: "
              << resp.substr(0, std::min(static_cast<int>(resp.size()), 50))
              << (resp.size() > 50 ? "..." : "") << std::endl;

    if (router) {
        sendRespuesta(resp);
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
