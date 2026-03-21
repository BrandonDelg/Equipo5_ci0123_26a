#include <iostream>
#include <queue>
#include <pthread.h>

enum Type {
    REQUEST_FIGURE,
    RESPONSE_PIECES,
    CLOSE
};

struct Message {
    Type type;
    int mitad;
    char figura[256];
    char message[256];
};

class ServidorIntermedio;

class Cliente {
    public:
        Cliente();
        ~Cliente();
        void Connect(ServidorIntermedio* serv);
        void send_to_server(void* package);
        bool receive_from_server();
        std::queue<Message*>& getQueue();
        pthread_cond_t* getVC();
        pthread_mutex_t* getMutex();

    private:
        ServidorIntermedio* server;
        std::queue<Message*> messageQueue;
        pthread_mutex_t queueMutex;
        pthread_cond_t queueCond;
};