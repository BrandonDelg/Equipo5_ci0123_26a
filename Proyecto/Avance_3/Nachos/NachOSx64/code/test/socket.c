#include "syscall.h"

int strlen(const char *str) {
   const char *s;
   for (s = str; *s; ++s);
   return s - str;
}

char *strcat(char *s, const char *append) {
   char *save = s;
   for (; *s; ++s);
   while ((*s++ = *append++) != '\0');
   return save;
}

void quitarSalto(char *s) {
   int i = 0;
   while (s[i] != '\0') {
      if (s[i] == '\n' || s[i] == '\r') {
         s[i] = '\0';
         return;
      }
      i++;
   }
}

int inicioBody(char *resp, int n) {
   int i;

   for (i = 0; i < n - 3; i++) {
      if (resp[i] == '\r' &&
          resp[i + 1] == '\n' &&
          resp[i + 2] == '\r' &&
          resp[i + 3] == '\n') {
         return i + 4;
      }
   }

   return 0;
}

void cerrarServidor() {
   int id;
   char resp[512];
   char *req = "GET /shutdown HTTP/1.0\r\nUser-Agent: nachos\r\n\r\n";

   id = Socket(AF_INET_NachOS, SOCK_STREAM_NachOS);
   Connect(id, "127.0.0.1", 1234);

   Write(req, strlen(req), id);

   int n = Read(resp, 511, id);
   if (n > 0) {
      int inicio = inicioBody(resp, n);
      Write(resp + inicio, n - inicio, 1);
      Write("\n", 1, 1);
   }

   Close(id);
}

void pedirLista() {
   int id;
   char resp[512];
   char *req = "GET /figuras HTTP/1.0\r\nUser-Agent: nachos\r\n\r\n";

   id = Socket(AF_INET_NachOS, SOCK_STREAM_NachOS);
   Connect(id, "127.0.0.1", 1234);

   Write(req, strlen(req), id);

   int n = Read(resp, 511, id);
   if (n > 0) {
      int inicio = inicioBody(resp, n);
      Write(resp + inicio, n - inicio, 1);
   }

   Close(id);
}

void pedirFigura() {
   int id;
   char figura[50];
   char parte[10];
   char req[150];
   char resp[512];

   Write("Nombre figura: ", 15, 1);
   Read(figura, 50, 0);
   quitarSalto(figura);

   Write("Parte (1,2): ", 12, 1);
   Read(parte, 10, 0);
   quitarSalto(parte);

   req[0] = '\0';
   strcat(req, "GET /figura/");
   strcat(req, figura);
   strcat(req, "/");
   strcat(req, parte);
   strcat(req, " HTTP/1.0\r\nUser-Agent: nachos\r\n\r\n");

   id = Socket(AF_INET_NachOS, SOCK_STREAM_NachOS);
   Connect(id, "127.0.0.1", 1234);

   Write(req, strlen(req), id);

   int n = Read(resp, 511, id);
   if (n > 0) {
      int inicio = inicioBody(resp, n);
      Write(resp + inicio, n - inicio, 1);
   };

   Close(id);
}

int main() {
   char op[10];

   while (1) {
      Write("\n1) Listar figuras\n", 19, 1);
      Write("2) Pedir figura\n", 16, 1);
      Write("3) Salir\n", 9, 1);
      Write("Comando: ", 9, 1);

      Read(op, 10, 0);

      if (op[0] == '1') {
         pedirLista();
      } else if (op[0] == '2') {
         pedirFigura();
      } else if (op[0] == '3') {
         Write("Cerrando servidor...\n", 20, 1);
         cerrarServidor();
         Write("Saliendo...\n", 11, 1);
         Exit(0);
      } else {
         Write("Comando invalido\n", 17, 1);
      }
   }
}