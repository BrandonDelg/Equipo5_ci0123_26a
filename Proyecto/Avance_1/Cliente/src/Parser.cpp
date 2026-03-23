#include "Parser.hpp"
#include <regex>
#include <iostream>

Parser::Parser() {}

Parser::~Parser() {}

void Parser::procesarFiguras(const std::string &html) {
    Figuras.clear();
    std::regex re(R"regex(<option\s+value="([^"]+)")regex", std::regex_constants::icase);
    auto begin = std::sregex_iterator(html.begin(), html.end(), re);
    auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        std::string figura = (*it)[1].str();
        if (figura != "None" && figura != "0" && figura != "1" && figura != "2") {
            Figuras.push_back(figura);
        }
    }
}
void Parser::procesarPiezas(const std::string &html) {
    piezas.clear();
    std::regex re(R"REGEX(<td[^>]*>\s*(\d+)\s*</td>\s*<td[^>]*>\s*([^<]+)\s*</td>)REGEX",std::regex_constants::icase);
    // Por si ocuparamos la URL de la imagen de la pieza
    std::regex img_re(R"REGEX(src="([^"]+)")REGEX",std::regex_constants::icase);

    auto begin = std::sregex_iterator(html.begin(), html.end(), re);
    auto end = std::sregex_iterator();

    for (auto it = begin; it != end; ++it) {

        int cantidad = std::stoi((*it)[1].str());
        std::string descripcion = (*it)[2].str();

        descripcion.erase(0, descripcion.find_first_not_of(" \t\n\r"));
        descripcion.erase(descripcion.find_last_not_of(" \t\n\r") + 1);

        piezas.push_back({descripcion, cantidad});
    }
}
std::vector<std::string> Parser::getFiguras() const {
    return Figuras;
}

std::vector<std::pair<std::string, int>> Parser::getPiezas() const {
    return piezas;
}