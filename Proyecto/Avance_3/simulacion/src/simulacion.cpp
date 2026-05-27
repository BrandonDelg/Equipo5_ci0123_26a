/**
 * @file simulacion.cpp
 * @brief Programa main de la simulación — protocolo v3.0
 *
 * Simula la interacción entre Cliente, ServidorIntermedio y ServidorPiezas.
 * El cliente envía solicitudes en protocolo v3.0:
 *   P/R/          → solicitar lista de figuras
 *   P/G/fig/mitad → solicitar figura específica
 *   P/Q/          → cerrar conexión
 *
 * Las señales internas SI↔SP (01|, 02|, 03|, 90|) no cambian.
 */
#include "Cliente.hpp"
#include "ServidorIntermedio.hpp"
#include "ServidorPiezas.hpp"
#include <pthread.h>
#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>

/**
 * @brief Argumentos para el hilo de heartbeat
 */
struct HeartbeatArgs {
    ServidorIntermedio* router;
    ServidorPiezas* piezas;
    bool* running;
};

/**
 * @brief Hilo del servidor intermediario
 */
void* router_thread(void* arg) {
    ((ServidorIntermedio*)arg)->listen();
    return nullptr;
}

/**
 * @brief Hilo del servidor de piezas
 */
void* piezas_thread(void* arg) {
    ((ServidorPiezas*)arg)->listen();
    return nullptr;
}

/**
 * @brief Hilo de heartbeat periódico (cada 5 segundos)
 *
 * Verifica la bandera running cada 100 ms para salir rápido
 * cuando el usuario cierra la simulación.
 */
void* heartbeat_thread(void* arg) {
    HeartbeatArgs* hb = (HeartbeatArgs*)arg;
    int ticks = 0;
    while (*(hb->running)) {
        usleep(100000);
        if (!*(hb->running)) break;
        if (++ticks >= 50) {
            ticks = 0;
            hb->router->sendHeartbeat();
        }
    }
    return nullptr;
}

int main() {
    Cliente            cliente;
    ServidorIntermedio router("SI-01");
    ServidorPiezas     piezas("SP-01");

    cliente.Connect(&router);
    router.ConnectServidor(&piezas);
    piezas.Connect(&router);

    bool running = true;
    HeartbeatArgs heartbeatArgs = { &router, &piezas, &running };

    pthread_t routerT, piezasT, heartbeatT;
    pthread_create(&routerT,    nullptr, router_thread,    &router);
    pthread_create(&piezasT,    nullptr, piezas_thread,    &piezas);
    pthread_create(&heartbeatT, nullptr, heartbeat_thread, &heartbeatArgs);

    sleep(1);

    std::string input;

    while (running) {
        std::cout << "\n1) Solicitar lista de figuras\n"
                     "2) Solicitar figura\n"
                     "3) Salir\n"
                     "\nOpcion: ";
        if (!std::getline(std::cin, input)) break;

        std::string msg;

        if (input == "1") {
            msg = "P/R/";
            std::cout << "[CLIENTE] Solicita lista de figuras\n";

        } else if (input == "2") {
            std::string linea;
            std::cout << "Ingrese figura y mitad (ej: Perro 1): ";
            std::getline(std::cin, linea);

            std::istringstream iss(linea);
            std::string figura;
            int mitad;

            if (!(iss >> figura >> mitad)) {
                std::cout << "Formato inválido. Use: Perro 1\n";
                continue;
            }
            if (mitad != 1 && mitad != 2) {
                std::cout << "Mitad inválida (solo 1 o 2)\n";
                continue;
            }

            msg = "P/G/" + figura + "/" + std::to_string(mitad);
            std::cout << "[CLIENTE] Solicita figura: " << figura << " mitad: " << mitad << "\n";

        } else if (input == "3") {
            msg = "P/Q/";
            running = false;
            cliente.running = false;
            std::cout << "[CLIENTE] Cerrando conexión\n";

        } else {
            std::cout << "Opcion inválida\n";
            continue;
        }

        cliente.send_to_server(msg);

        if (msg == "P/Q/") {
            break;
        }

        cliente.receive_from_server();
    }

    pthread_join(heartbeatT, nullptr);
    pthread_join(routerT,    nullptr);
    pthread_join(piezasT,    nullptr);

    return 0;
}
