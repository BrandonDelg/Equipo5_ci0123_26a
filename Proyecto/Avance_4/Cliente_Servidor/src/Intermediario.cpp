#include "Intermediario.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <sstream>

Intermediario::Intermediario(VSocket* fork, char* host, const char* port)
    : SERVER_HOST(host), SERVER_PORT(port), intermediario(fork) {
}

Intermediario::~Intermediario() {
    if (intermediario != nullptr) {
        intermediario->Close();
        delete intermediario;
    }
}
void Intermediario::task(VSocket* client, bool ipv6) {
    std::string figura;
    int parte = 0;
    char buffer[BUFSIZE] = {0};
    int st;
    std::string request;

    while ((st = client->Read(buffer, BUFSIZE - 1)) > 0) {
        buffer[st] = '\0';
        request += buffer;

        if (request.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    std::cout << "HTTP recibido:\n" << request << std::endl;

    if (request.empty()) {
        std::string http = "HTTP/1.1 400 Bad Request\r\n\r\nSolicitud vacia";
        client->Write(http.c_str(), http.length());
        client->Close();
        delete client;
        return;
    }

    if (request.find("GET /favicon.ico") != std::string::npos) {
        std::string http = "HTTP/1.1 204 No Content\r\n\r\n";
        client->Write(http.c_str(), http.length());
        client->Close();
        delete client;
        return;
    }

    std::string rutaServidor;
    bool valid = false;

    if (request.find("GET /shutdown") != std::string::npos) {
        std::cout << "[INTERMEDIARIO] Shutdown solicitado\n";

        std::string response = consultarServidorLocal("/shutdown", ipv6);

        std::string http =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "Apagando servidor e intermediario";

        client->Write(http.c_str(), http.length());
        client->Close();
        delete client;

        exit(0);
    }

    if (request.find("GET /figuras") != std::string::npos) {
        rutaServidor = "/figuras";
        valid = true;
    } else if (request.find("GET /figura/") != std::string::npos) {
        size_t start = request.find("/figura/") + 8;
        size_t end = request.find(" ", start);

        if (end == std::string::npos) {
            std::string http = "HTTP/1.1 400 Bad Request\r\n\r\nFormato invalido";
            client->Write(http.c_str(), http.length());
            client->Close();
            delete client;
            return;
        }

        std::string ruta = request.substr(start, end - start);
        size_t slash = ruta.find("/");

        if (slash == std::string::npos) {
            std::string http = "HTTP/1.1 400 Bad Request\r\n\r\nFalta mitad";
            client->Write(http.c_str(), http.length());
            client->Close();
            delete client;
            return;
        }

        figura = ruta.substr(0, slash);
        std::string parteStr = ruta.substr(slash + 1);

        if (figura.empty()) {
            std::string http = "HTTP/1.1 400 Bad Request\r\n\r\nFigura vacia";
            client->Write(http.c_str(), http.length());
            client->Close();
            delete client;
            return;
        }
        //parte  = 0;
        try {
            parte = std::stoi(parteStr);
        } catch (...) {
            std::string http = "HTTP/1.1 400 Bad Request\r\n\r\nMitad invalida";
            client->Write(http.c_str(), http.length());
            client->Close();
            delete client;
            return;
        }

        if (parte != 1 && parte != 2) {
            std::string http = "HTTP/1.1 400 Bad Request\r\n\r\nMitad fuera de rango";
            client->Write(http.c_str(), http.length());
            client->Close();
            delete client;
            return;
        }

        rutaServidor = "/figura/" + figura + "/" + std::to_string(parte);
        valid = true;
    }

    if (!valid) {
        std::string http = "HTTP/1.1 404 Not Found\r\n\r\nRuta no valida";
        client->Write(http.c_str(), http.length());
        client->Close();
        delete client;
        return;
    }

    std::cout << "[INTERMEDIARIO] Consultando servidor local: "
              << rutaServidor << std::endl;

    std::string response = consultarServidorLocal(rutaServidor, ipv6);

    std::cout << "[INTERMEDIARIO] Respuesta servidor local:\n"
              << response << std::endl;

    std::string http;

    if (response.find("HTTP/1.1 200 OK") == 0 ||
        response.find("HTTP/1.0 200 OK") == 0) {
        http = response;
    } else if (response.find("HTTP/1.1 404 Not Found") == 0 ||
        response.find("HTTP/1.0 404 Not Found") == 0) {
        std::cout << "[INTERMEDIARIO] No encontrada localmente. Buscando en otras islas...\n";
        std::string piezasRemotas =
            consultarIntermediariosTP(figura, parte, ipv6);
        if (!piezasRemotas.empty()) {
            http =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n" +
                piezasRemotas;

        } else {
            http =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                "Figura no encontrada";
        }
    } else if (response.find("HTTP/1.1 400 Bad Request") == 0 ||
        response.find("HTTP/1.0 400 Bad Request") == 0) {
        http =
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "Solicitud invalida";
    } else if (response.empty()) {
        http =
            "HTTP/1.1 502 Bad Gateway\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "No hubo respuesta del servidor local";
    } else {
        http =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "Respuesta desconocida del servidor local";
    }

    client->Write(http.c_str(), http.length());

    client->Close();
    delete client;
}

std::string Intermediario::consultarServidorLocal(const std::string& ruta, bool ipv6) {
    VSocket* server = new Socket('s', ipv6);

    server->Connect(SERVER_HOST, SERVER_PORT);

    std::string request =
        "GET " + ruta + " HTTP/1.1\r\n"
        "Host: " + std::string(SERVER_HOST) + "\r\n"
        "Connection: close\r\n"
        "\r\n";

    std::cout << "[INTERMEDIARIO -> SERVIDOR]\n" << request << std::endl;

    server->Write(request.c_str(), request.length());

    char buffer[BUFSIZE] = {0};
    int st;
    std::string response;

    while ((st = server->Read(buffer, BUFSIZE - 1)) > 0) {
        buffer[st] = '\0';
        response += buffer;
    }

    server->Close();
    delete server;

    std::cout << "[SERVIDOR -> INTERMEDIARIO]\n" << response << std::endl;

    return response;
}

VSocket*  Intermediario::getFork() {
    return this->intermediario;
}
std::string Intermediario::consultarIntermediariosTP(
    const std::string& figura,
    int mitad,
    bool ipv6) {

    std::string clave = figura;

    if (tablaRutas.find(clave) == tablaRutas.end()) {

        std::cout
            << "[TP] No tengo ruta para "
            << figura
            << ". Refrescando tabla global..."
            << std::endl;

        descubrirOtrosIntermediarios();

        std::this_thread::sleep_for(
            std::chrono::milliseconds(300)
        );

        if (tablaRutas.find(clave) == tablaRutas.end()) {

            std::cout
                << "[TP] Figura no encontrada tras refrescar tabla"
                << std::endl;

            return "";
        }
    }

    PaqueteTP solicitud;

    solicitud.tipo = INTERMEDIARY_REQUEST;
    solicitud.mitad = mitad;
    solicitud.content = figura;
    solicitud.contentLength = figura.length();

    std::string mensaje =
        empaquetarTP(solicitud);

    for (Ruta& ruta : tablaRutas[clave]) {

        if (!ruta.activo) {
            continue;
        }

        VSocket* remoto =
            new Socket('s', ipv6);

        try {

            remoto->Connect(
                ruta.host.c_str(),
                ruta.port.c_str()
            );

            remoto->Write(
                mensaje.c_str(),
                mensaje.length()
            );

            char buffer[BUFSIZE] = {0};

            int st =
                remoto->Read(
                    buffer,
                    BUFSIZE - 1
                );

            if (st > 0) {

                std::string respuestaRaw(
                    buffer,
                    st
                );

                uint8_t tipo =
                    static_cast<uint8_t>(
                        respuestaRaw[0]
                    );

                if (tipo == INTERMEDIARY_RESPONSE) {

                    std::string piezas =
                        extraerPiezasTP(
                            respuestaRaw
                        );

                    remoto->Close();
                    delete remoto;

                    return piezas;
                }

                if (tipo == FIGURE_NOT_FOUND) {

                    ruta.activo = false;

                    std::cout
                        << "[TABLA] Ruta inactiva: "
                        << figura
                        << " -> "
                        << ruta.host
                        << ":"
                        << ruta.port
                        << std::endl;
                }
            }

        } catch (...) {

            ruta.activo = false;

            std::cout
                << "[TABLA] Ruta inactiva por error de conexion: "
                << figura
                << " -> "
                << ruta.host
                << ":"
                << ruta.port
                << std::endl;
        }

        remoto->Close();
        delete remoto;
    }

    return "";
}
std::string Intermediario::empaquetarTP(const PaqueteTP& paquete) {

    std::string data;

    data.push_back(paquete.tipo);

    if (paquete.tipo == INTERMEDIARY_REQUEST) {

        data.push_back(paquete.mitad);

        uint8_t len =
            static_cast<uint8_t>(paquete.content.size());

        data.push_back(len);

        data += paquete.content;
    }
    else if (paquete.tipo == FIGURE_NOT_FOUND) {

        // solo el byte tipo
    }

    return data;
}
PaqueteTP Intermediario::desempaquetarTP(
    const std::string& data) {

    PaqueteTP paquete{};

    if (data.empty()) {

        paquete.tipo = FIGURE_NOT_FOUND;
        return paquete;
    }

    paquete.tipo =
        static_cast<uint8_t>(data[0]);

    if (paquete.tipo == FIGURE_NOT_FOUND) {

        return paquete;
    }

    if (data.size() < 3) {

        paquete.tipo = FIGURE_NOT_FOUND;
        return paquete;
    }

    paquete.mitad =
        static_cast<uint8_t>(data[1]);

    uint8_t len =
        static_cast<uint8_t>(data[2]);

    if (data.size() < 3 + len) {

        paquete.tipo = FIGURE_NOT_FOUND;
        return paquete;
    }

    paquete.content =
        data.substr(3, len);

    paquete.contentLength = len;

    return paquete;
}
void Intermediario::agregarRutaLocal(const std::string& figura, const std::string& host, const std::string& port) {
    std::lock_guard<std::mutex> lock(mutexTabla);
    for (Ruta& ruta : tablaRutas[figura]) {
        if (ruta.host == host && ruta.port == port && ruta.local == true) {
            ruta.activo = true;
            return;
        }
    }

    Ruta ruta;
    ruta.host = host;
    ruta.port = port;
    ruta.activo = true;
    ruta.local = true;

    tablaRutas[figura].push_back(ruta);

    std::cout << "[TABLA] Ruta local agregada: "
              << figura << " -> "
              << host << ":" << port
              << std::endl;
}

void Intermediario::iniciarDescubrimientoIntermediarios() {

    actualizarFigurasDesdeServidorLocal(false);

    std::thread udpListener(
        &Intermediario::escucharIntermediariosTP,
        this
    );
    udpListener.detach();

    std::thread tcpListener(
        &Intermediario::escucharSolicitudesTP,
        this
    );
    tcpListener.detach();

    descubrirOtrosIntermediarios();
}


void Intermediario::escucharIntermediariosTP() {

    VSocket* udp = new Socket('d', false);

    udp->Bind(PORT_JOIN);

    std::cout
        << "[TP UDP] Intermediario escuchando en puerto "
        << PORT_JOIN
        << std::endl;

    while (true) {

        char buffer[BUFSIZE] = {0};

        sockaddr_in sender;
        memset(&sender, 0, sizeof(sender));

        int st =
            udp->recvFrom(
                buffer,
                BUFSIZE,
                &sender
            );

        if (st <= 0)
            continue;

        uint8_t tipo =
            static_cast<uint8_t>(buffer[0]);

        char ip[INET_ADDRSTRLEN];

        inet_ntop(
            AF_INET,
            &sender.sin_addr,
            ip,
            sizeof(ip)
        );

        if (ipOrigen == std::string(SERVER_HOST)) {
            continue;
        }
        if (tipo == INTERMEDIARY_JOIN) {
            std::cout
                << "[TP UDP] JOIN recibido desde "
                << ipOrigen
                << std::endl;

            procesarJoinIntermediario(ipOrigen);

            actualizarFigurasDesdeServidorLocal(false);

            std::string contenido =
            std::string(SERVER_HOST) +
            ";" +
            std::to_string(PORT_HANDSHAKE) +
            ";" +
            obtenerFigurasLocalesComoCSV();

            std::vector<uint8_t> respuesta;

            respuesta.push_back(
                INTERMEDIARY_HANDSHAKE
            );

            uint32_t len =
                static_cast<uint32_t>(
                    contenido.size()
                );

            const uint8_t* lenBytes =
                reinterpret_cast<uint8_t*>(&len);

            respuesta.insert(
                respuesta.end(),
                lenBytes,
                lenBytes + 4
            );

            respuesta.insert(
                respuesta.end(),
                contenido.begin(),
                contenido.end()
            );

            try {

                VSocket* tcp =
                    new Socket('s', false);

                tcp->Connect(
                    ipOrigen.c_str(),
                    "3031"
                );

                tcp->Write(
                    reinterpret_cast<char*>(respuesta.data()),
                    respuesta.size()
                );

                std::cout
                    << "[TP TCP] HANDSHAKE enviado a "
                    << ipOrigen
                    << ":3031"
                    << std::endl;

                tcp->Close();
                delete tcp;

            } catch (...) {

                std::cout
                    << "[TP TCP] Error enviando HANDSHAKE a "
                    << ipOrigen
                    << std::endl;
            }
        }
        
    }
}

void Intermediario::descubrirOtrosIntermediarios() {

    VSocket* udp = new Socket('d', false);

    udp->EnableBroadcast();
    

    uint8_t paquete[5];

    paquete[0] = INTERMEDIARY_JOIN;

    in_addr ip;
    inet_aton(SERVER_HOST, &ip);

    memcpy(
        paquete + 1,
        &ip.s_addr,
        sizeof(ip.s_addr)
    );

    for (const char* broadcast : BROADCASTS) {

        udp->Broadcast(
            reinterpret_cast<char*>(paquete),
            sizeof(paquete),
            broadcast,
            PORT_JOIN
        );
        std::cout
        << "[BROADCAST] enviando a "
        << broadcast
        << ":"
        << PORT_JOIN
        << std::endl;
    }
    
    udp->Close();
    delete udp;
}
void Intermediario::procesarMensajeIntermediario(const std::string& raw,const std::string& ipOrigen) {
    std::cout
    // << "[HANDSHAKE] Procesando mensaje de "
    // << ipOrigen
    // << std::endl;
    if (raw.size() < 5)
        return;

    const uint8_t* data =
        reinterpret_cast<const uint8_t*>(raw.data());

    if (data[0] != INTERMEDIARY_HANDSHAKE)
        return;

    uint32_t len;

    memcpy(
        &len,
        data + 1,
        4
    );

    if (raw.size() < 5 + len)
        return;

    std::string contenido(
        raw.data() + 5,
        len
    );

    size_t pos1 = contenido.find(";");

    if (pos1 == std::string::npos)
        return;

    size_t pos2 = contenido.find(";", pos1 + 1);

    if (pos2 == std::string::npos)
        return;

    std::string ip =
        contenido.substr(0, pos1);

    std::string port =
        contenido.substr(
            pos1 + 1,
            pos2 - pos1 - 1
        );

    std::string figuras =
        contenido.substr(pos2 + 1);

    std::stringstream ss(figuras);

    std::string figura;

    std::cout
    << "[HANDSHAKE] puerto="
    << port
    << " figuras="
    << figuras
    << std::endl;

    while (getline(ss, figura, ',')) {

        if (!figura.empty()) {

            agregarRutaRemota(
                figura,
                ip,
                port
            );
        }
    }
}

void Intermediario::agregarRutaRemota(const std::string& figura,
                                      const std::string& host,
                                      const std::string& port) {
    std::lock_guard<std::mutex> lock(mutexTabla);

    for (Ruta& ruta : tablaRutas[figura]) {
        if (ruta.host == host &&
            ruta.port == port &&
            ruta.local == false) {
            ruta.activo = true;
            return;
        }
    }

    Ruta ruta;
    ruta.host = host;
    ruta.port = port;
    ruta.activo = true;
    ruta.local = false;

    tablaRutas[figura].push_back(ruta);

    std::cout << "[TABLA GLOBAL] Ruta remota agregada: "
              << figura << " -> "
              << host << ":" << port
              << std::endl;
}

std::string Intermediario::obtenerFigurasLocalesComoCSV() {
    std::lock_guard<std::mutex> lock(mutexTabla);

    std::string resultado;
    bool primero = true;

    for (auto& par : tablaRutas) {
        const std::string& figura = par.first;

        for (const Ruta& ruta : par.second) {
            if (ruta.local && ruta.activo) {
                if (!primero) {
                    resultado += ",";
                }

                resultado += figura;
                primero = false;
                break;
            }
        }
    }

    return resultado;
}
void Intermediario::escucharSolicitudesTP() {

    VSocket* listener = new Socket('s', false);

    listener->Bind(PORT_HANDSHAKE);
    listener->MarkPassive(5);

    std::cout
        << "[TP TCP] Escuchando solicitudes en puerto "
        << PORT_HANDSHAKE
        << std::endl;

    while (true) {

        VSocket* peer =
            listener->AcceptConnection();

        std::thread worker(
        [this](VSocket* socket) {

            char buffer[BUFSIZE] = {0};

            int st =
                socket->Read(
                    buffer,
                    BUFSIZE
                );

            if (st > 0) {

                std::string raw(
                    buffer,
                    st
                );
                uint8_t tipo =
                static_cast<uint8_t>(raw[0]);

                if (tipo == INTERMEDIARY_HANDSHAKE) {
                    std::cout
                    << "[TP TCP] HANDSHAKE recibido"
                    << std::endl;
                    procesarMensajeIntermediario(raw, "");
                    socket->Close();
                    delete socket;
                    return;
                }

                PaqueteTP paquete =
                    desempaquetarTP(raw);

                if (paquete.tipo ==
                    INTERMEDIARY_REQUEST) {

                    std::string figura =
                        paquete.content;

                    int mitad =
                        paquete.mitad;

                    std::string ruta =
                        "/figura/" +
                        figura +
                        "/" +
                        std::to_string(mitad);

                    std::string http =
                        consultarServidorLocal(
                            ruta,
                            false
                        );

                    if (http.find("HTTP/1.1 200 OK") == 0 ||
                        http.find("HTTP/1.0 200 OK") == 0) {

                        size_t pos =
                            http.find("\r\n\r\n");

                        std::string piezas =
                            (pos != std::string::npos)
                            ? http.substr(pos + 4)
                            : "";

                        std::vector<uint8_t> respuesta;

                        respuesta.push_back(
                            INTERMEDIARY_RESPONSE
                        );

                        respuesta.push_back(
                            static_cast<uint8_t>(mitad)
                        );

                        respuesta.push_back(
                            static_cast<uint8_t>(
                                figura.size()
                            )
                        );

                        respuesta.insert(
                            respuesta.end(),
                            figura.begin(),
                            figura.end()
                        );

                        uint32_t len =
                            static_cast<uint32_t>(
                                piezas.size()
                            );

                        const uint8_t* lenBytes =
                            reinterpret_cast<uint8_t*>(&len);

                        respuesta.insert(
                            respuesta.end(),
                            lenBytes,
                            lenBytes + 4
                        );

                        respuesta.insert(
                            respuesta.end(),
                            piezas.begin(),
                            piezas.end()
                        );

                        socket->Write(
                            reinterpret_cast<char*>(
                                respuesta.data()
                            ),
                            respuesta.size()
                        );

                    } else {

                        uint8_t notFound =
                            FIGURE_NOT_FOUND;

                        socket->Write(
                            reinterpret_cast<char*>(
                                &notFound
                            ),
                            1
                        );
                    }
                }
            }

            socket->Close();
            delete socket;

        }, peer);

        worker.detach();
    }
}
std::string Intermediario::extraerPiezasTP(const std::string& raw) {

    if (raw.size() < 7)
        return "";

    const uint8_t* data =
        reinterpret_cast<const uint8_t*>(raw.data());

    if (data[0] != INTERMEDIARY_RESPONSE)
        return "";

    uint8_t figuraLen = data[2];

    size_t pos =
        3 + figuraLen;

    if (raw.size() < pos + 4)
        return "";

    uint32_t piezasLen;

    memcpy(
        &piezasLen,
        data + pos,
        4
    );

    pos += 4;

    if (raw.size() < pos + piezasLen)
        return "";

    return raw.substr(
        pos,
        piezasLen
    );
}
void Intermediario::procesarJoinIntermediario(const std::string& ipOrigen) {

    actualizarFigurasDesdeServidorLocal(false);

    std::cout
        << "[JOIN] "
        << ipOrigen
        << std::endl;
}

void Intermediario::actualizarFigurasDesdeServidorLocal(bool ipv6) {
    std::string response = consultarServidorLocal("/figuras", ipv6);

    if (response.find("HTTP/1.1 200 OK") != 0 &&
        response.find("HTTP/1.0 200 OK") != 0) {
        return;
    }

    size_t pos = response.find("\r\n\r\n");

    if (pos == std::string::npos) {
        return;
    }

    std::string cuerpo = response.substr(pos + 4);

    std::stringstream ss(cuerpo);
    std::string figura;

    while (std::getline(ss, figura)) {
        if (!figura.empty()) {
            agregarRutaLocal(figura, SERVER_HOST, SERVER_PORT);
        }
    }
}

int main(int argc, char* argv[]) {
    VSocket* intermediario;
    std::thread* worker;
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <host> <ipv6 0|1>\n";
        return 1;
    }

    char* SERVER_HOST = argv[1];
    const char* SERVER_PORT = "1235";
    bool ipv6 = std::stoi(argv[2]);
    intermediario = new Socket('s', ipv6);
    Intermediario fork(intermediario, SERVER_HOST, SERVER_PORT);
    fork.iniciarDescubrimientoIntermediarios();
    fork.getFork()->Bind(PORT);
    fork.getFork()->MarkPassive(5);

    std::cout << "Intermediario escuchando en puerto " << PORT << "...\n";

    while (true) {
        VSocket* client = fork.getFork()->AcceptConnection();
        worker = new std::thread(&Intermediario::task, &fork, client, ipv6);
        worker->detach();
    }

    return 0;
}