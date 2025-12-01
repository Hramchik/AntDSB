//
// Created by sanek on 30/11/2025.
//


#include "configParser.h"

#include <yaml-cpp/yaml.h>

void configParser::configInit() {

    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "Config";
    out << YAML::BeginMap;
    out << YAML::Key << "Token";
    out << YAML::Value << "BOT_TOKEN";
    out << YAML::EndMap;
    out << YAML::EndMap;

    std::ofstream fout("config.yml");
    fout << out.c_str();
}

bool configParser::configExists(const std::string& name) {
    std::ifstream fin(name);
    return fin.good();
}

std::string configParser::configRead(const std::string& name) {
    YAML::Node config = YAML::LoadFile("config.yml");
    if (name == "token" || name == "Token") {
        std::string token = config["Config"]["Token"].as<std::string>();
        if (!token.empty()) {
            return token;
        }
        return "Incorrect Token";
    }
    return "Incorrect parameter";
}

