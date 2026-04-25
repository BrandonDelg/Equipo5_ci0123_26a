/**
 * @file Client.hpp
 * @brief Definición de la clase Client
 */
#ifndef CLIENTE_HPP
#define CLIENT_HPP
#include <vector>
#include <string>
#include "Logger.hpp"
#include "VSocket.hpp"
#include "Socket.hpp"
#define MAXBUF 1024

/**
 * @brief Clase cliente
 *
 * Representa el cliente que realiza solicitudes al servidor de piezas
 */
class Client {
    public: 
        /**
         * @brief Constructor del Client
         *
         * @param Id del cliente
         */
        Client(int id);

        /**
         * @brief Destructor del Client
         */
        ~Client();

        /**
         * @brief Solicita una figura
         *
         * @param client El cliente que solicita
         * @param figuraElegida Figura a solicitar
         * @param parteElegida Parte de la figura a solicitar
         * @param service Servicio https o http
         * @param log Referencia al Logger
         */
        void ClientRequestFigure(VSocket* client, std::string figuraElegida, int parteElegida, const char* service, Logger& log, const char* host);

        /**
         * @brief Solicita la lista de figuras
         *
         * @param client El cliente que solicita
         * @param service Servicio https o http
         * @param log Referencia al Logger
         */
        void ClientRequestList(VSocket* client, const char* service, Logger& log, const char* host);

        /**
         * @brief Devuelve la id del cliente
         */
        int getId();
        void CloseConnection(VSocket* client, const char* service, const char* host);


    private:
        int id; /** Id del cliente **/
};
#endif //CLIENT_HPP
