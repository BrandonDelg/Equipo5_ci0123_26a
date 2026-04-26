#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <fstream>
#include <vector>
#include <string>

#define BLOCK_SIZE 256

struct SuperBlock {
    int blockSize;
    int totalBlocks;
    int rootDirBlock;
    int bitmapBlock;
};

struct DirEntry {
    char nombre[32];
    int inodeBlock;
};

struct DirBlock {
    DirEntry entries[4];
    int nextBlock;
};

struct Inode {
    int size;
    int blocks[8];
};

class FileSystem {
    private:
        std::fstream disk;

    public:
        FileSystem(const std::string& filename);

        void init();

        void readBlock(int blockNum, char* buffer);
        void writeBlock(int blockNum, const char* buffer);

        int allocateBlock();

        void crearFigura(const std::string& nombre, const std::string& contenido);

        std::vector<std::string> getFiguras();
        int buscarFigura(const std::string& nombre);
        std::string getPiezas(const std::string& figura, int mitad);
        void freeBlock(int blockNum);
        bool borrarFigura(const std::string& nombre);


};

#endif