#include "FileSystem.hpp"
#include <cstring>
#include <sstream>
#include <algorithm>
#include <iostream>
FileSystem::FileSystem(const std::string& filename) {
    disk.open(filename, std::ios::in | std::ios::out | std::ios::binary);

    if (!disk.is_open()) {
        disk.open(filename, std::ios::out | std::ios::binary);
        disk.close();

        disk.open(filename, std::ios::in | std::ios::out | std::ios::binary);

        init();
    }
}

void FileSystem::init() {
    char buffer[BLOCK_SIZE] = {0};

    SuperBlock sb;
    sb.blockSize = BLOCK_SIZE;
    sb.totalBlocks = 100;
    sb.rootDirBlock = 2;
    sb.bitmapBlock = 1;

    memcpy(buffer, &sb, sizeof(sb));
    writeBlock(0, buffer);

    memset(buffer, 0, BLOCK_SIZE);
    buffer[0] = 1; // superblock
    buffer[1] = 1; // bitmap
    buffer[2] = 1; // root dir
    writeBlock(1, buffer);

    DirBlock dir = {};
    dir.nextBlock = -1;

    memset(buffer, 0, BLOCK_SIZE);
    memcpy(buffer, &dir, sizeof(dir));
    writeBlock(2, buffer);
}


void FileSystem::readBlock(int blockNum, char* buffer) {
    disk.seekg(blockNum * BLOCK_SIZE);
    disk.read(buffer, BLOCK_SIZE);
}

void FileSystem::writeBlock(int blockNum, const char* buffer) {
    disk.seekp(blockNum * BLOCK_SIZE);
    disk.write(buffer, BLOCK_SIZE);
    disk.flush();
}


int FileSystem::allocateBlock() {
    char buffer[BLOCK_SIZE];
    readBlock(1, buffer);

    for (int i = 0; i < BLOCK_SIZE; i++) {
        if (buffer[i] == 0) {
            buffer[i] = 1;
            writeBlock(1, buffer);
            return i;
        }
    }
    return -1;
}

void FileSystem::crearFigura(const std::string& nombre, const std::string& contenido) {
    Inode inode;
    inode.size = contenido.size();
    if (buscarFigura(nombre) != -1) {
        std::cout << "La figura ya existe: " << nombre << std::endl;
        return;
    }

    for (int i = 0; i < 8; i++) {
        inode.blocks[i] = -1;
    }

    int bytesWritten = 0;
    int blockIndex = 0;

    while (bytesWritten < (int)contenido.size() && blockIndex < 8) {
        int block = allocateBlock();
        char buffer[BLOCK_SIZE] = {0};
        int chunk = std::min(BLOCK_SIZE, (int)contenido.size() - bytesWritten);
        memcpy(buffer, contenido.c_str() + bytesWritten, chunk);
        writeBlock(block, buffer);
        inode.blocks[blockIndex++] = block;
        bytesWritten += chunk;
    }

    int inodeBlock = allocateBlock();

    char buffer[BLOCK_SIZE] = {0};
    memcpy(buffer, &inode, sizeof(inode));
    writeBlock(inodeBlock, buffer);

    int current = 2;

    while (true) {
        readBlock(current, buffer);
        DirBlock* dir = (DirBlock*)buffer;

        for (int i = 0; i < 4; i++) {
            if (dir->entries[i].nombre[0] == '\0') {
                strcpy(dir->entries[i].nombre, nombre.c_str());
                dir->entries[i].inodeBlock = inodeBlock;

                writeBlock(current, buffer);
                return;
            }
        }

        if (dir->nextBlock == -1) {
            int newBlock = allocateBlock();

            DirBlock newDir = {};
            newDir.nextBlock = -1;

            char newBuffer[BLOCK_SIZE] = {0};
            memcpy(newBuffer, &newDir, sizeof(newDir));
            writeBlock(newBlock, newBuffer);

            dir->nextBlock = newBlock;
            writeBlock(current, buffer);

            current = newBlock;
        } else {
            current = dir->nextBlock;
        }
    }
}

std::vector<std::string> FileSystem::getFiguras() {
    std::vector<std::string> figuras;
    char buffer[BLOCK_SIZE];

    int current = 2;

    while (current != -1) {
        readBlock(current, buffer);
        DirBlock* dir = (DirBlock*)buffer;

        for (int i = 0; i < 4; i++) {
            if (dir->entries[i].nombre[0] != '\0') {
                figuras.push_back(dir->entries[i].nombre);
            }
        }

        current = dir->nextBlock;
    }

    return figuras;
}
int FileSystem::buscarFigura(const std::string& nombre) {
    char buffer[BLOCK_SIZE];
    int current = 2;

    while (current != -1) {
        readBlock(current, buffer);
        DirBlock* dir = (DirBlock*)buffer;

        for (int i = 0; i < 4; i++) {
            if (nombre == dir->entries[i].nombre) {
                return dir->entries[i].inodeBlock;
            }
        }

        current = dir->nextBlock;
    }

    return -1;
}

std::string FileSystem::getPiezas(const std::string& figura, int mitad) {
    int inodeBlock = buscarFigura(figura);

    if (inodeBlock == -1)
        return "Figura no encontrada\n";

    char buffer[BLOCK_SIZE];
    readBlock(inodeBlock, buffer);

    Inode* inode = (Inode*)buffer;

    std::string contenido;
    int remaining = inode->size;

    for (int i = 0; i < 8 && remaining > 0; i++) {
        if (inode->blocks[i] == -1) break;

        char data[BLOCK_SIZE];
        readBlock(inode->blocks[i], data);

        int chunk = std::min(BLOCK_SIZE, remaining);

        contenido.append(data, chunk);

        remaining -= chunk;
    }

    std::istringstream iss(contenido);
    std::string linea, resultado;
    bool leyendo = false;

    while (std::getline(iss, linea)) {
        if (linea == std::to_string(mitad)) {
            leyendo = true;
            continue;
        }

        if (leyendo) {
            if (linea == "1" || linea == "2") break;
            resultado += linea + "\n";
        }
    }

    return resultado;
}

void FileSystem::freeBlock(int blockNum) {
    char buffer[BLOCK_SIZE];
    readBlock(1, buffer);
    buffer[blockNum] = 0;
    writeBlock(1, buffer);
}

bool FileSystem::borrarFigura(const std::string& nombre) {
    char buffer[BLOCK_SIZE];
    int current = 2;

    while (current != -1) {
        readBlock(current, buffer);
        DirBlock* dir = (DirBlock*)buffer;
        for (int i = 0; i < 4; i++) {
            if (nombre == dir->entries[i].nombre) {
                int inodeBlock = dir->entries[i].inodeBlock;
                char inodeBuffer[BLOCK_SIZE];
                readBlock(inodeBlock, inodeBuffer);
                Inode* inode = (Inode*)inodeBuffer;

                for (int j = 0; j < 8; j++) {
                    if (inode->blocks[j] != -1) {
                        freeBlock(inode->blocks[j]);
                    }
                }

                freeBlock(inodeBlock);

                memset(&dir->entries[i], 0, sizeof(dir->entries[i]));
                writeBlock(current, buffer);
                return true;
            }
        }
        current = dir->nextBlock;
    }

    return false;
}