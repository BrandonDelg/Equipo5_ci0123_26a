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
#include <mutex>


#include <sys/socket.h>
#include <arpa/inet.h>		// ntohs, htons
#include <stdexcept>            // runtime_error
#include <cstring>		// memset
#include <netdb.h>		// getaddrinfo, freeaddrinfo
#include <unistd.h>	

#include "Socket.hpp"

std::mutex fsMutex;
#define PORT 1236
#define BUFSIZE 512

#define DISCOVERY_PORT_INTERMEDIARIO 9094
#define DISCOVERY_PORT_SERVIDOR 9095

#define BROADCAST_ISLA "172.16.123.95"

//#define BROADCAST_ISLA "192.168.0.255"

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
   while (request.find("\r\n\r\n") == std::string::npos) {
      st = client->Read(buffer, BUFSIZE - 1);
      if (st <= 0) break;

      buffer[st] = 0;
      request += buffer;
   }

   if (request.find("GET /shutdown") != std::string::npos) {
      std::cout << "[SERVIDOR] Shutdown recibido\n";

      response =
         "HTTP/1.1 200 OK\r\n"
         "Access-Control-Allow-Origin: *\r\n"
         "Content-Type: text/plain\r\n"
         "\r\n"
         "OK";

      client->Write(response.c_str(), response.length());
      client->Close();
      delete client;

      exit(0);

   } else if (request.find("GET /figuras") != std::string::npos) {
      std::vector<std::string> figs;

      {
         std::lock_guard<std::mutex> lock(fsMutex);
         figs = fs.getFiguras();
      }

      response =
         "HTTP/1.1 200 OK\r\n"
         "Access-Control-Allow-Origin: *\r\n"
         "Content-Type: text/plain\r\n"
         "\r\n";

      for (auto& f : figs) {
         response += f + "\n";
      }

   } else if (request.find("GET /figura/") != std::string::npos) {
      size_t start = request.find("/figura/") + 8;
      size_t end = request.find(" ", start);

      if (end == std::string::npos) {
         response =
            "HTTP/1.1 400 Bad Request\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n";

      } else {
         std::string ruta = request.substr(start, end - start);
         size_t slash = ruta.find("/");

         if (slash == std::string::npos) {
            response =
               "HTTP/1.1 400 Bad Request\r\n"
               "Access-Control-Allow-Origin: *\r\n"
               "Content-Type: text/plain\r\n"
               "\r\n";

         } else {
            std::string nombre = ruta.substr(0, slash);
            int parte = std::stoi(ruta.substr(slash + 1));

            std::lock_guard<std::mutex> lock(fsMutex);
            std::string piezas = fs.getPiezas(nombre, parte);

            if (piezas.empty()) {
               response =
                  "HTTP/1.1 404 Not Found\r\n"
                  "Access-Control-Allow-Origin: *\r\n"
                  "Content-Type: text/plain\r\n"
                  "\r\n";

            } else {
               fs.getPiezas(nombre, parte);
               //fs.consumirPiezas(nombre, parte);

               response =
                  "HTTP/1.1 200 OK\r\n"
                  "Access-Control-Allow-Origin: *\r\n"
                  "Content-Type: text/plain\r\n"
                  "\r\n" + piezas;
            }
         }
      }

   } else {
      response =
         "HTTP/1.1 404 Not Found\r\n"
         "Access-Control-Allow-Origin: *\r\n"
         "Content-Type: text/plain\r\n"
         "\r\n";
   }

   client->Write(response.c_str(), response.length());
   client->Close();
   delete client;
}

std::string obtenerFigurasParaAnuncio() {
   std::vector<std::string> figs;
   {
      std::lock_guard<std::mutex> lock(fsMutex);
      figs = fs.getFiguras();
   }
   std::string resultado;
   for (size_t i = 0; i < figs.size(); ++i) {
      resultado += figs[i];
      if (i + 1 < figs.size()) {
         resultado += ",";
      }
   }
   return resultado;
}
void escucharDescubrimientoIntermediario() {
   VSocket* udp = new Socket('d', false);

   udp->Bind(DISCOVERY_PORT_SERVIDOR);

   std::cout << "[UDP SERVIDOR] Escuchando descubrimiento en puerto "
             << DISCOVERY_PORT_SERVIDOR << std::endl;

   while (true) {
      char buffer[BUFSIZE] = {0};

      sockaddr_in sender;
      memset(&sender, 0, sizeof(sender));

      int st = udp->recvFrom(buffer, BUFSIZE - 1, &sender);

      if (st > 0) {
         buffer[st] = '\0';

         std::string mensaje = buffer;

         std::cout << "[UDP SERVIDOR] Recibido: "
                   << mensaje << std::endl;

         if (mensaje.find("INTERMEDIARY_JOIN") == 0) {
            size_t posUdp = mensaje.find("udp=");
            size_t posPort = mensaje.find(";port=");

            if (posUdp != std::string::npos &&
                posPort != std::string::npos) {
               std::string udpStr = mensaje.substr(posUdp + 4,
                                                    posPort - (posUdp + 4));

               int puertoUdpIntermediario = std::stoi(udpStr);
               sender.sin_port = htons(puertoUdpIntermediario);
            }

            std::string figuras = obtenerFigurasParaAnuncio();

            std::string respuesta =
               "FIGURAS_JOIN|port=" + std::to_string(PORT) +
               ";figuras=" + figuras;

            udp->sendTo(respuesta.c_str(),
                        respuesta.length(),
                        &sender);

            std::cout << "[UDP SERVIDOR] Respuesta enviada: "
                      << respuesta << std::endl;
         }
      }
   }
}
void anunciarServidor() {
    VSocket* udp = new Socket('d', false);
    udp->EnableBroadcast();

    while (true) {
        std::string mensaje =
            "SERVER_JOIN|port=" + std::to_string(PORT);

        udp->Broadcast(
            mensaje.c_str(),
            mensaje.length(),
            BROADCAST_ISLA,
            DISCOVERY_PORT_INTERMEDIARIO
        );

        sleep(5);
    }
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

   std::thread discoveryThread(escucharDescubrimientoIntermediario);
   discoveryThread.detach();

   std::thread announceThread(anunciarServidor);
   announceThread.detach();
   for( ; ; ) {
      client = s1->AcceptConnection();	 	// Wait for a client connection
      worker = new std::thread( task, client );
   }

}