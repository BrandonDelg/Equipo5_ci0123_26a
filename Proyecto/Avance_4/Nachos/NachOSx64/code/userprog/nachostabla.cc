#include "nachostabla.h"

#include <stdio.h>
#include <unistd.h>

NachosOpenFilesTable::NachosOpenFilesTable() {
    openFiles = new int[MAX_OPEN_FILES];

    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        openFiles[i] = -1;
    }

    openFilesMap = new BitMap(MAX_OPEN_FILES);

    openFilesMap->Mark(0); // stdi
    openFilesMap->Mark(1); // stdout
    openFilesMap->Mark(2); // stderr

    openFiles[0] = 0;
    openFiles[1] = 1;
    openFiles[2] = 2;

    usage = 0;
}
NachosOpenFilesTable::~NachosOpenFilesTable() {

    for (int i = 3; i < MAX_OPEN_FILES; i++) {
        if (openFilesMap->Test(i)) {
            close(openFiles[i]);
        }
    }

    delete[] openFiles;
    delete openFilesMap;
}

int NachosOpenFilesTable::Open(int UnixHandle) {

    int nachosHandle = openFilesMap->Find();

    if (nachosHandle == -1) {
        return -1;
    }

    openFiles[nachosHandle] = UnixHandle;

    return nachosHandle;
}

int NachosOpenFilesTable::Close(int NachosHandle) {

    if (!isOpened(NachosHandle)) {
        return -1;
    }

    int unixHandle = openFiles[NachosHandle];

    openFiles[NachosHandle] = -1;

    openFilesMap->Clear(NachosHandle);

    return unixHandle;
}

bool NachosOpenFilesTable::isOpened(int NachosHandle) {

    if (NachosHandle < 0 || NachosHandle >= MAX_OPEN_FILES) {
        return false;
    }

    return openFilesMap->Test(NachosHandle);
}

int NachosOpenFilesTable::getUnixHandle(int NachosHandle) {

    if (!isOpened(NachosHandle)) {
        return -1;
    }

    return openFiles[NachosHandle];
}

void NachosOpenFilesTable::addThread() {
    usage++;
}

void NachosOpenFilesTable::delThread() {

    usage--;

    if (usage <= 0) {
        for (int i = 3; i < MAX_OPEN_FILES; i++) {
            if (openFilesMap->Test(i)) {
                close(openFiles[i]);
                openFiles[i] = -1;
                openFilesMap->Clear(i);
            }
        }
    }
}

void NachosOpenFilesTable::Print() {

    printf("\n===== Tabla de archivos abiertos =====\n");

    for (int i = 0; i < MAX_OPEN_FILES; i++) {

        if (openFilesMap->Test(i)) {

            printf("NachosHandle %d -> UnixHandle %d\n",
                   i,
                   openFiles[i]);
        }
    }

    printf("Threads usando tabla: %d\n", usage);
}