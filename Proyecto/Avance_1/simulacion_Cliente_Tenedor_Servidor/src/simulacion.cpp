#include "Cliente.hpp"
#include "ServidorIntermedio.hpp"
#include "ServidorPiezas.hpp"
#include <pthread.h>
#include <cstring>

void* router_thread(void* arg) {
    ((ServidorIntermedio*)arg)->listen();
    return nullptr;
}

void* piezas_thread(void* arg) {
    ((ServidorPiezas*)arg)->listen();
    return nullptr;
}

int main() {
    Cliente cliente;
    ServidorIntermedio router;
    ServidorPiezas piezas;
    cliente.Connect(&router);
    router.ConnectServidor(&piezas);
    piezas.Connect(&router);
    pthread_t routerT, piezasT;
    pthread_create(&routerT, nullptr, router_thread, &router);
    pthread_create(&piezasT, nullptr, piezas_thread, &piezas);

    bool running = true;
    while(running) {
        int opcion;
        std::cout<<"\n1 Pedir mitad 1 figura car\n";
        std::cout<<"2 Pedir mitad 2 figura car\n";
        std::cout<<"3 Salir\n";

        std::cin >> opcion;
        if(opcion == 3){
            Message* msg = new Message();
            msg->type = CLOSE;
            cliente.send_to_server(msg);
            running = false;
            break;
        }
        Message* msg = new Message();
        
        msg->type = REQUEST_FIGURE;
        strcpy(msg->figura,"car");
        msg->mitad = opcion;
        cliente.send_to_server(msg);
        cliente.receive_from_server();


    }
    pthread_join(routerT, nullptr);
    pthread_join(piezasT, nullptr);

}