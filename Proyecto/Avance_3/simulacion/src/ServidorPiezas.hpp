/**
 * @file ServidorPiezas.hpp
 * @brief Definición de la clase Servidor de Piezas
 */
#pragma once
#include <queue>
#include <string>
#include <pthread.h>

class ServidorIntermedio;

/**
 * @brief Servidor donde se almacenan las figuras
 *
 * Recibe mensajes v3.0 (P/R/, P/G/fig/n) del servidor intermedio y responde
 * con P/D/datos, P/D/400 o P/D/404. Las señales internas con el SI
 * (01|, 02|, 03|, 90|) no cambian.
 */
class ServidorPiezas {
    public:
        /**
         * @brief Constructor del servidor de piezas con valores por defecto
         */
        ServidorPiezas();

        /**
         * @brief Constructor del servidor de piezas con id de isla
         *
         * @param id Identificador de la isla del servidor
         */
        ServidorPiezas(const std::string& id);

        /**
         * @brief Destructor del servidor de piezas
         */
        ~ServidorPiezas();

        /**
         * @brief Se conecta al servidor intermedio
         *
         * @param r Puntero al servidor intermedio que se va a conectar
         */
        void Connect(ServidorIntermedio* r);

        bool sendRespuesta(const std::string& msg);
        bool sendToRouter(const std::string& msg);

        /**
         * @brief Responde al HEARTBEAT enviado por el servidor intermedio
         *
         * Envía 90| (ALIVE) al SI si el SP está activo.
         *
         * @return true si respondió ALIVE, false si no
         */
        bool responderHeartbeat();

        /**
         * @brief Escucha mensajes provenientes del servidor intermedio
         *
         * Maneja señales internas (01|, 02|, 03|) y mensajes v3.0 (P/R/, P/G/).
         */
        void listen();

        /**
         * @brief Detiene el servidor de piezas
         */
        void Stop();

        /**
         * @brief Procesa una solicitud v3.0 y responde con P/D/...
         *
         * @param msg Mensaje v3.0 recibido (P/R/ o P/G/fig/n)
         */
        void procesarSolicitud(std::string msg);

        /**
         * @brief Devuelve el mutex de la cola
         *
         * @return Puntero a mutex
         */
        pthread_mutex_t* getMutex();

        /**
         * @brief Devuelve variable de condición de la cola
         *
         * @return Puntero a variable de condición
         */
        pthread_cond_t* getVC();

        /**
         * @brief Devuelve la cola de mensajes
         *
         * @return Referencia a cola de mensajes
         */
        std::queue<std::string>& getQueue();

        /**
         * @brief Devuelve el identificador de isla del servidor
         *
         * @return Identificador de isla
         */
        std::string getIslaId() const;

        /**
         * @brief Devuelve el puerto del servidor
         *
         * @return Puerto del servidor
         */
        int getPuerto() const;

    private:
        ServidorIntermedio* router; /** Servidor intermediario */
        std::queue<std::string> queue; /** Cola de mensajes */
        pthread_mutex_t mutex; /** Mutex de la cola */
        pthread_cond_t cond; /** Variable de condición de la cola */
        bool running; /** Bandera que indica si el servidor está activo */
        std::string islaId; /** Identificador de la isla del servidor */
        int puerto; /** Puerto en el que escucha el servidor */
};
