/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  ****** VSocket base class interface
  *
  * (Fedora version)
  *
 **/

#ifndef VSocket_hpp
#define VSocket_hpp

#include <cstddef>
 
class VSocket {
   public:
      void init( char, bool = false );
      virtual ~VSocket();

      void Close();
      int TryToConnect( const char *, int );
      int TryToConnect( const char *, const char * );
      virtual int Connect( const char *, int ) = 0;
      virtual int Connect( const char *, const char * ) = 0;
      virtual std::size_t Read( void *, std::size_t ) = 0;
      virtual std::size_t Write( const void *, std::size_t ) = 0;
      virtual std::size_t Write( const char * ) = 0;

      int Bind( int );			// Assign a socket address to a socket descriptor
      int MarkPassive( int );		// Mark a socket passive: will be used to accept connections
      int WaitForConnection( void );	// Wait for a peer connection
      virtual VSocket * AcceptConnection() = 0;
      int Shutdown( int );		// cause all or part of a full-duplex connection on the socket
                                        // associated with the file descriptor socket to be shut down

// UDP methods
      // std::size_t sendTo( const void *, std::size_t, void * );
      // std::size_t recvFrom( void *, std::size_t, void * );
      size_t sendTo( const void *, size_t, void * );
      size_t recvFrom( void *, size_t, void * );

   protected:
      int idSocket;   // Socket identifier
      bool IPv6;      // Is IPv6 socket?
      int port;       // Socket associated port
      char type;      // Socket type (datagram, stream, etc.)
        
};

#endif // VSocket_h


