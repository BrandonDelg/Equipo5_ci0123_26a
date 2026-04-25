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
     * @param html Documento html en formato string
     */
    void procesarFiguras(const std::string &html);

    /**
     * @brief Extrae las piezas
     *
     * @param html String html de la pieza
     */
    void procesarPiezas(const std::string &html);

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


private:
    std::vector<std::string> Figuras; /** Lista de figuras */
    std::vector<std::pair<std::string, int>> piezas; /** Par pieza y cantidad */
};

#endif //PARSER_HPP
