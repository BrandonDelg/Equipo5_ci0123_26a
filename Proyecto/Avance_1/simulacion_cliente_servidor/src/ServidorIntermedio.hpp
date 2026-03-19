#include <queue>
#include <pthread.h>

class Cliente;
class ServidorPiezas;
struct Message;

class ServidorIntermedio {
    public:
        ServidorIntermedio();
        ~ServidorIntermedio();
        void Connect(Cliente* c);
        void ConnectServidor(ServidorPiezas* s);
        void listen();
        pthread_mutex_t* getMutex();
        pthread_cond_t* getVC();
        std::queue<Message*>& getQueue();

    private:
        Cliente* cliente;
        ServidorPiezas* servidor;
        std::queue<Message*> respuestas;
        pthread_mutex_t mutex;
        pthread_cond_t cond;
};