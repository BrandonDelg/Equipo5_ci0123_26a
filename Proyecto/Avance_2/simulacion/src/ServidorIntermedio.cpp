/**
 * @file ServidorIntermedio.cpp
 * @brief Implementación del servidor intermedio
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
    // std::cout << "[" << id << "] " << linea << std::endl;
    pthread_mutex_unlock(&logMutex);
}

void ServidorIntermedio::Connect(Cliente* c) {
    cliente = c;
}

void ServidorIntermedio::ConnectServidor(ServidorPiezas* s) {
    servidor = s;
    sendToServer("01|");
}

std::string ServidorIntermedio::getId() const {
    return id;
}

void ServidorIntermedio::sendHeartbeat() {
    if (!servidor) {
        logEvento("ERROR", "HEARTBEAT_ERROR", "Puntero SP nulo en sendHeartbeat");
        return;
    }

    logEvento("INFO", "HEARTBEAT_SENT", "Enviando HEARTBEAT a SP");
    sendToServer("03|");
}

void ServidorIntermedio::handleHeartbeat(bool alive){
    if(alive){
        logEvento("INFO", "ALIVE_RECEIVED", "ALIVE recibido");
    } else {
        logEvento("WARN", "HEARTBEAT_NO_RESPONSE", "Sin respuesta ALIVE");
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
    pthread_cond_t* cond_client = cliente->getVC();

    while (true) {
        pthread_mutex_lock(mutex_client);
        while (cliente->getQueue().empty()) {
            pthread_cond_wait(cond_client, mutex_client);
        }

        std::string msg = cliente->getQueue().front();
        cliente->getQueue().pop();
        pthread_mutex_unlock(mutex_client);
        int tipo = parser.getTipo(msg);
        if (tipo == 01) {
            logEvento("INFO","REGISTER", "Servidor de piezas registrado correctamente");
            continue;
        }
        if (tipo == 02) {
            std::cout << "[ROUTER] Solicitud para cerrar el servidor procesada" << std::endl;
            logEvento("INFO", "CLOSE", "Solicitud de cierre recibida del cliente");
            if (servidor) {
                sendToServer("02|");
            }
            break;
        }
        if (tipo != 10 && tipo != 14 && tipo != 90) {
            logEvento("ERROR", "UNSUPPORTED_TYPE", "Tipo no soportado recibido del cliente: " + std::to_string(tipo));
            // sendToClient("102|;");
            continue;
        }
        if(tipo == 90){
            logEvento("INFO", "ALIVE", "Recibido mensaje de ALIVE del servidor de piezas");
            continue;
        }
        std::cout << "[ROUTER] Recibe solicitud del cliente" << std::endl;
        std::cout << "[ROUTER] Consultando servidor de piezas" << std::endl;
        logEvento("INFO", "CLIENT_REQUEST", "Solicitud tipo=" + std::to_string(tipo) + " recibida del cliente");

        if (!sendToServer(msg)) {
            logEvento("ERROR", "SP_UNAVAILABLE", "Servidor de piezas no disponible para tipo=" + std::to_string(tipo));
            sendToClient("102|");
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

Cliente* ServidorIntermedio::getCliente(){
    return cliente;
};
