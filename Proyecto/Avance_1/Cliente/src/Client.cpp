#include <stdio.h>
#include <string.h>

#include "VSocket.hpp"
#include "Socket.hpp"
#include "SSLSocket.hpp"

#define MAXBUF 1024

int main( int argc, char * argv[] ) {

   VSocket * client;
   int st, port = 443;
   char a[MAXBUF];

   char * os = (char *) "os.ecci.ucr.ac.cr";

   char * request = (char *)
   "GET /lego/index.php HTTP/1.1\r\n"
   "Host: os.ecci.ucr.ac.cr\r\n"
   "Connection: close\r\n"
   "\r\n";

   if (argc > 1) {
      port = 443;
      client = new SSLSocket();
   } else {
      client = new Socket('s');
      port = 80;
   }

   memset(a, 0, MAXBUF);

   client->Connect(os, "http");
   client->Write(request, strlen(request));

   //st = client->Read(a, MAXBUF);
   while ((st = client->Read(a, MAXBUF-1)) > 0) {
      a[st] = 0;
      printf("%s", a);
   }
}