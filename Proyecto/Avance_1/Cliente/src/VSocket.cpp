#include <sys/socket.h>
#include <arpa/inet.h>		// ntohs, htons
#include <stdexcept>            // runtime_error
#include <cstring>		// memset
#include <netdb.h>		// getaddrinfo, freeaddrinfo
#include <unistd.h>		// close

#include "VSocket.hpp"
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
  *  Class creator (constructor)
  *     use Unix socket system call
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool ipv6: if we need a IPv6 socket
  *
 **/
void VSocket::Init( char t, bool IPv6 ){

   this->IPv6 = IPv6;
   int domain = this->IPv6 ? AF_INET6 : AF_INET;
   int type;

   if (t == 's' || t == ' ') {
      type = SOCK_STREAM;
   } else if (t == 'd') {
      type = SOCK_DGRAM;
   } else {
      throw std::invalid_argument("Invalid socket type");
   }

   sockId = socket(domain, type, 0);
   if ( -1 == sockId ) {
      throw std::runtime_error( "VSocket::Init, (reason)" );
   }
   
}


/**
  * Class destructor
  *
 **/
VSocket::~VSocket() {

   this->Close();

}


/**
  * Close method
  *    use Unix close system call (once opened a socket is managed like a file in Unix)
  *
 **/
void VSocket::Close(){
   if (sockId >= 0) {
      close(sockId);
      sockId = -1;
   } else {
      throw std::runtime_error( "VSocket::Close()" );

   }
}


/**
  * TryToConnect method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dot notation, example "10.84.166.62"
  * @param      int port: process address, example 80
  *
 **/
int VSocket::TryToConnect( const char * hostip, int port ) {

   int st;
   if (!IPv6) {
      struct sockaddr_in  host4;
      memset( (char *) &host4, 0, sizeof( host4 ) );
      host4.sin_family = AF_INET;
      st = inet_pton( AF_INET, hostip, &host4.sin_addr );
      if (st <= 0 ) {
         throw( std::runtime_error( "VSocket::TryToConnect, inet_pton" ));
      }
      host4.sin_port = htons( port );
      st = connect( sockId, (sockaddr *) &host4, sizeof( host4 ) );
      if ( -1 == st ) {
         throw( std::runtime_error( "VSocket::TryToConnect, connect" ));
      }
   } else {
      struct sockaddr_in6  host6;
      memset( (char *) &host6, 0, sizeof( host6 ) );
      host6.sin6_family = AF_INET6;
      st = inet_pton( AF_INET6, hostip, &host6.sin6_addr );
      if (st <= 0 ) {
         throw( std::runtime_error( "VSocket::TryToConnect, inet_pton" ));
      }
      host6.sin6_port = htons( port );
      st = connect( sockId, (sockaddr *) &host6, sizeof( host6 ) );
      if ( -1 == st ) {
         throw( std::runtime_error( "VSocket::TryToConnect, connect" ));
      }
   }
   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::TryToConnect" );
   }

   return st;

}


/**
  * TryToConnect method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dns notation, example "os.ecci.ucr.ac.cr"
  * @param      char * service: process address, example "http"
  *
 **/
int VSocket::TryToConnect(const char *host, const char *service) {

   int st;
   struct addrinfo hints, *result, *rp;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;

   st = getaddrinfo(host, service, &hints, &result);
   if (st != 0) {
      throw std::runtime_error("VSocket::TryToConnect getaddrinfo");
   }

   for (rp = result; rp != NULL; rp = rp->ai_next) {
      st = connect(sockId, rp->ai_addr, rp->ai_addrlen);
      if (st == 0) {
         break;
      }
   }

   freeaddrinfo(result);

   if (rp == NULL) {
      throw std::runtime_error("VSocket::TryToConnect connect");
   }

   return st;
}


/**
  * Bind method
  *    use "bind" Unix system call (man 3 bind) (server mode)
  *
  * @param      int port: bind a unamed socket to a port defined in sockaddr structure
  *
  *  Links the calling process to a service at port
  *
 **/
int VSocket::Bind( int port ) {
   struct sockaddr_in my_addr;

   memset(&my_addr, 0, sizeof(my_addr));
   my_addr.sin_family = AF_INET;
   my_addr.sin_port = htons(port);
   my_addr.sin_addr.s_addr = INADDR_ANY;
   int st = bind(this->sockId, (struct sockaddr*)&my_addr, sizeof(my_addr));
   if ( st == -1) {
      throw std::runtime_error("ERROR bind() ipv4");   
   }
   return st;
}


/**
  *  sendTo method
  *
  *  @param	const void * buffer: data to send
  *  @param	size_t size data size to send
  *  @param	void * addr address to send data
  *
  *  Send data to another network point (addr) without connection (Datagram)
  *
 **/
size_t VSocket::sendTo( const void * buffer, size_t size, void * addr) {
   int st = sendto(this->sockId, buffer, size, 0,(struct sockaddr*) addr,sizeof(struct sockaddr_in));   
   if (st == -1) {
      throw std::runtime_error("ERROR sendTo()");   
   }
   return st;
}


/**
  *  recvFrom method
  *
  *  @param	const void * buffer: data to send
  *  @param	size_t size data size to send
  *  @param	void * addr address to receive from data
  *
  *  @return	size_t bytes received
  *
  *  Receive data from another network point (addr) without connection (Datagram)
  *
 **/
size_t VSocket::recvFrom( void * buffer, size_t size, void * addr ) {
   socklen_t addrlen = sizeof(struct sockaddr_in);
   int st = recvfrom(this->sockId, buffer, size, 0, (struct sockaddr*) addr, &addrlen);
   if (st == -1) {
      throw std::runtime_error("ERROR recvFrom()");   
   }
   return st;

}

