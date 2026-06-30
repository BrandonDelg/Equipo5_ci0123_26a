// Protocolo grupal entre intermediarios
#ifndef PROTO_SS_HPP
#define PROTO_SS_HPP

#include <netinet/in.h>
#include <cstdint>
#include <string>
#include <vector>

class ProtoSS {
 public:  
  enum MsgType : uint8_t {
    INTERMEDIARY_JOIN = 0,
    INTERMEDIARY_HANDSHAKE = 1,
    INTERMEDIARY_REQUEST = 2,
    INTERMEDIARY_RESPONSE = 3,
    FIGURE_NOT_FOUND = 4
  };

  // puerto 3030
  struct Join {
    uint8_t tipo = INTERMEDIARY_JOIN;
    in_addr sourceIp;

    // constructor para deserializar 
    Join() : tipo(INTERMEDIARY_JOIN), sourceIp{} {}

    // constructor para serialize
    Join(in_addr ip) : tipo(INTERMEDIARY_JOIN), sourceIp(ip) {}

    std::vector<uint8_t> serialize() {
      std::vector<uint8_t> buffer;
      buffer.push_back(tipo);
      const uint8_t* ipBytes =
          reinterpret_cast<const uint8_t*>(&sourceIp.s_addr);
      buffer.insert(buffer.end(), ipBytes, ipBytes + sizeof(sourceIp.s_addr));
      return buffer;
    }
    // 
    static int deserialize(const uint8_t* buffer, size_t size,
                           Join& outMsg) {  // el ip lo saca en bufferin
                                            // se podria cambiar esto
      int retorno = 0;
      if (size >= 1 + sizeof(in_addr)) {
        outMsg.tipo = buffer[0];
        if (outMsg.tipo == INTERMEDIARY_JOIN) {
          std::copy(buffer + 1, buffer + 1 + sizeof(in_addr) ,
                    reinterpret_cast<uint8_t*>(&outMsg.sourceIp.s_addr));// asumo que in addr tiene 156
          ++retorno;
        }
      }
      return retorno;
    }
  };

  // TCP 3031 Jafet Sapo
  // cuando se recibe un handshake, lo normal seria reenviarlo
  // al origen del handshake, aunque tambien podria simplemente agarrar 
  // la ip del que hizo handshake y enviarle pero esa cosa se encicla
  struct Handshake {  
    uint8_t tipo = INTERMEDIARY_HANDSHAKE;
    uint32_t contentLength = 0;
    std::string content;

    Handshake() : tipo(INTERMEDIARY_HANDSHAKE), contentLength(0), content("") {}
    Handshake(const std::string& cont) 
        : tipo(INTERMEDIARY_HANDSHAKE), 
          contentLength(static_cast<uint32_t>(cont.size())), 
          content(cont) {}

    std::vector<uint8_t> serialize() {
      std::vector<uint8_t> buffer;
      buffer.push_back(tipo);
      uint32_t len = static_cast<uint32_t>(content.size());
      // ocupo que no paddee
      const uint8_t* lenBytes = reinterpret_cast<const uint8_t*>(&len);
      // los bytes de lensito
      buffer.insert(buffer.end(), lenBytes, lenBytes + 4);
      buffer.insert(buffer.end(), content.begin(), content.end());
      return buffer;
    }
  };

  //  INTERMEDIARY_REQUEST (TCP/3031)
  struct Request {
    uint8_t tipo = INTERMEDIARY_REQUEST;
    uint8_t mitad = 1;  // 1: primera, 2: segunda, 3: todas
    uint8_t contentLength = 0;
    // por el momento nombre figura no sirve de nada,
    //  si no me da pereza lo quito
    std::string content;  // Nombre de la figura solicitada
    //constructores bonitos
    Request() : tipo(INTERMEDIARY_REQUEST), mitad(1), contentLength(0), content("") {}
    Request(uint8_t mit, const std::string& cont) 
        : tipo(INTERMEDIARY_REQUEST), 
          mitad(mit), 
          contentLength(static_cast<uint8_t>(cont.size())), 
          content(cont) {}

    std::vector<uint8_t> serialize() const {
      std::vector<uint8_t> buffer;
      buffer.push_back(tipo);
      buffer.push_back(mitad);
      buffer.push_back(static_cast<uint8_t>(content.size()));
      buffer.insert(buffer.end(), content.begin(), content.end());
      return buffer;
    }
  };

  //INTERMEDIARY_RESPONSE (TCP/3031)
  struct Response {
    uint8_t tipo = INTERMEDIARY_RESPONSE;
    uint8_t mid = 1;  // 1: primera, 2: segunda, 3: todas
    uint8_t figureNameLength = 0;
    std::string figureName;
    uint32_t contentLength = 0;
    // Estructura: [cant,nombre][cant,nombre]
    std::string content; 

    Response() 
        : tipo(INTERMEDIARY_RESPONSE), mid(1), figureNameLength(0), 
          figureName(""), contentLength(0), content("") {}
          
    Response(uint8_t m, const std::string& name, const std::string& cont) 
        : tipo(INTERMEDIARY_RESPONSE), 
          mid(m), 
          figureNameLength(static_cast<uint8_t>(name.size())), 
          figureName(name), 
          contentLength(static_cast<uint32_t>(cont.size())), 
          content(cont) {}
          
    std::vector<uint8_t> serialize() {
      std::vector<uint8_t> buffer;
      buffer.reserve(1 + 1 + 1 + figureName.size() + 4 + content.size());
      buffer.push_back(tipo);
      buffer.push_back(mid);
      buffer.push_back(static_cast<uint8_t>(figureName.size()));
      buffer.insert(buffer.end(), figureName.begin(), figureName.end());
      uint32_t len = static_cast<uint32_t>(content.size());
      const uint8_t* lenBytes = reinterpret_cast<const uint8_t*>(&len);
      buffer.insert(buffer.end(), lenBytes, lenBytes + 4);
      buffer.insert(buffer.end(), content.begin(), content.end());

      return buffer;
    }
  };
};

#endif  // PROTOSS