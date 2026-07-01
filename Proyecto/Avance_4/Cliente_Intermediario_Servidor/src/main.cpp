/**
 * @file main.cpp
 * @brief Creación del cliente y interfaz para interactuar
 */
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <exception>
#include "VSocket.hpp"
#include "Socket.hpp"
#include "Client.hpp"
#include "Logger.hpp"
#include "FileSystem.hpp"

#define MAXBUF 1024 /** Tamaño maximo del buffer */

int main(int argc, char* argv[]) {
   if (argc < 4) {
   std::cerr << "Uso: " << argv[0]
             << " <host> <ipv6 0|1> <puerto>\n";
   return 1;
   }

   const char* host = argv[1];
   bool ipv6 = std::stoi(argv[2]);
   const char* service = argv[3];

   Client cliente(1);
   Logger log("./logs.log");
   //const char* service = "8080";

   char buffer[MAXBUF];
   int st;

   bool running = true;

   std::vector<std::string> figuras;

   while (running) {
      std::cout << "1) Listar figuras\n";
      std::cout << "2) Pedir figura\n";
      std::cout << "3) Salir\n";
      std::string comando;
      std::cout << "\nComando: ";
      std::getline(std::cin, comando);

      if (comando == "1") {
         VSocket* client = new Socket('s', ipv6);
         try {
            client->SetReceiveTimeout(5);
            cliente.ClientRequestList(client, service, log, host);
            std::string response;
            while ((st = client->Read(buffer, MAXBUF-1)) > 0) {
               buffer[st] = 0;
               response += buffer;
            }
            delete client;
            size_t pos = response.find("\r\n\r\n");
            if (pos != std::string::npos) {
               response = response.substr(pos + 4);
            }

            std::cout << "Figuras disponibles:\n";

            std::stringstream ss(response);
            std::string linea;

            figuras.clear();

            while (std::getline(ss, linea)) {
               if (!linea.empty()) {
                  figuras.push_back(linea);
                  std::cout << linea << std::endl;
               }
            }
         } catch (const std::exception&) {
            delete client;
            std::cerr << "\nError: no se pudo conectar al servidor\n\n";
         }
      } else if (comando == "2") {
         std::string figura;
         std::cout << "Nombre figura: ";
         std::getline(std::cin, figura);
         std::cout << "Parte (1,2): ";
         std::string parte;
         std::getline(std::cin, parte);
         int p = std::stoi(parte);
         VSocket* client = new Socket('s', ipv6);
         try {
            client->SetReceiveTimeout(5);
            cliente.ClientRequestFigure(client, figura, p, service, log, host);
            std::string response;
            while ((st = client->Read(buffer, MAXBUF-1)) > 0) {
               buffer[st] = 0;
               response += buffer;
            }
            delete client;
            size_t pos = response.find("\r\n\r\n");
            if (pos != std::string::npos) {
               response = response.substr(pos + 4);
            }
            if (response != "") {
               if (response.find("Error") == 0) {
                  std::cout << response << std::endl;
               } else {
                  std::cout << "Piezas:\n";
                  std::cout << response << std::endl;
               }
            }
         } catch (const std::exception&) {
            delete client;
            std::cerr << "\nError: no se pudo conectar al servidor\n\n";
         }
      } else if (comando == "3") {
         VSocket* client = new Socket('s', ipv6);
         std::cout << "Saliendo...\n";
         try {
            client->SetReceiveTimeout(5);
            cliente.CloseConnection(client, service, host);
         } catch (const std::exception&) {}
         running = false;
         delete client;
      } else {
         std::cout << "Comando invalido\n";
      }
   }

   return 0;
}