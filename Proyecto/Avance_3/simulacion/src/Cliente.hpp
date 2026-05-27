#pragma once

/**
 * @file Cliente.hpp
 * @brief Definición de la clase Cliente
 */
#include <iostream>
#include <sstream>
#include <queue>
#include <pthread.h>

class ServidorIntermedio;

/**
 * @brief Clase cliente
 *
 * Actua como un cliente que solicita piezas al
 * servidor intermedio usando el protocolo v3.0 (P/Cmd/args).
 */
class Cliente {
    public:
        /**
         * @brief Constructor del cliente
         */
        Cliente();
        /**
         * @brief Destructor del cliente
         */
        ~Cliente();
        /**
         * @brief Se conecta al servidor
         *
         * @param serv Puntero al servidor que se va a conectar
         */
        void Connect(ServidorIntermedio* serv);
        /**
         * @brief Manda una solicitud al servidor
         *
         * @param msg Mensaje en protocolo v3.0 (P/R/, P/G/fig/n, P/Q/)
         */
        void send_to_server(std::string msg);
        /**
         * @brief Recibe mensajes del servidor
         *
         * @return false si el servidor cerró la conexión, true en caso contrario
         */
        bool receive_from_server();
        /**
         * @brief Devuelve la cola de mensajes
         *
         * @return Referencia a cola de mensajes
         */
        std::queue<std::string>& getQueue();
        /**
         * @brief Devuelve variable de condición de la cola
         *
         * @return Puntero a variable de condición
         */
        pthread_cond_t* getVC();
        /**
         * @brief Devuelve el mutex de la cola
         *
         * @return Puntero a mutex
         */
        pthread_mutex_t* getMutex();

        bool running;
    private:
        ServidorIntermedio* server; /** Servidor intermedio con el que se comunica */
        std::queue<std::string> messageQueue; /** Cola de mensajes salientes */
        pthread_mutex_t queueMutex; /** Mutex de la cola */
        pthread_cond_t queueCond; /** Variable de condición de la cola */
};
