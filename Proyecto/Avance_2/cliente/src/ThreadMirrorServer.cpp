/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  *   Socket client/server example with threads
  *
  * (Fedora version)
  *
 **/
 
#include <iostream>
#include <thread>
#include "FileSystem.hpp"

#include "Socket.hpp"

#define PORT 1234
#define BUFSIZE 512


/**
 *   Task each new thread will run
 *      Read string from socket
 *      Write it back to client
 *
 **/

FileSystem fs("filesystem.dat");

void task(VSocket* client) {
   char buffer[BUFSIZE] = {0};
   int st;

   std::string request;

   st = client->Read(buffer, BUFSIZE - 1);
   if (st <= 0) {
      client->Close();
      delete client;
      return;
   }

   buffer[st] = 0;
   request = buffer;

   std::string response;

   std::cout << "Request:\n" << request << std::endl;

   if (request.find("|") != std::string::npos) {
      if (request.find("14|") == 0) {
         size_t posFig = request.find("figura=");
         size_t posMid = request.find(";mitad=");
         if (posFig == std::string::npos || posMid == std::string::npos) {
               response = "108|";
         } else {
            std::string figura = request.substr(posFig + 7, posMid - (posFig + 7));
            int parte = 0;
            try {
               parte = std::stoi(request.substr(posMid + 7));
            } catch (...) {
               response = "108|";
            }

            if (parte != 1 && parte != 2) {
               response = "105|";
            } else {
               std::string piezas = fs.getPiezas(figura, parte);

               if (piezas.empty()) {
                  response = "16|";
               } else {
                  response = "15|figura=" + figura +
                              ";mitad=" + std::to_string(parte) +
                              ";piezas=" + piezas;
               }
            }
         }
      } else if (request.find("10|") == 0) {
         auto figs = fs.getFiguras();
         response = "11|figuras=";
         for (auto& f : figs) {
               response += f + "\n";
         }
      }else if (request.find("99|") == 0) {
         std::cout << "[SERVIDOR] Shutdown recibido\n";

         std::string response = "OK";
         client->Write(response.c_str(), response.length());

         client->Close();
         delete client;

         exit(0);
      } else {
         response = "107|";
      }
      client->Write(response.c_str(), response.length());
      client->Close();
      delete client;
      return;
    }

   while (request.find("\r\n\r\n") == std::string::npos) {
      st = client->Read(buffer, BUFSIZE - 1);
      if (st <= 0) break;
      buffer[st] = 0;
      request += buffer;
   }
   if (request.find("GET /figuras") != std::string::npos) {
      auto figs = fs.getFiguras();
      response = "HTTP/1.1 200 OK\r\n\r\n";
      for (auto& f : figs) {
         response += f + "\n";
      }
   } else if (request.find("GET /figura/") != std::string::npos) {
      size_t start = request.find("/figura/") + 8;
      size_t end = request.find(" ", start);

      if (end == std::string::npos) {
         response = "HTTP/1.1 400 Bad Request\r\n\r\n";
      } else {
         std::string ruta = request.substr(start, end - start);
         size_t slash = ruta.find("/");
         if (slash == std::string::npos) {
            response = "HTTP/1.1 400 Bad Request\r\n\r\n";
         } else {
            std::string nombre = ruta.substr(0, slash);
            int parte = std::stoi(ruta.substr(slash + 1));
            std::string piezas = fs.getPiezas(nombre, parte);
            if (piezas.empty()) {
               response = "HTTP/1.1 404 Not Found\r\n\r\n";
            } else {
               response = "HTTP/1.1 200 OK\r\n\r\n" + piezas;
            }
         }
      }
   } else {
      response = "HTTP/1.1 404 Not Found\r\n\r\n";
   }

   client->Write(response.c_str(), response.length());
   client->Close();
   delete client;
}
/**
 *   Create server code
 *      Infinite for
 *         Wait for client conection
 *         Spawn a new thread to handle client request
 *
 **/
int main(int argc, char* argv[]) {
   std::thread * worker;
   VSocket * s1, * client;
   if (argc < 2) {
      std::cerr << "Uso: " << argv[0] << "<ipv6 0|1>\n";
      return 1;
   }
   bool ipv6 = std::stoi(argv[1]);

   s1 = new Socket( 's', ipv6);
   std::cout << "Servidor en " << (ipv6 ? " (IPv6)" : " (IPv4)") << std::endl;
   s1->Bind( PORT );		// Port to access this mirror server
   s1->MarkPassive( 5 );	// Set socket passive and backlog queue to 5 connections

   for( ; ; ) {
      client = s1->AcceptConnection();	 	// Wait for a client connection
      worker = new std::thread( task, client );
   }

}


