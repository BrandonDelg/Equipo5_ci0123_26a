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
#include "SSLSocket.hpp"
#include "Parser.hpp"
#include "Client.hpp"
#include "Logger.hpp"

#define MAXBUF 1024 /** Tamaño maximo del buffer */
/**
 * @brief Creación del cliente y interfaz para interactuar
 * 
 * @param argc Contador de argumentos
 * @param argv Argumentos, si es más de 1 es https de otra manera http
 *
 * @return Ejecución exitosa en 0
 */
int main( int argc, char * argv[] ) {
   Client cliente(1);
   Logger log("./logs.log");
   (void)argv;
   char a[MAXBUF];
   Parser parser;
   VSocket * client;
   std::string html;
   int st;
   bool running = true;
   const char* service = argc > 1 ? "https" : "http";
   if (argc > 1) {
      client = new SSLSocket();
      std::cout << "Usando https!" << std::endl;
   } else {
      client = new Socket('s');
      std::cout << "Usando http" << std::endl;

   }
   std::vector<std::string> figuras;

   std::string comandoRequestList = "[GET/REQUEST/FIGURESLIST]";
   std::string comandoResponseLista = "[RESPONSE/FIGURESLIST]";
   std::string comandoFigura = "GET/FIGURE";
   std::string comandoErrorServ = "[Servidor][ERROR]";
   std::string comandoSalirServ = "[Servidor][EXIT]";
   
   std::cout << "You can do:" << std::endl;
   std::cout << "1) Solicitar lista de Figuras" << std::endl;
   std::cout << "2) Solicitar Figura en especifico" << std::endl;
   std::cout << "3) Salir" << std::endl;

   std::string comando;
   std::string clientId = "[Cliente #" +  std::to_string(cliente.getId()) + "]";
   while (running) {
      std::cout<< "\n" << "Comando:";
      std::getline(std::cin, comando);
      log.log(comando.c_str(), Usuario);
      std::cout << std::endl;
      if (comando == "1") {
         if (figuras.empty()) {
            std::cout << clientId << comandoRequestList << std::endl;
            html.clear();
            cliente.ClientRequestList(client, service,log);
            memset(a, 0, MAXBUF);
            while ((st = client->Read(a, MAXBUF-1)) > 0) {
               a[st] = 0;
               html += a;
            }
            
            parser.procesarFiguras(html);
            figuras = parser.getFiguras();
            log.logv(figuras, Server);
            std::cout << "[Servidor]" << comandoResponseLista << " Lista de figuras en el servidor:\n";
            for (const auto &p : figuras) {
               std::cout << " - " << p << std::endl;
            }
         } else {
            std::cout << "[Servidor]" << comandoResponseLista  << "Lista de figuras en el servidor:\n";
            log.log("Lista de figuras en el servidor:", Server);
            for (const auto &p : figuras) {
               std::cout << " - " << p << std::endl;
            }
         }
      } else if (comando == "2") {

         std::cout << "Ingrese la figura y la mitad que desea solicitar\n";
         std::cout << "Figura:";

         std::string datos;
         std::getline(std::cin, datos);

         std::istringstream iss(datos);
         std::string piezaElegida;
         int parteElegida;

         if (!(iss >> piezaElegida >> parteElegida)) {
            std::cout << "Formato invalido. Use: figura numero (ej: lion 1)" << std::endl;
            continue;
         }

         std::cout << clientId << "[" << comandoFigura << "/"<< piezaElegida << "/" << parteElegida << "]\n";
         if (figuras.empty()) {

            html.clear();
            cliente.ClientRequestList(client, service,log);

            memset(a, 0, MAXBUF);
            while ((st = client->Read(a, MAXBUF-1)) > 0) {
               a[st] = 0;
               html += a;
            }

            parser.procesarFiguras(html);
            figuras = parser.getFiguras();
            log.logv(figuras, Server);
         }

         bool piezaValida = false;
         for (const auto &p : figuras) {
            if (p == piezaElegida) {
               piezaValida = true;
               break;
            }
         }

         if (!piezaValida) {
            std::cout << comandoErrorServ << " Figura: " << piezaElegida << " no existe en el servidor\n";
            continue;
         }

         if (parteElegida != 0 && parteElegida != 1 && parteElegida != 2) {
            std::cout << comandoErrorServ << " Parte elegida incorrecta, ingrese (0|1|2)\n";
            log.log("Comando invalido", Cliente);
            continue;
         }
         if (parteElegida == 0) {
            std::cout << "[Servidor][RESPONSE_FIGURE/Figure=" <<  piezaElegida << "/PART=" << parteElegida << "]" << std::endl;
            for (int i = 1; i <= 2; i++) {
               auto listaPiezas = cliente.pedirFigura(piezaElegida, i, argc, service, parser,log);
               int totalPiezas = 0;
               for (const auto &p : listaPiezas) {
                  std::cout << "Pieza: " << p.first << "  Cantidad: " << p.second << std::endl;
                  totalPiezas += p.second;
               }
               std::cout << "Cantidad de piezas totales de la mitad " << parteElegida << " : " << totalPiezas << std::endl;

            }
            continue;
         }
         auto listaPiezas = cliente.pedirFigura(piezaElegida, parteElegida, argc, service, parser,log);
         if (listaPiezas.empty()) {
            std::cout << "[ERROR] No se pudieron parsear las piezas.\n";
         } else {
            std::cout << "[Servidor][RESPONSE_FIGURE/Figure=" <<  piezaElegida << "/PART=" << parteElegida << "]" << std::endl;
            int totalPiezas = 0;
            for (const auto &p : listaPiezas) {
               std::cout << "Pieza: " << p.first << "  Cantidad: " << p.second << std::endl;
               totalPiezas += p.second;
            }
            std::cout << "Cantidad de piezas totales de la mitad " << parteElegida << " : " << totalPiezas << std::endl;
         }
      } else if (comando == "3") {
         std::cout << comandoSalirServ << " Cerrando comunicacion" << std::endl;
         log.log("Cerrando comunicacion", Server);
         break;
      } else {
         std::cout << "[ERROR] Comando Invalido!" << std::endl;
         log.log("Comando invalido", Cliente);
      }
   }
   delete client;
}
