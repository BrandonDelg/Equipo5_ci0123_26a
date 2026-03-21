#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <string>

class Parser {
public:
    Parser();
    ~Parser();
    void procesarFiguras(const std::string &html);
    void procesarPiezas(const std::string &pieza);
    std::vector<std::string> getFiguras() const;
    std::vector<std::pair<std::string, int>> getPiezas() const;


private:
    std::vector<std::string> Figuras;
    std::vector<std::pair<std::string, int>> piezas;
};

#endif //PARSER_HPP