#include <iostream>
#include <fstream>
#include "Logger.hpp"
#include <ctime>
#include <string>

//"./logs/log.txt"

Logger::Logger(std::string dir) {
  this->logs = std::ofstream(dir.c_str(), std::ios::app);
}

void Logger::log(std::string txt, Nodo t) {

  // Fecha
  this->tiempo = std::time(nullptr);
  this->tm = std::localtime(&tiempo);
  std::strftime(this->fecha, sizeof(this->fecha), "%Y-%m-%d %H:%M:%S", this->tm);

  // NODO
  std::string nodo;
  if (t == 0) {
    nodo = "Cliente";
  } else if (t == 1) {
    nodo = "Tenedor";
  } else {
    nodo = "Server";
  }

  this->logs << "[" << this->fecha << "] " << "[" << nodo << "]: ";
  this->logs << txt.c_str() << std::endl;

  return;
}


Logger::~Logger(){
  this->logs.close();
}