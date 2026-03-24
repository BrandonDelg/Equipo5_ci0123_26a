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
#include <sstream>

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

    bool running = true;
    std::string input;

    std::cout << "1) Solicitar lista de figuras\n";
    std::cout << "2) Solicitar figura\n";
    std::cout << "3) Salir\n";

    while (running) {
        std::cout << "\nOpcion: ";
        std::getline(std::cin, input);

        Message* msg = new Message();

        if (input == "1") {
            msg->type = REQUEST_FIGURE;
            strcpy(msg->figura, "GET_figures");
            msg->mitad = 0;

            std::cout << "[CLIENTE] Solicita lista de figuras\n";

        } else if (input == "2") {

            std::string linea;
            std::cout << "Ingrese figura y mitad (ej: Perro 1): ";
            std::getline(std::cin, linea);

            std::istringstream iss(linea);
            std::string figura;
            int mitad;

            if (!(iss >> figura >> mitad)) {
                std::cout << "Formato invalido. Use: Perro 1\n";
                delete msg;
                continue;
            }

            if (mitad != 1 && mitad != 2) {
                std::cout << "Mitad invalida (solo 1 o 2)\n";
                delete msg;
                continue;
            }

            msg->type = REQUEST_FIGURE;
            std::string figuraProtocolo = "figure_" + figura;
            strcpy(msg->figura, figuraProtocolo.c_str());
            msg->mitad = mitad;

            std::cout << "[CLIENTE] Solicita figura: "
                      << figura << " mitad: " << mitad << "\n";

        } else if (input == "3") {
            msg->type = CLOSE;
            running = false;

            std::cout << "[CLIENTE] Cerrando conexion\n";

        } else {
            std::cout << "Opcion invalida\n";
            delete msg;
            continue;
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