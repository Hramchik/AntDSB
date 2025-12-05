//
// Created by sanek on 05/12/2025.
//

#ifndef ANTDSB_CONFIGMANAGER_H
#define ANTDSB_CONFIGMANAGER_H

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

class ConfigManager {
public:
    static bool CreateDefaultConfig(const std::string& configPath);

    static bool ConfigExists(const std::string& configPath);

    static YAML::Node ReadConfig(const std::string& configPath);

    static bool SaveConfig(const std::string& configPath, const YAML::Node& config);

    static std::string ReadConfigValue(const std::string& configPath, const std::string& key);

    template<typename T>
    static T ReadConfigValue(const std::string& configPath, const std::string& key, const T& defaultValue = T()) {
        try {
            YAML::Node config = ReadConfig(configPath);

            if (config.IsNull()) {
                return defaultValue;
            }

            YAML::Node node = GetNodeByPath(config, key);

            if (node.IsNull()) {
                std::cerr << "Key not found: " << key << std::endl;
                return defaultValue;
            }

            return node.as<T>();

        } catch (const std::exception& e) {
            std::cerr << "Error reading config value: " << e.what() << std::endl;
            return defaultValue;
        }
    }

    static std::vector<uint64_t> ReadConfigArray(const std::string& configPath, const std::string& key) {
        try {
            YAML::Node config = ReadConfig(configPath);

            if (config.IsNull()) {
                return {};
            }

            YAML::Node node = GetNodeByPath(config, key);

            if (node.IsNull() || !node.IsSequence()) {
                std::cerr << "Key not found or not a sequence: " << key << std::endl;
                return {};
            }

            std::vector<uint64_t> result;
            for (const auto& item : node) {
                result.push_back(item.as<uint64_t>());
            }

            return result;

        } catch (const std::exception& e) {
            std::cerr << "Error reading config array: " << e.what() << std::endl;
            return {};
        }
    }

private:
    static YAML::Node GetNodeByPath(const YAML::Node& root, const std::string& path);

    static std::string GetDefaultConfigTemplate();
};

#endif //ANTDSB_CONFIGMANAGER_H