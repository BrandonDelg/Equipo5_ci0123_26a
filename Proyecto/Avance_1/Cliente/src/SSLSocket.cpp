// SSL includes
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <stdexcept>

#include "SSLSocket.hpp"
#include "Socket.hpp"

/**
  *  Class constructor
  *     use base class
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool ipv6: if we need a IPv6 socket
  *
 **/
SSLSocket::SSLSocket( bool IPv6 ) {

   this->Init( 's', IPv6 );

   this->Context = nullptr;
   this->BIO = nullptr;

   this->InitSSL();					// Initializes to client context

}


/**
  *  Class constructor
  *     use base class
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool IPv6: if we need a IPv6 socket
  *
 **/
SSLSocket::SSLSocket(char * certFileName, char * keyFileName, bool IPv6) {

   this->Init('s', IPv6);

   const SSL_METHOD * method = TLS_server_method();
   SSL_CTX * ctx = SSL_CTX_new(method);

   if (!ctx) {
      throw std::runtime_error("SSLSocket: failed to create SSL_CTX");
   }

   this->Context = ctx;
   this->LoadCertificates(certFileName, keyFileName);
}


/**
  *  Class constructor
  *
  *  @param     int id: socket descriptor
  *
 **/
SSLSocket::SSLSocket( int id ) {

   this->Init( id );
   this->Context = nullptr;
   this->BIO = nullptr;

}


/**
  * Class destructor
  *
 **/
SSLSocket::~SSLSocket() {

// SSL destroy
   if ( nullptr != this->Context ) {
      SSL_CTX_free( reinterpret_cast<SSL_CTX *>( this->Context ) );
   }
   SSL * ssl = (SSL *) this->BIO;
   if (ssl) {
      SSL_shutdown(ssl);
      SSL_free(ssl);
   }

   this->Close();
}


/**
  *  InitSSL
  *     use SSL_new with a defined context
  *
  *  Create a SSL object
  *
 **/
void SSLSocket::InitSSL( bool serverContext ) {
   this->InitContext( serverContext );
}


/**
  *  InitContext
  *     use SSL_library_init, OpenSSL_add_all_algorithms, SSL_load_error_strings, TLS_server_method, SSL_CTX_new
  *
  *  Creates a new SSL server context to start encrypted comunications, this context is stored in class instance
  *
 **/
void SSLSocket::InitContext(bool serverContext) {

   SSL_library_init();
   OpenSSL_add_all_algorithms();
   SSL_load_error_strings();

   const SSL_METHOD *method;
   SSL_CTX *context;

   if (serverContext) {
      method = TLS_server_method();
   } else {
      method = TLS_client_method();
   }

   context = SSL_CTX_new(method);
   if (!context) {
      throw std::runtime_error("SSLSocket::InitContext()");
   }

   this->Context = context;
}


/**
 *  Load certificates
 *    verify and load certificates
 *
 *  @param	const char * certFileName, file containing certificate
 *  @param	const char * keyFileName, file containing keys
 *
 **/
void SSLSocket::LoadCertificates(const char * certFileName, const char * keyFileName) {

   SSL_CTX * ctx = reinterpret_cast<SSL_CTX *>(this->Context);

   if (!ctx) {
      throw std::runtime_error("SSLSocket::LoadCertificates: Context is null");
   }

   if (SSL_CTX_use_certificate_file(ctx, certFileName, SSL_FILETYPE_PEM) <= 0) {
      ERR_print_errors_fp(stderr);
      throw std::runtime_error("Error loading certificate file");
   }

   if (SSL_CTX_use_PrivateKey_file(ctx, keyFileName, SSL_FILETYPE_PEM) <= 0) {
      ERR_print_errors_fp(stderr);
      throw std::runtime_error("Error loading private key file");
   }

   if (!SSL_CTX_check_private_key(ctx)) {
      throw std::runtime_error("Private key does not match the certificate public key");
   }
}

/**
 *  Connect
 *     use SSL_connect to establish a secure conection
 *
 *  Create a SSL connection
 *
 *  @param	char * hostName, host name
 *  @param	int port, service number
 *
 **/
int SSLSocket::Connect(const char *hostName, int port) {

   int st = this->TryToConnect(hostName, port);

   if (st < 0) return st;

   SSL *ssl = SSL_new((SSL_CTX*)this->Context);

   SSL_set_fd(ssl, this->sockId);

   this->BIO = ssl;

   st = SSL_connect(ssl);
   if (st != 1) {
      ERR_print_errors_fp(stderr);
      return -1;
   }

   return 0;
}


/**
 *  Connect
 *     use SSL_connect to establish a secure conection
 *
 *  Create a SSL connection
 *
 *  @param	char * hostName, host name
 *  @param	char * service, service name
 *
 **/
int SSLSocket::Connect(const char * host, const char * service) {

   int st = this->TryToConnect(host, service);
   if (st < 0) {
      return st;
   }
   SSL * ssl = SSL_new((SSL_CTX *) this->Context);

   SSL_set_fd(ssl, this->sockId);

   this->BIO = ssl;
   st = SSL_connect(ssl);
   if (st != 1) {
      ERR_print_errors_fp(stderr);
      return -1;
   }

   return 0;
}

/**
  *  Read
  *     use SSL_read to read data from an encrypted channel
  *
  *  @param	void * buffer to store data read
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity read
  *
  *  Reads data from secure channel
  *
 **/
size_t SSLSocket::Read( void * buffer, size_t size ) {
   SSL * ssl = (SSL *) this->BIO;
   int st = SSL_read(ssl, buffer, size);
   if ( -1 == st ) {
      throw std::runtime_error( "SSLSocket::Read( void *, size_t )" );
   }
   return st;

}


/**
  *  Write
  *     use SSL_write to write data to an encrypted channel
  *
  *  @param	void * buffer to store data read
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity written
  *
  *  Writes data to a secure channel
  *
 **/
size_t SSLSocket::Write( const char * string ) {
   SSL * ssl = (SSL *) this->BIO;
   int st = SSL_write(ssl, string, strlen(string));
   if ( st <= 0 ) {
      throw std::runtime_error( "SSLSocket::Write( const char * )" );
   }

   return st;

}


/**
  *  Write
  *     use SSL_write to write data to an encrypted channel
  *
  *  @param	const void * buffer to store data to write
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity written
  *
  *  Reads data from secure channel
  *
 **/
size_t SSLSocket::Write( const void * buffer, size_t size ) {
   SSL * ssl = (SSL *) this->BIO;
   int st = SSL_write(ssl, buffer, size);
   if (  st <= 0 ) {
      throw std::runtime_error( "SSLSocket::Write( void *, size_t )" );
   }

   return st;

}


/**
 *   Show SSL certificates
 *
 **/
void SSLSocket::ShowCerts() {
   X509 *cert;
   char *line;

   cert = SSL_get_peer_certificate( reinterpret_cast<SSL *>( this->BIO ) );		 // Get certificates (if available)
   if ( nullptr != cert ) {
      printf("Server certificates:\n");
      line = X509_NAME_oneline( X509_get_subject_name( cert ), 0, 0 );
      printf( "Subject: %s\n", line );
      free( line );
      line = X509_NAME_oneline( X509_get_issuer_name( cert ), 0, 0 );
      printf( "Issuer: %s\n", line );
      free( line );
      X509_free( cert );
   } else {
      printf( "No certificates.\n" );
   }

}


/**
 *   Return the name of the currently used cipher
 *
 **/
const char * SSLSocket::GetCipher() {

   return SSL_get_cipher( reinterpret_cast<SSL *>( this->BIO ) );

}

