/**
 * @file Parser.hpp
 * @brief Definición de clase parser
 */
#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <string>

/**
 * @brief Clase Parser, procesa html del servidor
 *
 * Encargado de procesar el html que devuelve el servidor 
 * para extraer la lista de figuras y las piezas del html
 * 
 */
class Parser {
public:

    /**
     * @brief Constructor del parser
     */
    Parser();

    /**
     * @brief Destructor del parser
     */
    ~Parser();

    /**
     * @brief Extrae la lista de figuras
     *
     * @param msg String que contiene el mensaje completo
     */
    int getTipo(const std::string &msg);

    /**
     * @brief Extrae las piezas
     *
     * @param msg String que contiene el mensaje completo
     */
    std::string getFigura(const std::string &msg);

    /**
     * @brief Extrae las piezas
     *
     * @param msg String que contiene el mensaje completo
     */
    int getMitad(const std::string &msg);

    /**
     * @brief Devuelve las figuras
     *
     * @return Vector de figuras
     */
    std::vector<std::string> getFiguras() const;
    /**
     * @brief Devuelve las figuras
     *
     * @return Par de pieza y cantidad
     */
    std::vector<std::pair<std::string, int>> getPiezas() const;
    std::string getMensaje(const std::string &msg);


private:
    std::vector<std::string> Figuras; /** Lista de figuras */
    std::vector<std::pair<std::string, int>> piezas; /** Par pieza y cantidad */
};

#endif //PARSER_HPP
