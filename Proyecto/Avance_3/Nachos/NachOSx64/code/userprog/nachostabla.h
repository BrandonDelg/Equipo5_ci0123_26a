#ifndef NACHOSTABLA_H
#define NACHOSTABLA_H

#include "bitmap.h"

#define MAX_OPEN_FILES 256

class NachosOpenFilesTable {
  public:
    NachosOpenFilesTable();          // Constructor
    ~NachosOpenFilesTable();         // Destructor

    int Open(int UnixHandle);       // Registrar archivo/socket
    int Close(int NachosHandle);    // Desregistrar
    bool isOpened(int NachosHandle);
    int getUnixHandle(int NachosHandle);

    void addThread();               // Un hilo más usando la tabla
    void delThread();               // Un hilo menos usando la tabla

    void Print();

  private:
    int *openFiles;                 // Vector de handles Unix
    BitMap *openFilesMap;           // Bitmap de ocupación
    int usage;                      // Cantidad de hilos usando la tabla
};

#endif