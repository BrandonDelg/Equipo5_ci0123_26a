/**
 * @file Parser.cpp
 * @brief Implementación de clase Parser
 */
#include "Parser.hpp"
#include <regex>
#include <iostream>

Parser::Parser() {}

Parser::~Parser() {}

int Parser::getTipo(const std::string &msg) {
    std::smatch match;
    std::regex re_type(R"(^([^|]+)\|)");

    if (std::regex_search(msg, match, re_type)) {
        return std::stoi(match[1]);
    }

    return 0;
}

std::string Parser::getFigura(const std::string &msg) {
    std::smatch match;
    std::string figura = "figura";
    std::regex re_valor("\\b" + figura + R"(=([^;]+))");

    if (std::regex_search(msg, match, re_valor)) {
        return match[1];
    }

    return 0;
}

int Parser::getMitad(const std::string &msg) {
    std::smatch match;
    std::string mitad = "mitad";
    std::regex re_valor("\\b" + mitad + R"(=([^;]+))");

    if (std::regex_search(msg, match, re_valor)) {
        return std::stoi(match[1]);
    }
    return 0;
}

std::string Parser::getMensaje(const std::string &msg) {
    std::smatch match;
    std::string mensaje = "mensaje";
    std::regex re_valor("\\b" + mensaje + R"(=([^;]+))");

    if (std::regex_search(msg, match, re_valor)) {
        return match[1];
    }

    return 0;
}
