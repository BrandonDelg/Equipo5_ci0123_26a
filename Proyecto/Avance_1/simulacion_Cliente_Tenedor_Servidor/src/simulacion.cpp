#include "Cliente.hpp"
#include "ServidorIntermedio.hpp"
#include "ServidorPiezas.hpp"
#include <pthread.h>
#include <cstring>
#include <iostream>
#include <string>

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

    std::string op;
    bool running = true;

    std::cout << "Use: GET_figures\n";
    std::cout << "Use: GET figure_Name Mitad(1|2) Ejemplo: GET figure_Perro 1\n";
    std::cout << "Use: Exit\n";

    while (running) {
        std::getline(std::cin, op);

        Message* msg = new Message();

        if (op == "Exit") {
            msg->type = CLOSE;
            running = false;
        } else {
            msg->type = REQUEST_FIGURE;
            if (op == "GET_figures") {
                strcpy(msg->figura, "GET_figures");
            } else if (op.find("GET ") == 0) {
                std::string resto = op.substr(4);
                size_t pos = resto.find(" ");

                std::string figura = resto.substr(0, pos);
                int mitad = std::stoi(resto.substr(pos + 1));

                msg->mitad = mitad;
                strcpy(msg->figura, figura.c_str());

            } else {
                strcpy(msg->figura, op.c_str());
            }
        }
        cliente.send_to_server(msg);

        if (msg->type == CLOSE) {
            break;
        }

        cliente.receive_from_server();
    }

    pthread_join(routerT, nullptr);
    pthread_join(piezasT, nullptr);

    return 0;
}