/**
 * @file Logger.cpp
 * @brief Implementación de clase Logger
 */
#include <iostream>
#include <fstream>
#include "Logger.hpp"
#include <ctime>
#include <string>

//"./logs/log.txt"

Logger::Logger(std::string dir) {
  this->logs = std::ofstream(dir.c_str(), std::ios::app);
}

void Logger::loadTime() {
  this->tiempo = std::time(nullptr);
  this->tm = std::localtime(&tiempo);
  std::strftime(this->fecha, sizeof(this->fecha), "%Y-%m-%d %H:%M:%S", this->tm);

}


void Logger::log(std::string txt, Nodo t) {

  // Fecha
  this->loadTime();

  // NODO
  std::string nodo;
  if (t == 0) {
    nodo = "Cliente";
  } else if (t == 1) {
    nodo = "Tenedor";
  } else if (t == 2){
    nodo = "Server";
  } else {
    nodo = "Usuario";
  }

  std::string t_print;

  for (char c : txt) {
    if (c == '\n') {
      t_print += "\\n";
      t_print += " [>] ";
    } else if (c == '\r'){
      t_print += "\\r";
      t_print += " [>] ";
    } else {
      t_print += c;
    }


  }

  this->logs << "[" << this->fecha << "] " << "[" << nodo << "]: ";
  this->logs << "\n\t[>]\t" << t_print.c_str() << std::endl;

  return;
}

void Logger::logv(std::vector<std::string>& txt, Nodo t = Cliente) {
  
  // Fecha
  this->loadTime();

  // NODO
  std::string nodo;
  if (t == 0) {
    nodo = "Cliente";
  } else if (t == 1) {
    nodo = "Tenedor";
  } else if (t == 2){
    nodo = "Server";
  } else {
    nodo = "Usuario";
  }

  std::string t_print;

  for (std::string s : txt) {
    t_print += "\n\t[>]\t";
    t_print += s;
  }

  this->logs << "[" << this->fecha << "] " << "[" << nodo << "]: ";
  this->logs << t_print.c_str() << std::endl;

  return;
}



Logger::~Logger(){
  this->logs.close();
}
