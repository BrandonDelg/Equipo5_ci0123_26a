/**
 * @file ServidorPiezas.hpp
 * @brief Definición de la clase Servidor de Piezas
 */
#include <queue>
#include <pthread.h>

#include "Parser.hpp"

class ServidorIntermedio;
struct Message;

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
         * @brief Constructor del servidor de piezas
         */
        ServidorPiezas();
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
        /**
         * @brief Escucha mensajes provenientes del servidor intermedio
         *
         * Se encarga de manejar todas las solicitudes que vienen 
         * del servidor intermedio y de llamar al metodo procesarSolicitud()
         */
        void listen();
        void Stop();
        /**
         * @brief Se encarga de procesar las solicitudes
         *
         * Procesa, valida y responde las solicitudes. Maneja 
         * posibles errores con las solicitudes y retorno las 
         * figuras
         *
         * @param msg Puntero a mensaje a procesar
         */
        void procesarSolicitud(std::string msg);
        pthread_mutex_t* getMutex();
        pthread_cond_t* getVC();
        std::queue<std::string>& getQueue();

    private:
        ServidorIntermedio* router; /** Servidor intermediario */
        Parser parser;
        std::queue<std::string> queue; /** Cola de mensajes */
        pthread_mutex_t mutex; /** Mutex de la cola */
        pthread_cond_t cond; /** Variable de condición de la cola */
        bool running; /** Bandera que indica si el servidor esta activo */
};
