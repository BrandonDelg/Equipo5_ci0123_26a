/**
 * @file Logger.hpp
 * @brief Definición de la clase Logger
 */
#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <fstream>
#include <string>
#include <ctime>
#include <vector>

/**
 * @brief Tipo de nodo
 */
enum Nodo {
Cliente,
Tenedor,
Server,
Usuario
}; 


/**
 * @brief
 */
enum EVENT {
  WARN,
  ERROR
};




/**
 * @brief Clase que administra la bitacora de eventos del sistema cliente - intermediario - servidor.
 */
class Logger
{
private:
  std::ofstream logs; // Archivo donde se registra la bitacora. 
  std::time_t tiempo; // Registro de tiempo para la bitacora. 
  std::tm* tm; // Auxiliar del registro de tiempo.
  char fecha[80]; // Buffer donde se guarda la fecha en formato string.

public:
  /**
   * @brief Constructor de la clase Logger
   * @param dir Directorio del archivo donde se registrara la bitacora.
   */
  Logger(std::string dir);

  /**
   * @brief Destructor por defecto de la clase Logger.
   */
  ~Logger();

  /**
   * @brief Metodo que registra un evento en la bitacora.
   * @param txt String con el mensaje del evento.
   * @param t Enum con el nodo que genera el registro.
   */
  void log(std::string txt, Nodo t = Cliente);

  /**
   * @brief Metodo que registra un evento (en formato vector<string>) en la bitacora.
   * @param txt Vector de strings con el mensaje del evento.
   * @param t Enum con el nodo que genera el registro.
   */
  void logv(std::vector<std::string>& txt, Nodo t);
private:
  /**
   * @brief Metodo que actualiza la fecha y hora de un evento,
   */
  void loadTime();
};




#endif
