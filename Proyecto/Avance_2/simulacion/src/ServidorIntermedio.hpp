/**
 * @file ServidorIntermedio.hpp
 * @brief Definición de la clase del servidor intermedio
 */
#include <queue>
#include <pthread.h>

#include "Parser.hpp"

class Cliente;
class ServidorPiezas;
struct Message;

/**
 * @brief Clase Servidor Intermedio
 *
 * Se encargo de manejar la comunicación entre el cliente y 
 * el servidor de piezas
 */
class ServidorIntermedio {
    public:
        /**
         * @brief Constructor del servidor intermedio
         */
        ServidorIntermedio();
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
         * @brief Se conecta al servidor de piezas
         *
         * @param s Puntero al servidor de piezas que se va a conectar
         */
        void ConnectServidor(ServidorPiezas* s);
        /**
         * @brief Escucha mensajes del cliente y responde al servidor
         *
         * Se encarga de recibar constantemente solicitudes del cliente 
         * y mandarselas al servidor, al mismo tiempo si recibe una respuesta del servidor se la retorno al cliente
         */
        void listen();
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

    private:
        Cliente* cliente; /** Cliente al que se va a conectar */
        ServidorPiezas* servidor; /** Servidor de piezas al que se va a conectar */
        Parser parser;
        std::queue<std::string> respuestas; /** Cola de respuestas del servidor */
        pthread_mutex_t mutex; /** Mutex de la cola */
        pthread_cond_t cond; /** Condición de variable de la cola */
};
