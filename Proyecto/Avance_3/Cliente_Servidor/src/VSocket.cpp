
/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  ****** VSocket base class implementation
  *
  * (Fedora version)
  *
 **/

#include <sys/socket.h>
#include <arpa/inet.h>		// ntohs, htons
#include <stdexcept>            // runtime_error
#include <cstring>		// memset
#include <netdb.h>		// getaddrinfo, freeaddrinfo
#include <unistd.h>		// close
/*
#include <cstddef>
#include <cstdio>

//#include <sys/types.h>
*/
#include "VSocket.hpp"


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
void VSocket::init( char t, bool IPv6 ){
   int st = -1;
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
   st = this->idSocket = socket(domain, type, 0);
    if ( -1 == st ) {
      throw std::runtime_error( "VSocket::BuildSocket, (reason)" );
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
  if (this->idSocket >= 0) {
      close(this->idSocket);
      this->idSocket = -1;
   }
}


/**
  * EstablishConnection method
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
      st = connect(idSocket, (sockaddr *) &host4, sizeof( host4 ) );
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
      st = connect(this->idSocket, (sockaddr *) &host6, sizeof( host6 ) );
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
  * EstablishConnection method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dns notation, example "os.ecci.ucr.ac.cr"
  * @param      char * service: process address, example "http"
  *
 **/
int VSocket::TryToConnect( const char *host, const char *service ) {
   int st;
   struct addrinfo hints, *result, *rp;
   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = 0;
   hints.ai_protocol = 0;          /* Any protocol */

   st = getaddrinfo( host, service, &hints, &result );

   for ( rp = result; rp; rp = rp->ai_next ) {
   st = connect(this->idSocket, rp->ai_addr, rp->ai_addrlen );
   if ( 0 == st )
      break;
   }

   freeaddrinfo( result );
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
int VSocket::Bind(int port) {
   int st;
   if (!this->IPv6) {
      struct sockaddr_in my_addr;

      memset(&my_addr, 0, sizeof(my_addr));
      my_addr.sin_family = AF_INET;
      my_addr.sin_port = htons(port);
      my_addr.sin_addr.s_addr = INADDR_ANY;

      st = bind(this->idSocket, (struct sockaddr*)&my_addr, sizeof(my_addr));

      if (st == -1) {
         throw std::runtime_error("ERROR bind() ipv4");
      }

   } else {
      struct sockaddr_in6 my_addr6;

      memset(&my_addr6, 0, sizeof(my_addr6));
      my_addr6.sin6_family = AF_INET6;
      my_addr6.sin6_port = htons(port);
      my_addr6.sin6_addr = in6addr_any;

      st = bind(this->idSocket, (struct sockaddr*)&my_addr6, sizeof(my_addr6));

      if (st == -1) {
         throw std::runtime_error("ERROR bind() ipv6");
      }
   }

   return st;
}



/**
  * MarkPassive method
  *    use "listen" Unix system call (man listen) (server mode)
  *
  * @param      int backlog: defines the maximum length to which the queue of pending connections for this socket may grow
  *
  *  Establish socket queue length
  *
 **/
int VSocket::MarkPassive( int backlog ) {
   int st = listen(this->idSocket, backlog);
   if (st < 0) {
      throw std::runtime_error( "VSocket::MarkPassive" );
   }
   return st;
}


/**
  * WaitForConnection method
  *    use "accept" Unix system call (man 3 accept) (server mode)
  *
  *
  *  Waits for a peer connections, return a sockfd of the connecting peer
  *
 **/
int VSocket::WaitForConnection( void ) {
   struct sockaddr_storage peer_addr;
   socklen_t addrlen = sizeof(peer_addr);
   int st = accept(this->idSocket, (struct sockaddr*)&peer_addr, &addrlen);
   if (st < 0) {
      throw std::runtime_error( "VSocket::WaitForConnection" );
   }

   return st;

}


/**
  * Shutdown method
  *    use "shutdown" Unix system call (man 3 shutdown) (server mode)
  *
  *
  *  cause all or part of a full-duplex connection on the socket associated with the file descriptor socket to be shut down
  *
 **/
int VSocket::Shutdown( int mode ) {
   int st = shutdown(this->idSocket, mode);
   if (st < 0) {
      throw std::runtime_error( "VSocket::Shutdown" );
   }
   return st;
}


// UDP methods 2025

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
   int st = sendto(this->idSocket, buffer, size, 0,(struct sockaddr*) addr,sizeof(struct sockaddr_in));   
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
   int st = recvfrom(this->idSocket, buffer, size, 0, (struct sockaddr*) addr, &addrlen);
   if (st == -1) {
      throw std::runtime_error("ERROR recvFrom()");   
   }
   return st;

}

