#include <stdio.h>
#include <string.h>
#include <iostream>
#include "VSocket.hpp"
#include "Socket.hpp"
#include "SSLSocket.hpp"
#include "Parser.hpp"

#include "Logger.hpp"

#define MAXBUF 1024

// Despues metemos esto en el main.
Logger log("./logs.txt");


void ClientRequestList(VSocket* client, const char* service) {
   char * os = (char *) "os.ecci.ucr.ac.cr";
   char * request = (char *)
      "GET /lego/index.php HTTP/1.1\r\n"
      "Host: os.ecci.ucr.ac.cr\r\n"
      "Connection: close\r\n"
      "\r\n";
   client->Connect(os, service);
   std::string mlog =  "Conexion establecida: "; 
   mlog += os;
   log.log(mlog.c_str());
   log.log(request);
   client->Write(request, strlen(request));
}

void ClientRequestFigure(VSocket* client, std::string figuraElegida, int parteElegida, const char* service) {
   char * os = (char *) "os.ecci.ucr.ac.cr";
   std::string path = "/lego/list.php?figure=" + figuraElegida + "&part=" + std::to_string(parteElegida);      
   std::string requestFigurePiezas = 
         "GET " + path + " HTTP/1.1\r\n"
         "Host: os.ecci.ucr.ac.cr\r\n"
         "Connection: close\r\n"
         "\r\n";
   std::cout << "[Servidor] Figura " << figuraElegida << std::endl;
   client->Connect(os, service);
   std::string mlog =  "Conexion establecida: "; 
   mlog += os;
   log.log(mlog.c_str());
   log.log(requestFigurePiezas );
   client->Write((char*)requestFigurePiezas.c_str(), requestFigurePiezas.length());

}

int main( int argc, char * argv[] ) {

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
   } else {
      client = new Socket('s');
   }
   std::vector<std::string> figuras;

   std::cout << "You can do:" << std::endl;
   std::string comandoLista = "GET/FiguresList";
   std::string comandoFigura = "GET/Figure";
   std::string comandoSalir = "Exit";

   std::cout << comandoLista << std::endl;
   std::cout << comandoFigura +"_Name/Half" << std::endl;
   std::cout << comandoSalir;

   std::string comando;
   while (running) {
      std::cout<< "\n[Cliente] Comando:";
      std::cin >> comando;
      log.log(comando.c_str(), Usuario);
      std::cout << std::endl;
      if (comando == comandoLista) {
         if (figuras.empty()) {
            html.clear();
            ClientRequestList(client, service);
            memset(a, 0, MAXBUF);
            while ((st = client->Read(a, MAXBUF-1)) > 0) {
               a[st] = 0;
               html += a;
            }
            
            parser.procesarFiguras(html);
            figuras = parser.getFiguras();

            

            log.logv(figuras, Server);

            std::cout << "[Servidor] Lista de figuras en el servidor:\n";

            for (const auto &p : figuras) {
               std::cout << p << std::endl;
            }
         } else {
            std::cout << "[Servidor] Lista de figuras en el servidor:\n";
            log.log("Lista de figuras en el servidor:", Server);
            for (const auto &p : figuras) {
               std::cout << p << std::endl;
            }
         }
      }  else if (comando.find("GET/Figure_") == 0) {
         if (figuras.empty()) {
            html.clear();
            ClientRequestList(client, service);
            memset(a, 0, MAXBUF);
            while ((st = client->Read(a, MAXBUF-1)) > 0) {
               a[st] = 0;
               html += a;
            }
            
            parser.procesarFiguras(html);
            figuras = parser.getFiguras();
            log.logv(figuras, Server);
         }
         std::string resto = comando.substr(11);
         size_t pos = resto.find("/");
         int parteElegida;
         std::string piezaElegida = resto.substr(0, pos);
         bool piezaValida = false;
         for (const auto &p: figuras) {
            if (p == piezaElegida) {
               piezaValida = true;
            }
         }
         if (!piezaValida) {
            std::cout << "Figura: " << piezaElegida << " no existe en el servidor" << std::endl;
            continue;
         }
         if (isdigit(resto[pos + 1])) {
            parteElegida  = std::stoi(resto.substr(pos + 1));
            if (parteElegida > 2) {
               std::cout << "Parte elegida incorrecta, ingrese (1|2)" << std::endl;
               log.log("Comando invalido", Cliente);
               continue;
            }
         } else {
            std::cout << "Parte elegida incorrecta, debe ser un valor numerico!" << std::endl;
            log.log("Comando invalido", Cliente);
            continue;
         }
         delete client;
         if (argc > 1) {
            client = new SSLSocket();
         } else {
            client = new Socket('s');
         }
         ClientRequestFigure(client, piezaElegida, parteElegida, service);
      
         std::string htmlPieza;
         while ((st = client->Read(a, MAXBUF-1)) > 0) {
            a[st] = 0;
            htmlPieza += a;
         }
         parser.procesarPiezas(htmlPieza);
         auto listaPiezas = parser.getPiezas();

         if (listaPiezas.empty()) {
            std::cout << "No se pudieron parsear las piezas." << std::endl;
         } else {
            for (const auto &p : listaPiezas) {
               std::cout << "Pieza: " << p.first << "  Cantidad: " << p.second << std::endl;
            }
         }
      } else if (comando == comandoSalir) {
         std::cout << "[Servidor] Cerrando comunicacion" << std::endl;
         log.log("Cerrando comunicacion", Server);
         break;
      } else {
         std::cout << "Comando Invalido!" << std::endl;
         log.log("Comando invalido", Cliente);
      }
   }
   delete client;
}