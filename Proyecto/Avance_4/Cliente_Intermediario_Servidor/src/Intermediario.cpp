#include "Intermediario.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <sstream>
#include "FileSystem.hpp"

// Intermediario::Intermediario(VSocket* fork, char* host, const char* port)
//     : SERVER_HOST(host), SERVER_PORT(port), intermediario(fork) {}

Intermediario::Intermediario(VSocket* fork)
    : SERVER_HOST(nullptr), SERVER_PORT(nullptr), intermediario(fork) {
      SERVER_PORT = strdup("1235");
    }


Intermediario::~Intermediario() {
  if (SERVER_HOST != nullptr) {
    free(SERVER_HOST);
  }

  if (SERVER_PORT != nullptr) {
    free(SERVER_PORT);
  }
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
    std::string figuras = obtenerFigurasGlobales();
    std::string http =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n" +
        figuras;

    client->Write(http.c_str(), http.length());
    client->Close();
    delete client;
    return;

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
    // parte  = 0;
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
  std::cout << "[INTERMEDIARIO] Consultando servidor local: " << rutaServidor
            << std::endl;
  std::string response = consultarServidorLocal(rutaServidor, ipv6);
  std::cout << "[INTERMEDIARIO] Respuesta servidor local:\n"<< response << std::endl;
  std::string http;
  if (response.find("HTTP/1.1 200 OK") == 0 ||
      response.find("HTTP/1.0 200 OK") == 0) {
    http = response;
  } else if (response.find("HTTP/1.1 404 Not Found") == 0 ||
             response.find("HTTP/1.0 404 Not Found") == 0) {
    std::cout << "[INTERMEDIARIO] No encontrada localmente. Buscando en otras "
                 "islas...\n";
    std::string piezasRemotas = consultarIntermediariosTP(figura, parte, ipv6);
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

//TODO(brandon) Tener todas las rutas de mis servidores de figuras locales
std::string Intermediario::consultarServidorLocal(const std::string& ruta,
                                                  bool ipv6) {
  while (SERVER_HOST == nullptr) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  VSocket* server = new Socket('s', ipv6);
  while (true) {
    try {
      server->Connect(SERVER_HOST, SERVER_PORT);
      break;
    } catch (...) {
      std::cerr << "[INTERMEDIARIO] Servidor no disponible, esperando...\n";
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  std::string request = "GET " + ruta +
                        " HTTP/1.1\r\n"
                        "Host: " +
                        std::string(SERVER_HOST) +
                        "\r\n"
                        "Connection: close\r\n"
                        "\r\n";
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
  return response;
}
VSocket* Intermediario::getFork() { return this->intermediario; }
std::string Intermediario::consultarIntermediariosTP(const std::string& figura,
                                                     int mitad, bool ipv6) {
  std::string clave = figura;
  std::lock_guard<std::mutex> lock(mutexTabla);
  if (tablaRutas.find(clave) == tablaRutas.end()) {
    std::cout << "[TP] No tengo ruta para " << figura
              << ". Refrescando tabla global..." << std::endl;
    descubrirOtrosIntermediarios();
    // es mejor liberar el mutex y  despues agarrarlo
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    if (tablaRutas.find(clave) == tablaRutas.end()) {
      std::cout << "[TP] Figura no encontrada tras refrescar tabla"
                << std::endl;
      return "";
    }
  }
  ProtoSS::Request solicitud(mitad,figura);
  std::vector<uint8_t> mensaje  = solicitud.serialize();
  for (Ruta& ruta : tablaRutas[clave]) {
    if (!ruta.activo) {
      continue;
    }
    VSocket* remoto = new Socket('s');
    try {
      remoto->Connect(ruta.host.c_str(), ruta.port.c_str());
      remoto->Write(mensaje.data(), mensaje.size());
      uint8_t tipo = 0;
      int st = remoto->Read(&tipo,sizeof(tipo));
      if (st > 0) {
        //std::string respuestaRaw(buffer, st);
        if (tipo == ProtoSS::INTERMEDIARY_RESPONSE) {
          std::string piezas = extraerPiezasTP(*remoto);
          remoto->Close();
          delete remoto;
          return piezas;
          // figura 1
          // mitad 1
          // mitad 2
        }
        if (tipo == ProtoSS::FIGURE_NOT_FOUND) {
          ruta.activo = false;
          std::cout << "[TABLA] Ruta inactiva: " << figura << " -> "
                    << ruta.host << ":" << ruta.port << std::endl;
          limpiarTablaRutas();
        }
      }
    } catch (...) {
      ruta.activo = false;
      std::cout << "[TABLA] Ruta inactiva por error de conexion: " << figura
                << " -> " << ruta.host << ":" << ruta.port << std::endl;
      limpiarTablaRutas();
    }
    remoto->Close();
    delete remoto;
  }
  return "";
}

void Intermediario::agregarRutaLocal(const std::string& figura,
                                     const std::string& host,
                                     const std::string& port) {
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
  std::cout << "[TABLA] Ruta local agregada: " << figura << " -> " << host
            << ":" << port << std::endl;
}
void Intermediario::iniciarDescubrimientoIntermediarios() {
  //actualizarFigurasDesdeServidorLocal(false);
  std::thread udpListener(&Intermediario::escucharIntermediariosTP, this);
  udpListener.detach();
  std::thread tcpListener(&Intermediario::escucharSolicitudesTP, this);
  tcpListener.detach();
  descubrirOtrosIntermediarios();
}
// puerto 3030 Join UDP
void Intermediario::escucharIntermediariosTP() {
  VSocket* udp = new Socket('d', false);
  udp->Bind(PORT_JOIN);
  std::cout << "[TP UDP] Intermediario escuchando en puerto " << PORT_JOIN
            << std::endl;
  while (true) {
    char buffer[BUFSIZE] = {0};
    sockaddr_in sender;
    memset(&sender, 0, sizeof(sender));
    int st = udp->recvFrom(buffer, BUFSIZE, &sender);
    if (st <= 0) continue;
    uint8_t tipo = static_cast<uint8_t>(buffer[0]);
    char ip[INET_ADDRSTRLEN];

    if (tipo == ProtoSS::INTERMEDIARY_JOIN) {
        inet_ntop(AF_INET, &sender.sin_addr, ip, sizeof(ip));
        std::string ipOrigen(ip);
        if (SERVER_HOST != nullptr &&
          ipOrigen == SERVER_HOST) {
          continue;
        }
      std::cout << "[TP UDP] JOIN recibido desde " << ipOrigen << std::endl;
      actualizarFigurasDesdeServidorLocal(false);
      ProtoSS::Handshake handshake(obtenerFigurasLocalesComoCSV());
      std::vector<uint8_t> respuesta = handshake.serialize();
      try {
        VSocket* tcp = new Socket('s', false);
        tcp->Connect(ipOrigen.c_str(), "3031");
        tcp->Write((respuesta.data()), respuesta.size());
        std::cout << "[TP TCP] HANDSHAKE enviado a " << ipOrigen << ":3031"
                  << std::endl;
          uint8_t tipo = 0;
          uint32_t contentLength = 0;
          tcp->Read(&tipo, sizeof(tipo));
          if(tipo == ProtoSS::INTERMEDIARY_HANDSHAKE){
            if (tcp->Read(&contentLength, sizeof(contentLength)) > 0) {
              std::string content(contentLength, '\0');
              if (contentLength > 0) {
                tcp->Read(&content[0], contentLength);
                std::stringstream ss(content);
                std::string figura;
                std::lock_guard<std::mutex> lock(mutexTabla);
                while (std::getline(ss, figura, ',')) {
                  Ruta nuevaRuta;
                  nuevaRuta.host = ipOrigen;
                  nuevaRuta.port =
                      std::string("3031"); 
                  nuevaRuta.activo = true;
                  nuevaRuta.local = false;
                  tablaRutas[figura].push_back(nuevaRuta);
                  std::cout << "   [Descubierta] " << figura << " - " << ipOrigen
                            << ":" << 3031 << std::endl;
                }

              }
            }
          }
        tcp->Close();
        delete tcp;
      } catch (...) {
        std::cout << "[TP TCP] Error enviando HANDSHAKE a " << ipOrigen
                  << std::endl;
      }
    }
  }
}
void Intermediario::descubrirOtrosIntermediarios() {
  VSocket* udp = new Socket('d', false);
  udp->EnableBroadcast();
  in_addr sourceIp {};
  ProtoSS::Join join(sourceIp);
  std::vector<uint8_t> soli = join.serialize();
  for (const char* broadcast : BROADCASTS) {
    udp->Broadcast(soli.data(), soli.size(), broadcast, PORT_JOIN);
    std::cout << "[BROADCAST] enviando a " << broadcast << ":" << PORT_JOIN
              << std::endl;
  }
  udp->Close();
  delete udp;
}

void Intermediario::procesarMensajeIntermediario(VSocket* client) {
  uint32_t incomingLength = 0;
  if (client->Read(&incomingLength, sizeof(incomingLength)) <= 0) return;
  std::string figuras(incomingLength, '\0');
  if (incomingLength > 0) {
    client->Read(&figuras[0], incomingLength);
     std::string ipRemota = client->getRemoteIPV4();
    std::cout << "la ip remota es" << ipRemota << std::endl;
    std::stringstream ss(figuras);
    std::string figura;
    std::string puerto = "3031";
    std::cout << "[HANDSHAKE] puerto=" << 3031 << " figuras=" << figuras
              << std::endl;
    while (getline(ss, figura, ',')) {
      if (!figura.empty()) {
        agregarRutaRemota(figura, ipRemota,puerto );
      }      
    }
    ProtoSS::Handshake miHandshake(obtenerFigurasLocalesComoCSV());
      std::vector<uint8_t> responseBytes = miHandshake.serialize();
      client->Write(responseBytes.data(), responseBytes.size());  // todo fino

  }
}
void Intermediario::agregarRutaRemota(const std::string& figura,
                                      const std::string& host,
                                      const std::string& port) {
  std::lock_guard<std::mutex> lock(mutexTabla);
  for (Ruta& ruta : tablaRutas[figura]) {
    if (ruta.host == host && ruta.port == port && ruta.local == false) {
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
  std::cout << "[TABLA GLOBAL] Ruta remota agregada: " << figura << " -> "
            << host << ":" << port << std::endl;
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
  std::cout << "[TP TCP] Escuchando solicitudes en puerto " << PORT_HANDSHAKE
            << std::endl;
  while (true) {
    VSocket* peer = listener->AcceptConnection();
    std::thread worker(
        [this](VSocket* socket) {
          uint8_t tipo = 0;
          int bytes = socket->Read(&tipo, 1);
          if (tipo == ProtoSS::INTERMEDIARY_HANDSHAKE) {
            std::cout << "[TP TCP] HANDSHAKE recibido" << std::endl;
            procesarMensajeIntermediario(socket);
            socket->Close();
            //delete socket;
          } else {
            if (tipo == ProtoSS::INTERMEDIARY_REQUEST) {
              uint8_t mitad = 0;
              uint8_t nameLength = 0;
              if (socket->Read(&mitad, 1) > 0 &&
                  socket->Read(&nameLength, 1) > 0) {
                std::string figura(nameLength, '\0');
                if (nameLength > 0) {
                  socket->Read(&figura[0], nameLength);
                  std::string ruta =
                      "/figura/" + figura + "/" + std::to_string(mitad);
                  std::string http = consultarServidorLocal(ruta, false);
                  if (http.find("HTTP/1.1 200 OK") == 0 ||
                      http.find("HTTP/1.0 200 OK") == 0) {
                    size_t pos = http.find("\r\n\r\n");
                     std::string piezas =
                        (pos != std::string::npos) ? http.substr(pos + 4) : "";
                    std::vector<uint8_t> respuesta;
                    respuesta.push_back(ProtoSS::INTERMEDIARY_RESPONSE);
                    respuesta.push_back(static_cast<uint8_t>(mitad));
                    respuesta.push_back(static_cast<uint8_t>(figura.size()));
                    respuesta.insert(respuesta.end(), figura.begin(),
                                     figura.end());
                    uint32_t len = static_cast<uint32_t>(piezas.size());
                    const uint8_t* lenBytes = reinterpret_cast<uint8_t*>(&len);
                    respuesta.insert(respuesta.end(), lenBytes, lenBytes + 4);
                    respuesta.insert(respuesta.end(), piezas.begin(),
                                     piezas.end());
                    socket->Write((respuesta.data()),
                                  respuesta.size());
                  } else {
                    uint8_t notFound = ProtoSS::FIGURE_NOT_FOUND;
                    socket->Write((&notFound), 1);
                  }
                }
              }
            }
          }
          socket->Close();
          delete socket;
        },
        peer);
    worker.detach();
  }
}

//TODO (brandon) meter en try catch esta parte
 std::string Intermediario::extraerPiezasTP(VSocket & socket) {
  ProtoSS::Response response;
  uint8_t buffer = 0;
  uint8_t mitad = 0;
  uint32_t content_length;
//   socket.Read(&buffer, 1);

    socket.Read(&mitad, 1);
    socket.Read(&buffer, 1);
    char name_figura[buffer];
    socket.Read(name_figura, buffer);
    socket.Read(&content_length, sizeof(content_length));
    std::string partes(content_length, ' ');

    socket.Read(
        &partes[0],
        content_length);
    return partes;

 }

void Intermediario::actualizarFigurasDesdeServidorLocal(bool ipv6) {
    {
      std::lock_guard<std::mutex> lock(mutexTabla);

      for (auto it = tablaRutas.begin(); it != tablaRutas.end();) {

          it->second.remove_if([](const Ruta& ruta) {
              return ruta.local;
          });

          if (it->second.empty()) {
              it = tablaRutas.erase(it);
          } else {
              ++it;
          }
      }
    }

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

    limpiarTablaRutas();
}

std::string Intermediario::obtenerFigurasGlobales() {
  actualizarFigurasDesdeServidorLocal(false);
  std::lock_guard<std::mutex> lock(mutexTabla);
  int id = 0;
  std::string resultado;

  for (const auto& par : tablaRutas) {
    id += 1;
    resultado += std::to_string(id) + "| " + par.first + "\n";
  }

  return resultado;
}

void Intermediario::escucharServidorLocal() {
    VSocket* udp = new Socket('d', false);

    udp->Bind(9092);

    while (true) {
        char buffer[512] = {0};

        sockaddr_in sender;
        memset(&sender,0,sizeof(sender));

        int st = udp->recvFrom(
            buffer,
            sizeof(buffer)-1,
            &sender
        );

        if (st <= 0)
            continue;

        buffer[st]=0;

        std::string mensaje = buffer;

        if (mensaje.find("SERVER_JOIN") == 0) {

          char ip[INET_ADDRSTRLEN];
          inet_ntop(AF_INET, &sender.sin_addr, ip, sizeof(ip));

          if (SERVER_HOST != nullptr && strcmp(SERVER_HOST, ip) == 0) {
              return;
          }

          if (SERVER_HOST != nullptr) {
              free(SERVER_HOST);
          }

          SERVER_HOST = strdup(ip);
          SERVER_PORT = strdup("1235");

          std::cout << "[DISCOVERY] servidor local encontrado: "
                    << SERVER_HOST << std::endl;

          actualizarFigurasDesdeServidorLocal(false);
      }
    }
}

void Intermediario::limpiarTablaRutas() {
    std::lock_guard<std::mutex> lock(mutexTabla);

    for (auto itFigura = tablaRutas.begin();
         itFigura != tablaRutas.end();) {

        itFigura->second.remove_if([](const Ruta& ruta) {
            return !ruta.activo;
        });

        if (itFigura->second.empty()) {
            itFigura = tablaRutas.erase(itFigura);
        } else {
            ++itFigura;
        }
    }
}
void Intermediario::eliminarRutasLocales(const std::string& figura) {
    std::lock_guard<std::mutex> lock(mutexTabla);

    auto it = tablaRutas.find(figura);

    if (it == tablaRutas.end()) {
        return;
    }

    it->second.remove_if([](const Ruta& ruta) {
        return ruta.local;
    });

    if (it->second.empty()) {
        tablaRutas.erase(it);
    }
}
int main(int argc, char* argv[]) {
  VSocket* intermediario;
  if (argc < 2) {
    std::cerr << "Uso: " << argv[0] << " <host> <ipv6 0|1>\n";
    return 1;
  }
  //char* SERVER_HOST = argv[1];
  //const char* SERVER_PORT = "1235";
  bool ipv6 = std::stoi(argv[1]);
  intermediario = new Socket('s', ipv6);
  // Intermediario fork(intermediario, SERVER_HOST, SERVER_PORT);
  Intermediario fork(intermediario);
  fork.iniciarDescubrimientoIntermediarios();
  std::thread serverDiscovery(
    &Intermediario::escucharServidorLocal,
    &fork
  );
    serverDiscovery.detach();
  fork.getFork()->Bind(PORT);
  fork.getFork()->MarkPassive(5);
  std::cout << "Intermediario escuchando en puerto " << PORT << "...\n";
  while (true) {
    VSocket* client = fork.getFork()->AcceptConnection();
    std::thread(&Intermediario::task, &fork, client, ipv6).detach();
  }
  return 0;
}