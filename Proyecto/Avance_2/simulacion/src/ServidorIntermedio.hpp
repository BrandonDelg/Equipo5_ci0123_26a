/**
 * @file ServidorIntermedio.hpp
 * @brief Definición de la clase del servidor intermedio
 */
#pragma once
#include <queue>
#include <vector>
#include <string>
#include <pthread.h>
#include <fstream>

#include "Parser.hpp"

class Cliente;
class ServidorPiezas;

/**
 * @brief Clase Servidor Intermedio
 *
 * Se encarga de manejar la comunicación entre el cliente y
 * el servidor de piezas.
 */
class ServidorIntermedio {
    public:
        /**
         * @brief Constructor del servidor intermedio con id por defecto
         */
        ServidorIntermedio();

        /**
         * @brief Constructor del servidor intermedio con id de isla
         *
         * @param id Identificador de la isla del servidor
         */
        ServidorIntermedio(const std::string& id);

        /**
         * @brief Destructor del servidor intermedio
         */
        ~ServidorIntermedio();

        /**
         * @brief Se conecta al cliente
         *
         * @param c Puntero al cliente que se va a conectar
         */
        void Connect(Cliente* c);

        /**
         * @brief Se conecta al servidor de piezas local
         *
         * @param s Puntero al servidor de piezas que se va a conectar
         */
        void ConnectServidor(ServidorPiezas* s);


        /**
         * @brief Escucha mensajes del cliente y los reenvía al servidor de piezas
         *
         * Se encarga de recibir constantemente solicitudes del cliente
         * y mandárselas al servidor. Si recibe una respuesta del servidor
         * se la retorna al cliente. Valida tipos de mensaje y maneja errores
         * de protocolo.
         */
        void listen();

        /**
         * @brief Envía un HEARTBEAT al servidor de piezas registrado
         *
         * El intermediario inicia el ciclo: envía HEARTBEAT al SP y espera
         * respuesta ALIVE. Si no responde registra WARN en el log. Si falla
         * múltiples veces registra ERROR.
         *
         * @param sp Puntero al SP al que se envía el HEARTBEAT
         */
        void sendHeartbeat();
        void handleHeartbeat(bool alive);

        /**
         * @brief Devuelve el identificador del servidor
         *
         * @return Identificador de isla del servidor
         */
        std::string getId() const;

        /**
         * @brief Devuelve el mutex de la cola de respuestas al cliente
         *
         * @return Puntero a mutex
         */
        pthread_mutex_t* getMutex();

        /**
         * @brief Devuelve variable de condición de la cola de respuestas al cliente
         *
         * @return Puntero a variable de condición
         */
        pthread_cond_t* getVC();

        /**
         * @brief Devuelve la cola de respuestas al cliente
         *
         * @return Referencia a cola de mensajes
         */
        std::queue<std::string>& getQueue();

        /**
         * @brief Devuelve la cola de mensajes entre peers
         *
         * @return Referencia a cola de mensajes peer
         */
        std::queue<std::string>& getPeerQueue();

        /**
         * @brief Devuelve el mutex de la cola de mensajes entre peers
         *
         * @return Puntero a mutex
         */
        pthread_mutex_t* getPeerMutex();

        /**
         * @brief Devuelve la variable de condición de la cola de mensajes entre peers
         *
         * @return Puntero a variable de condición
         */
        pthread_cond_t* getPeerVC();


        Cliente* getCliente();

    private:

        /**
         * @brief Escribe una entrada en el archivo de log
         *
         * @param tipo Tipo de evento: INFO, WARN o ERROR
         * @param evento Nombre del evento
         * @param detalle Información adicional del evento
         */
        void logEvento(const std::string& tipo, const std::string& evento, const std::string& detalle);

        /**
         * @brief Genera un timestamp con fecha y hora actual
         *
         * @return String con formato YYYY-MM-DD HH:MM:SS
         */
        std::string timestamp();


        /**
         * @brief Coloca un mensaje en la cola de respuestas al cliente
         *
         * @param msg Mensaje a enviar al cliente
         */
        void sendToClient(const std::string& msg);

        /**
         * @brief Reenvía un mensaje al servidor de piezas local
         *
         * @param msg Mensaje a reenviar
         * @return true si el SP local está disponible, false si no
         */
        bool sendToServer(const std::string& msg);

        std::string id; /** Identificador de la isla del intermediario */
        Cliente* cliente; /** Cliente al que se va a conectar */
        ServidorPiezas* servidor; /** Servidor de piezas local */
        Parser parser;

        std::vector<ServidorIntermedio*> peers; /** Lista de servidores intermediarios conocidos */
        std::queue<std::string> respuestas; /** Cola de respuestas al cliente */
        pthread_mutex_t mutex; /** Mutex de la cola de respuestas */
        pthread_cond_t cond; /** Variable de condición de la cola de respuestas */

        std::queue<std::string> peerQueue; /** Cola de mensajes entre peers */
        pthread_mutex_t peerMutex; /** Mutex de la cola peer */
        pthread_cond_t peerCond; /** Variable de condición de la cola peer */

        std::ofstream logFile; /** Archivo de log de eventos */
        pthread_mutex_t logMutex; /** Mutex del archivo de log */
};
