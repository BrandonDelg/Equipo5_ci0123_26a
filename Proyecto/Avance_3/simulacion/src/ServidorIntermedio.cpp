/**
 * @file ServidorIntermedio.cpp
 * @brief Implementación del servidor intermedio — protocolo v3.0
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

#include "ServidorIntermedio.hpp"
#include "ServidorPiezas.hpp"
#include "Cliente.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <ctime>

ServidorIntermedio::ServidorIntermedio() {
    id = "SI-default";
    cliente = nullptr;
    servidor = nullptr;
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);
    pthread_mutex_init(&peerMutex, nullptr);
    pthread_cond_init(&peerCond, nullptr);
    pthread_mutex_init(&logMutex, nullptr);
    logFile.open("bitacora.log", std::ios::app);
}

ServidorIntermedio::ServidorIntermedio(const std::string& islaId) {
    id = islaId;
    cliente = nullptr;
    servidor = nullptr;
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);
    pthread_mutex_init(&peerMutex, nullptr);
    pthread_cond_init(&peerCond, nullptr);
    pthread_mutex_init(&logMutex, nullptr);
    logFile.open("bitacora.log", std::ios::app);
}

ServidorIntermedio::~ServidorIntermedio() {
    if (logFile.is_open()) logFile.close();
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&peerMutex);
    pthread_cond_destroy(&peerCond);
    pthread_mutex_destroy(&logMutex);
}

std::string ServidorIntermedio::timestamp() {
    std::time_t now = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

void ServidorIntermedio::logEvento(const std::string& tipo, const std::string& evento, const std::string& detalle) {
    pthread_mutex_lock(&logMutex);
    std::string linea = "[" + timestamp() + "] [" + tipo + "] [" + evento + "]: " + detalle;
    if (logFile.is_open()) {
        logFile << linea << std::endl;
    }
    pthread_mutex_unlock(&logMutex);
}

void ServidorIntermedio::Connect(Cliente* c) {
    cliente = c;
}

void ServidorIntermedio::ConnectServidor(ServidorPiezas* s) {
    servidor = s;
    sendToServer("P/C/");
}

std::string ServidorIntermedio::getId() const {
    return id;
}

void ServidorIntermedio::sendHeartbeat() {
    if (!servidor) {
        logEvento("ERROR", "HEARTBEAT_ERROR", "Puntero SP nulo en sendHeartbeat");
        return;
    }
    logEvento("INFO", "HEARTBEAT_SENT", "Enviando P/H/ a SP");
    sendToServer("P/H/");
}

void ServidorIntermedio::handleHeartbeat(bool alive) {
    if (alive) {
        logEvento("INFO", "ALIVE_RECEIVED", "P/A/ recibido del SP");
    } else {
        logEvento("WARN", "HEARTBEAT_NO_RESPONSE", "Sin respuesta P/A/ del SP");
    }
}

void ServidorIntermedio::sendToClient(const std::string& msg) {
    pthread_mutex_lock(&mutex);
    respuestas.push(msg);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

bool ServidorIntermedio::sendToServer(const std::string& msg) {
    if (!servidor) {
        return false;
    }
    pthread_mutex_lock(servidor->getMutex());
    servidor->getQueue().push(msg);
    pthread_cond_signal(servidor->getVC());
    pthread_mutex_unlock(servidor->getMutex());
    return true;
}

void ServidorIntermedio::listen() {
    pthread_mutex_t* mutex_client = cliente->getMutex();
    pthread_cond_t*  cond_client  = cliente->getVC();

    while (true) {
        pthread_mutex_lock(mutex_client);
        while (cliente->getQueue().empty()) {
            pthread_cond_wait(cond_client, mutex_client);
        }
        std::string msg = cliente->getQueue().front();
        cliente->getQueue().pop();
        pthread_mutex_unlock(mutex_client);

        if (msg.size() < 3 || msg[0] != 'P' || msg[1] != '/') {
            logEvento("ERROR", "UNSUPPORTED_MSG", "Mensaje no reconocido: " + msg);
            continue;
        }

        char cmd = msg[2];

        switch (cmd) {
            case 'C':
                logEvento("INFO", "REGISTER", "Servidor de piezas registrado (P/C/)");
                break;

            case 'A':
                logEvento("INFO", "ALIVE_RECEIVED", "P/A/ recibido del SP");
                break;

            case 'Q':
                std::cout << "[ROUTER] Solicitud de cierre del cliente (P/Q/)" << std::endl;
                logEvento("INFO", "CLOSE", "Solicitud de cierre del cliente procesada");
                if (servidor) {
                    sendToServer("P/Q/");
                }
                return;

            case 'R':
            case 'G':
                std::cout << "[ROUTER] Recibe del cliente: " << msg
                          << " — reenvía al SP" << std::endl;
                logEvento("INFO", "CLIENT_REQUEST",
                          "Solicitud cmd=" + std::string(1, cmd) + " recibida");
                if (!sendToServer(msg)) {
                    logEvento("ERROR", "SP_UNAVAILABLE",
                              "SP no disponible para cmd=" + std::string(1, cmd));
                    sendToClient("P/D/400");
                }
                break;

            default:
                logEvento("ERROR", "UNSUPPORTED_CMD",
                          "Comando no soportado: " + std::string(1, cmd));
                sendToClient("P/D/400");
                break;
        }
    }
}

pthread_mutex_t* ServidorIntermedio::getMutex() {
    return &mutex;
}

pthread_cond_t* ServidorIntermedio::getVC() {
    return &cond;
}

std::queue<std::string>& ServidorIntermedio::getQueue() {
    return respuestas;
}

std::queue<std::string>& ServidorIntermedio::getPeerQueue() {
    return peerQueue;
}

pthread_mutex_t* ServidorIntermedio::getPeerMutex() {
    return &peerMutex;
}

pthread_cond_t* ServidorIntermedio::getPeerVC() {
    return &peerCond;
}

Cliente* ServidorIntermedio::getCliente() {
    return cliente;
}
