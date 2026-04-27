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
        std::string val = match[1].str();
        if (!val.empty()) {
            return std::stoi(val);
        }
    }

    return 0;
}

std::string Parser::getCampo(const std::string &msg, const std::string &campo) {
    std::smatch match;
    std::regex re(campo + R"(=([^;]+))");

    if (std::regex_search(msg, match, re)) {
        return match[1].str();
    }
    return "";
}

std::string Parser::getFigura(const std::string &msg) {
    return getCampo(msg, "figura");
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
    return getCampo(msg, "mensaje");
}

std::string Parser::getId(const std::string &msg) {
    return getCampo(msg, "id");
}

std::string Parser::getIdInt(const std::string &msg) {
    return getCampo(msg, "idInt");
}

std::string Parser::getEstado(const std::string &msg) {
    return getCampo(msg,"estado");
}

std::string Parser::getTipoRuta(const std::string &msg) {
    return getCampo(msg,"tipo_ruta");
}
