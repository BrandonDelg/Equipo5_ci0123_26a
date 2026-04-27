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
#include "VSocket.hpp"
#include "Socket.hpp"
#include "Client.hpp"
#include "Logger.hpp"
#include "FileSystem.hpp"

#define MAXBUF 1024 /** Tamaño maximo del buffer */

int main(int argc, char* argv[]) {
   FileSystem fs("filesystem.dat");
   fs.crearFigura("Perro",
      "1\nlego 1x2:4\nlego 2x2:2\nlego 3x2:4\nlego 3x3:2\nlego 3x2:4\nlego 3x3:2\n"
      "2\nlego 3x2:4\nlego 3x3:2\n");

   fs.crearFigura("Gato",
      "1\nlego 5x2:4\n"
      "2\nlego 6x2:4\n");

   fs.crearFigura("Ballena",
      "1\n"
      "lego 3x2:4\n"
      "lego 2x2:2\n"
      "lego 1x5:3\n"
      "2\n"
      "lego 1x2:4\n"
      "lego 4x2:2\n"
      "lego 3x4:3\n"
   );

   fs.crearFigura("Oveja",
      "1\n"
      "lego 2x2:2\n"
      "lego 1x5:3\n"
      "lego 1x2:4\n"
      "2\n"
      "lego 1x2:4\n"
      "lego 4x2:2\n"
      "lego 3x4:3\n"
   );

   fs.crearFigura("Carro",
      "1\n"
      "Medium Stone Gray Bar 1:2\n"
      "Black Bracket 1x1 with 1x1 Plate Down:2\n"
      "Medium Stone Gray Bracket 1x2 with 12 Up:1\n"
      "Dark Red Brick 1x2 with Bottom Tube:1\n"
      "Red Brick 1x2 with Grille:2\n"
      "Transparent Brick 1x2 without Bottom Tube:2\n"
      "Red Brick 1x4:2\n"
      "Transparent Diamond:2\n"
      "Black Mudguard Plate 2x4 with Arches with Hole:2\n"
      "Red Panel 1x2x1 with Rounded Corners:1\n"
      "Pearl Gold Plate 1x1 Round:3\n"
      "Medium Stone Gray Plate 1x1 with Clip (Thick Ring):2\n"
      "Pearl Gold Plate 1x1 with Horizontal Clip (Thick Open O Clip):1\n"
      "Red Plate 1x2:1\n"
      "Red Plate 1x2 with 1 Stud (with Groove and Bottom Stud Holder):1\n"
      "Medium Stone Gray Plate 1x2 with Horizontal Clips (Open O Clips):2\n"
      "2\n"
      "Medium Stone Gray Plate 1x6:2\n"
      "Red Plate 1x8:2\n"
      "Black Plate 2x2 Corner:2\n"
      "Medium Stone Gray Plate 2x2 with Two Wheel Holder Pins:2\n"
      "Black Plate 2x4:1\n"
      "Black Slope 1x2 Curved:2\n"
      "Black Slope 1x3 Curved:2\n"
      "Reddish Brown Suitcase:1\n"
      "Transparent Red Tile 1x1 Round:2\n"
      "Pearl Gold Tile 1x1 Round with Crown Coin:1\n"
      "Medium Stone Gray Tile 1x2 (with Bottom Groove):1\n"
      "Black Tile 2x2 with Studs on Edge:4\n"
      "Dark Red Tile 2x4:1\n"
      "Red Tile 2x4:1\n"
      "Black Tire 14x6:4\n"
      "Flat Silver Wheel Rim 11x6 with Hub Cap:4\n"
   );

   if (argc < 3) {
      std::cerr << "Uso: " << argv[0] << " <host> <ipv6 0|1>\n";
      return 1;
   }

   const char* host = argv[1];
   bool ipv6 = std::stoi(argv[2]);
   const char* service = "8080";

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
               std::cout << "- " << linea << std::endl;
            }
         }
      } else if (comando == "2") {
         std::string figura;
         std::cout << "Nombre figura: ";
         std::getline(std::cin, figura);
         std::cout << "Parte (1,2): ";
         std::string parte;
         std::getline(std::cin, parte);
         int p = std::stoi(parte);
         // if (p > 2 || p <= 1) {
         //    std::cout << "parte incorrecta, ingrese nuevamente" << std::endl;
         //    continue;
         // } else {
            VSocket* client = new Socket('s', ipv6);
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
               std::cout << "Piezas:\n";
               std::cout << response << std::endl;
            }
         //}

      } else if (comando == "3") {
         VSocket* client = new Socket('s', ipv6);
         std::cout << "Saliendo...\n";
         cliente.CloseConnection(client, service, host);
         running = false;
         delete client;
      } else {
         std::cout << "Comando invalido\n";
      }
   }

   return 0;
}