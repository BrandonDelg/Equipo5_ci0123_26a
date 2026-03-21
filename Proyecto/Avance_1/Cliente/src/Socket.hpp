#ifndef Socket_hpp
#define Socket_hpp
#include "VSocket.hpp"
#include <cstddef>

class Socket : public VSocket {

   public:
      Socket( char, bool = false );
      ~Socket();
      int Connect( const char *, int );
      int Connect( const char *, const char * );
      size_t Read( void *, size_t );
      size_t Write( const void *, size_t );
      size_t Write( const char * );

   protected:

};

#endif

