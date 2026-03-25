/**
 * @file VSocket.hpp
 * @brief Definición de la clase virtual VSocket
 */
#ifndef VSocket_hpp
#define VSocket_hpp
 
/**
 * @brief Clase virtual VSocket
 *
 * Se encarga de manejar los metodos de conexión del socket, 
 * así como los metodos de envió y recepción de mensajes udp
 */
class VSocket {
   public:
       void Init( char type, bool IPv6 = false );

       /**
        * @brief Destructo de VSocket
        */
       virtual ~VSocket();

      void Close();

      int TryToConnect( const char * hostip, int port );

      int TryToConnect( const char * hostip, const char * service);

      /**
       * @brief Metodo virtual de conexion por puerto
       *
       * @param hostip Ip a la que se va a conectar
       * @param port Puerto al que se va a conectar
       *
       * @return Devuelve < 0 si falla, 1 si funciona
       */
      virtual int Connect( const char *, int ) = 0;

      /**
       * @brief Metodo virtual de conexion por servicio
       *
       * @param hostip Ip a la que se va a conectar
       * @param service Servicio al que se va a conectar
       *
       * @return Devuelve < 0 si falla, 1 si funciona
       */
      virtual int Connect( const char *, const char * ) = 0;

      /**
       * @brief Metodo virtual de lectura de mensajes
       *
       * @param buffer Buffer donde se lee el mensaje
       * @param size Tamaño del buffer
       *
       * @return Cantidad bytes leidos
       */
      virtual size_t Read( void * buffer, size_t size ) = 0;

      /**
       * @brief Metodo virtual de escritura de mensajes
       *
       * @param buffer Buffer donde se escribe el mensaje
       * @param size Tamaño del buffer
       *
       * @return Cantidad bytes escritos
       */
      virtual size_t Write( const void * buffer, size_t size ) = 0;

      /**
       * @brief Metodo virtual de escritura de mensajes
       *
       * @param buffer Buffer donde se escribe el mensaje
       *
       * @return Cantidad bytes escritos
       */
      virtual size_t Write( const char * buffer) = 0;

      int Bind( int port );

// UDP methods
      size_t sendTo( const void *, size_t, void * );
      size_t recvFrom( void *, size_t, void * );

   protected:
      int sockId;   /// Socket identifier
      bool IPv6;      /// Is IPv6 socket?
      int port;       /// Socket associated port
      char type;      /// Socket type (datagram, stream, etc.)
        
};

#endif // VSocket_h
