#ifndef CLIENTE_HPP
#define CLIENT_HPP
#include <vector>
#include <string>
#include "Parser.hpp"
#include "Logger.hpp"
#include "VSocket.hpp"
#include "SSLSocket.hpp"
#include "Socket.hpp"
#define MAXBUF 1024

class Client {
    public: 
        Client(int id);
        ~Client();
        void ClientRequestFigure(VSocket* client, std::string figuraElegida, int parteElegida, const char* service, Logger& log);
        void ClientRequestList(VSocket* client, const char* service, Logger& log);
        std::vector<std::pair<std::string, int>> pedirFigura(std::string figura, 
        int parte, int argc, const char* service,Parser& parser, Logger& log);
        int getId();

    private:
        int id;
};
#endif //CLIENT_HPP