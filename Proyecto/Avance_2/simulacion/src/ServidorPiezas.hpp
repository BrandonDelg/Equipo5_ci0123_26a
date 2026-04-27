/**
 * @file ServidorPiezas.hpp
 * @brief Definición de la clase Servidor de Piezas
 */
#pragma once
#include <queue>
#include <string>
#include <pthread.h>

#include "Parser.hpp"

class ServidorIntermedio;

/**
 * @brief Servidor donde se almacenan las figuras
 *
 * En este servidor se almacenan las piezas o figuras. Es
 * el encargado de responder con una figura o lista de figuras
 * cuando lo solicita el servidor intermedio. También se encarga
 * de validar todos los request y responder con mensajes informativos
 * o de error
 */
class ServidorPiezas {
    public:
        /**
         * @brief Constructor del servidor de piezas con valores por defecto
         */
        ServidorPiezas();

        /**
         * @brief Constructor del servidor de piezas con id de isla y puerto
         *
         * @param islaId Identificador de la isla del servidor
         * @param puerto Puerto en el que escucha el servidor
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
         * El intermediario llama a este método como parte del ciclo de heartbeat.
         * El SP responde ALIVE si está activo.
         *
         * @return true si el SP está activo y responde ALIVE, false si no puede responder
         */
        bool responderHeartbeat();

        /**
         * @brief Escucha mensajes provenientes del servidor intermedio
         *
         * Se encarga de manejar todas las solicitudes que vienen
         * del servidor intermedio y de llamar al metodo procesarSolicitud().
         * Al recibir cierre envía NOTIFY_DROP a través del router.
         */
        void listen();

        /**
         * @brief Detiene el servidor de piezas
         *
         * Coloca una señal de cierre en la cola para terminar el loop de listen().
         */
        void Stop();

        /**
         * @brief Se encarga de procesar las solicitudes
         *
         * Procesa, valida y responde las solicitudes. Maneja
         * posibles errores con las solicitudes y retorna las
         * figuras.
         *
         * @param msg Mensaje a procesar
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
        Parser parser;
        std::queue<std::string> queue; /** Cola de mensajes */
        pthread_mutex_t mutex; /** Mutex de la cola */
        pthread_cond_t cond; /** Variable de condición de la cola */
        bool running; /** Bandera que indica si el servidor está activo */
        std::string islaId; /** Identificador de la isla del servidor */
        int puerto; /** Puerto en el que escucha el servidor */
};
