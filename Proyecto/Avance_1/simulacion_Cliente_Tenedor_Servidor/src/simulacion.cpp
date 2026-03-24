/**
 * @file simulacion.cpp
 * @brief Programa main de la simulación del cliente, intermediario y servidor
 */
#include "Cliente.hpp"
#include "ServidorIntermedio.hpp"
#include "ServidorPiezas.hpp"
#include <pthread.h>
#include <cstring>
#include <iostream>
#include <string>

/**
 * @brief Función de los hilos router(Servidor intermedio)
 * @param Puntero a servidor intermedio
 */
void* router_thread(void* arg) {
    ((ServidorIntermedio*)arg)->listen();
    return nullptr;
}

/**
 * @brief Función de los hilos piezas(Servidor de piezas)
 * @param Puntero a servidor de piezas
 */
void* piezas_thread(void* arg) {
    ((ServidorPiezas*)arg)->listen();
    return nullptr;
}

/**
 * @brief Simulación de comunicación
 *
 * Se inicializan el cliente, intermediario y servidor. Se conectan unos con otros 
 * y se colocan a escuchar los servidores. Se construye una solicitud y el cliente 
 * la envía
 */
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
            // Si se escribe exit se detiene la ejecución de todos los servidores y de la simulación
            msg->type = CLOSE;
            running = false;
        } else {
            msg->type = REQUEST_FIGURE;
            if (op == "GET_figures") {
                strcpy(msg->figura, "GET_figures");
            } else if (op.find("GET figure") == 0) {
                std::string resto = op.substr(4);
                size_t pos = resto.find(" ");
                int mitad;
                std::string figura = resto.substr(0, pos);
                if (isdigit(resto[pos + 1])) {
                    mitad = std::stoi(resto.substr(pos + 1));
                    msg->mitad = mitad;
                } else {
                    std::cout << "Mitad invalida, intente nuevamente" << std::endl;
                    continue;
                }
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
