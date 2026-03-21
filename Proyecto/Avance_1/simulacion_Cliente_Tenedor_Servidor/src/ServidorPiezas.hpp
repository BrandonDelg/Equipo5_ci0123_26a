#include <queue>
#include <pthread.h>

class ServidorIntermedio;
struct Message;

class ServidorPiezas {
    public:
        ServidorPiezas();
        ~ServidorPiezas();
        void Connect(ServidorIntermedio* r);
        void listen();
        void Stop();
        void procesarSolicitud(Message* msg);
        pthread_mutex_t* getMutex();
        pthread_cond_t* getVC();
        std::queue<Message*>& getQueue();

    private:
        ServidorIntermedio* router;
        std::queue<Message*> queue;
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        bool running;
    };