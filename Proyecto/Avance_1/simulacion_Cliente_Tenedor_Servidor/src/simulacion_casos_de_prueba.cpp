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
#include <fstream>
#include <vector>
#include <unistd.h>

/**
 * @brief Función de los hilos router(Servidor intermedio)
 *
 * @param arg Puntero a servidor intermedio
 */
void* router_thread(void* arg) {
    ((ServidorIntermedio*)arg)->listen();
    return nullptr;
}

/**
 * @brief Función de los hilos piezas(Servidor de piezas)
 *
 * @param arg Puntero a servidor de piezas
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

    std::vector<std::string> cdp = {
        "../../Casos_De_Prueba/test1.txt",
        "../../Casos_De_Prueba/test2.txt",
        "../../Casos_De_Prueba/test3.txt",
        "../../Casos_De_Prueba/test4.txt",
        "../../Casos_De_Prueba/test5.txt",
        "../../Casos_De_Prueba/test6.txt",
        "../../Casos_De_Prueba/test7.txt",
        "../../Casos_De_Prueba/test8.txt",
        "../../Casos_De_Prueba/test9.txt",
    };

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
    
    std::cout << "1) Solicitar lista de figuras\n";
    std::cout << "2) Solicitar figura\n";
    std::cout << "3) Salir\n";
    
    for (const auto& nombre : cdp) {
        
        std::ifstream file(nombre);
        
        if (!file.is_open()) {
            std::cerr << "Error abriendo archivo: " << nombre << "\n";
            continue;
        }
        
        std::cout << "\n\n\nEjecutando: " << nombre << "\n";
        
        std::string input;

        while (running) {
            std::cout << "\nOpcion: ";
            std::getline(file, input);
    
            Message* msg = new Message();
            msg->clientId = 1;
            if (input == "1") {
                msg->type = REQUEST_LIST;
                strcpy(msg->figura, "GET_FIGURES");
                msg->mitad = 0;
                std::cout << "[CLIENTE] Solicita lista de figuras \n";
    
            } else if (input == "2") {
                std::string linea;
                std::cout << "Ingrese figura y mitad (ej: Perro 1): ";
                std::getline(file, linea);
    
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
                std::string figuraProtocolo = "Figure_" + figura;
                strcpy(msg->figura, figuraProtocolo.c_str());
                msg->mitad = mitad;
    
                std::cout << "[CLIENTE] Solicita figura: " << figura << " mitad: " << mitad << "\n";
    
            } else if (input == "3") {

                std::cout << "[CLIENTE] Fin de este test\n";

                delete msg;
                break;

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
        
    }

    Message* msg = new Message();
    msg->clientId = 1;
    msg->type = CLOSE;

    std::cout << "\n[CLIENTE] Cerrando sistema completo\n";

    cliente.send_to_server(msg);


    pthread_join(routerT, nullptr);
    pthread_join(piezasT, nullptr);

    return 0;
}
