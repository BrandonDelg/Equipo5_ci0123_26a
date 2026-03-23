#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <fstream>
#include <string>
#include <ctime>
#include <vector>

enum Nodo {
Cliente,
Tenedor,
Server,
Usuario
}; 

class Logger
{
private:
  std::ofstream logs;
  std::time_t tiempo;
  std::tm* tm;
  char fecha[80];

public:
  Logger(std::string dir);
  ~Logger();
  void log(std::string txt, Nodo t = Cliente);
  void logv(std::vector<std::string>& txt, Nodo t);
private:
  void loadTime();
};




#endif