#pragma once

/**
 * @file Cliente.hpp
 * @brief Definición de la clase Cliente
 */
#include <iostream>
#include <queue>
#include <pthread.h>

#include "Parser.hpp"

class ServidorIntermedio;

/**
 * @brief Clase cliente
 *
 * Actua como un cliente que solicita piezas al 
 * servidor intermedio o tenedor
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
         * @param package Puntero al mensaje empaquetado
         */
        void send_to_server(std::string msg);
        /**
         * @brief Recibe mensajes del servidor
         *
         * @return devueve false si el mensaje es un close en caso contrario true
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

    private:
        Parser parser;
        ServidorIntermedio* server; /** Servidor intermedio con el que se comunica el cliente */
        std::queue<std::string> messageQueue; /** Cola de mensajes */
        pthread_mutex_t queueMutex; /** Mutex de la cola */
        pthread_cond_t queueCond; /** Condición de variable de la cola */
};
