/**
 * @file SSLSocket.hpp
 * @brief Definición de la clase SSLSocket
 */
#ifndef SSLSocket_hpp
#define SSLSocket_hpp

#include "VSocket.hpp"
#include <cstddef>

/**
 * @brief Clase SSLSocket
 *
 * Construye un socket usando certificados ssl para comunicación 
 * segura entre dispositivos
 */
class SSLSocket : public VSocket {

   public:
      SSLSocket( bool IPv6 = false );				// Not possible to create with UDP, client constructor
      SSLSocket( char *, char *, bool = false );		// For server connections
      SSLSocket( int );
      ~SSLSocket();
      int Connect( const char *, int );
      int Connect( const char *, const char * );
      size_t Write( const char * );
      size_t Write( const void *, size_t );
      size_t Read( void *, size_t );
      void ShowCerts();
      const char * GetCipher();

   private:
      void InitSSL( bool = false );// Defaults to create a client context, true if server context needed
      void InitContext( bool );
      void LoadCertificates( const char *, const char * );

// Instance variables      
      void * Context;/// SSL context
      void * BIO;/// SSL BIO (Basic Input/Output)

};

#endif

