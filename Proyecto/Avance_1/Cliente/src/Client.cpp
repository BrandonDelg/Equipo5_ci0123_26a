#include <stdio.h>
#include <string.h>
#include <iostream>
#include "VSocket.hpp"
#include "Socket.hpp"
#include "SSLSocket.hpp"
#include "Parser.hpp"

#define MAXBUF 1024

int main( int argc, char * argv[] ) {
   (void)argv;
   Parser parser;
   VSocket * client;
   int st;
   std::string html;
   char a[MAXBUF];
   char * os = (char *) "os.ecci.ucr.ac.cr";
   char * request = (char *)
   "GET /lego/index.php HTTP/1.1\r\n"
   "Host: os.ecci.ucr.ac.cr\r\n"
   "Connection: close\r\n"
   "\r\n";

   if (argc > 1) {
      client = new SSLSocket();
      std::cout << "Cliente ssl creado" << std::endl;
   } else {
      client = new Socket('s');
      std::cout << "Cliente creado" << std::endl;
   }

   memset(a, 0, MAXBUF);

   client->Connect(os, (argc > 1 ? "https" : "http"));
   client->Write(request, strlen(request));

   while ((st = client->Read(a, MAXBUF-1)) > 0) {
      a[st] = 0;
      html += a;
   }

   parser.procesarFiguras(html);
   std::vector<std::string> figuras = parser.getFiguras();

   if (!figuras.empty()) {
      std::string piezaElegida = figuras[1];
      int parteElegida = 1;

      std::string path = "/lego/list.php?figure=" + piezaElegida + "&part=" + std::to_string(parteElegida);      
      std::string requestFigurePiezas = 
         "GET " + path + " HTTP/1.1\r\n"
         "Host: os.ecci.ucr.ac.cr\r\n"
         "Connection: close\r\n"
         "\r\n";

      std::cout << "Figura elegida " << piezaElegida << std::endl;

      delete client;
      if (argc > 1) {
         client = new SSLSocket(); 
      }else {
         client = new Socket('s');
      }
      client->Connect(os, (argc > 1 ? "https" : "http"));
      client->Write((char*)requestFigurePiezas.c_str(), requestFigurePiezas.length());

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
            std::cout << "Cantidad: " << p.second << "  Pieza: " << p.first << std::endl;
         }
      }
   }
   delete client;
}