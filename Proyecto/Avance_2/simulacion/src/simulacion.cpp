/**
 * @file simulacion.cpp
 * @brief Programa main de la simulación del cliente, intermediario y servidor
 *
 * Se simulan dos servidores intermediarios (SI-01 y SI-02) cada uno con su
 * propio servidor de piezas (isla-01 e isla-02). Ambos intermediarios se
 * registran como peers entre sí. Un hilo de heartbeat por cada par SI↔SP
 * corre de forma independiente. El cliente se conecta a SI-01.
 */
#include "Cliente.hpp"
#include "ServidorIntermedio.hpp"
#include "ServidorPiezas.hpp"
#include "Parser.hpp"
#include <pthread.h>
#include <cstring>
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
 * @brief Función de los hilos router (Servidor intermedio escucha cliente)
 *
 * @param arg Puntero a servidor intermedio
 */
void* router_thread(void* arg) {
    ((ServidorIntermedio*)arg)->listen();
    return nullptr;
}

/**
 * @brief Función de los hilos piezas (Servidor de piezas)
 *
 * @param arg Puntero a servidor de piezas
 */
void* piezas_thread(void* arg) {
    ((ServidorPiezas*)arg)->listen();
    return nullptr;
}

/**
 * @brief Hilo de heartbeat periódico
 *
 * El servidor intermediario envía HEARTBEAT al servidor de piezas
 * cada 5 segundos mientras la simulación esté activa.
 *
 * @param arg Puntero a HeartbeatArgs
 */
void* heartbeat_thread(void* arg) {
    HeartbeatArgs* heartbeatArgs = (HeartbeatArgs*)arg;
    while (*(heartbeatArgs->running)) {
        sleep(5);
        if (*(heartbeatArgs->running)) {
            heartbeatArgs->router->sendHeartbeat();
        }
    }
    return nullptr;
}

// void* cliente_listener(void* arg) {
//     Cliente* cliente = (Cliente*)arg;
//     while (cliente->running) {
//         if (!cliente->receive_from_server()) break;
//     }
//     return nullptr;
// }

/**
 * @brief Simulación de comunicación con múltiples peers
 *
 * Ambos intermediarios se sincronizan al inicio con SYNC_REQUEST/REPLY.
 * Cada SI tiene su propio hilo de heartbeat hacia su SP local.
 * El cliente opera contra SI-01; las notificaciones NOTIFY_NEW y
 * NOTIFY_DROP se propagan automáticamente entre peers.
 */
int main() {
    Cliente cliente;

    ServidorIntermedio router("SI-01");

    ServidorPiezas piezas("SP-01");

    Parser parser;

    // Conectar cliente al primer intermediario
    cliente.Connect(&router);

    // Conectar cada intermediario con su SP local
    router.ConnectServidor(&piezas);

    piezas.Connect(&router);

    bool running = true;

    HeartbeatArgs heartbeatArgs = { &router, &piezas, &running };

    pthread_t routerT, piezasT, heartbeatT;
    // pthread_t clienteT;
    // pthread_create(&clienteT, nullptr, cliente_listener, &cliente);
    pthread_create(&routerT, nullptr, router_thread, &router);
    pthread_create(&piezasT, nullptr, piezas_thread, &piezas);
    pthread_create(&heartbeatT, nullptr, heartbeat_thread, &heartbeatArgs);


    sleep(1);

    std::string input;

    while (running) {
        std::string menu = "1) Solicitar lista de figuras\n2) Solicitar figura\n3) Salir";
        std::cout << menu << std::endl;
        std::cout << "\nOpcion: ";
        std::getline(std::cin, input);

        std::string msg = "";

        if (input == "1") {
            msg += "10|";
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

            msg += "14|";
            msg += "figura=" + figura + ";mitad=" + std::to_string(mitad);

            std::cout << "[CLIENTE] Solicita figura: " << figura << " mitad: " << mitad << "\n";

        } else if (input == "3") {
            msg += "02|";
            running = false;
            cliente.running = false;
            std::cout << "[CLIENTE] Cerrando conexión\n";

        } else {
            std::cout << "Opcion inválida\n";
            continue;
        }

        cliente.send_to_server(msg);

        int tipoMsg = parser.getTipo(msg);
        if (tipoMsg == 02) {
            break;
        }

        cliente.receive_from_server();
    }

    pthread_join(heartbeatT, nullptr);
    pthread_join(routerT, nullptr);
    pthread_join(piezasT, nullptr);
    // pthread_join(clienteT, nullptr);

    return 0;
}
