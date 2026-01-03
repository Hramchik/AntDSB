//
// Created by sanek on 05/12/2025.
//

#include "ConfigManager.h"
#include "logger/Logger.h"

#include <fstream>

bool ConfigManager::ConfigExists(const std::string& configPath) {
    return fs::exists(configPath);
}

std::string ConfigManager::GetDefaultConfigTemplate() {
    return R"(# === Application Information ===
application:
  version: "0.4.1"                  # Application version (e.g., 0.1.0, 1.0.0)
  name: "ANTHEMIC_SYSTEM"             # Application name

# === Discord Bot Configuration ===
discord:
  token: "YOUR_DISCORD_BOT_TOKEN"   # Get from Discord Developer Portal: https://discord.com/developers/applications
  command_prefix: "!"               # Command prefix (e.g., !, >, $, /)

# === Logging Configuration ===
logging:
  send_hour: 0                      # Hour to send logs in 24-hour format (0-23)
  log_channel_id: 0                 # Discord channel ID for general logs
  application_log_channel_id: 0     # Discord channel ID for application logs

# === Channel and Category IDs ===
channels:
  ticket_channel_id: 0              # Discord channel ID where tickets are created
  ticket_archive_category_id: 0     # Discord category ID for archived tickets
  tickets_category_id: 0            # Discord category ID where ticket channels are created
  voice_category_id: 0              # Discord category ID for voice channels
  voice_join_channel_id: 0          # Discord voice channel ID for joining
  application_channel_id: 0         # Discord channel ID for applications
  applications_category_id: 0       # Discord category ID for application channels

# === Permissions and Roles ===
roles:
  admins_role_id: 0                 # Discord role ID for administrators
  technical_role_id: 0              # Discord role ID for technical support team
  whitelist_role_id: 0              # Discord role ID for whitelist access
  dupe_role_id: 0                   # Discord role ID for duplicate report handlers
  other_role_id: 0                  # Discord role ID for other issues handlers
  support_role_id: 0                # Discord role ID for support team
  ban_role_id: 0                    # Discord role ID for ban appeal handlers
  application_review_role_id: 0     # Discord role ID for application reviewers
  remove_role_id: 0                 # Discord role ID to be removed from new members
  grant_role_id: 0                  # Discord role ID to be granted to new members
  auto_roles_ids:                   # Discord role IDs to auto-assign to new members
    - 0                             # Replace 0 with actual role IDs
    - 0
    - 0

# === Database Configuration ===
database:
  default_database: "tickets.db"    # Path to SQLite database file (or use other database name)

# === Rate Limiting ===
rate_limits:
  ticket_ratelimit_seconds: 30      # Cooldown between ticket creation attempts in seconds

# === Whitelist Configuration ===
whitelist:
  api_url: "http://localhost:8080"  # Whitelist API endpoint URL (e.g., http://localhost:19132/add)
  api_token: "your_api_token_here"  # API authentication token for whitelist service
  command_template: "whitelist add {nick}"  # Command template with {nick} placeholder for username
)";
}


bool ConfigManager::CreateDefaultConfig(const std::string& configPath) {
    try {
        fs::path configFile(configPath);
        fs::path configDir = configFile.parent_path();

        if (!configDir.empty() && !fs::exists(configDir)) {
            fs::create_directories(configDir);
            std::cout << "Created directory: " << configDir << std::endl;
        }

        if (fs::exists(configPath)) {
            std::cout << "Config file already exists: " << configPath << std::endl;
            return true;
        }

        std::ofstream outFile(configPath);

        if (!outFile.is_open()) {
            std::cerr << "Failed to create config file: " << configPath << std::endl;
            return false;
        }

        outFile << GetDefaultConfigTemplate();
        outFile.close();

        std::cout << "Default config created: " << configPath << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error creating config: " << e.what() << std::endl;
        return false;
    }
}

YAML::Node ConfigManager::ReadConfig(const std::string& configPath) {
    try {
        if (!ConfigExists(configPath)) {
            LogError("Config file not found: " + configPath);
            return YAML::Node();
        }

        YAML::Node config = YAML::LoadFile(configPath);
        LogInfo("Config loaded successfully: " + configPath);
        return config;

    } catch (const YAML::Exception& e) {
        std::string except = e.what();
        LogError("Error parsing config: " + except);
        return YAML::Node();
    }
}

bool ConfigManager::SaveConfig(const std::string& configPath, const YAML::Node& config) {
    try {
        std::ofstream outFile(configPath);

        if (!outFile.is_open()) {
            LogError("Failed to open config file for writing: " + configPath);
            return false;
        }

        outFile << config;
        outFile.close();

        LogInfo("Config saved successfully: " + configPath);
        return true;

    } catch (const std::exception& e) {
        std::string except = e.what();
        LogError("Error saving config: " + except);
        return false;
    }
}

YAML::Node ConfigManager::GetNodeByPath(const YAML::Node& root, const std::string& path) {
    std::istringstream iss(path);
    std::string part;
    YAML::Node current = root;

    while (std::getline(iss, part, '.')) {
        if (current.IsNull()) {
            return YAML::Node();
        }
        current = current[part];
    }

    return current;
}

std::string ConfigManager::ReadConfigValue(const std::string& configPath, const std::string& key) {
    try {
        YAML::Node config = ReadConfig(configPath);

        if (config.IsNull()) {
            return "";
        }

        YAML::Node node = GetNodeByPath(config, key);

        if (node.IsNull()) {
            LogError("Key not found: " + key);
            return "";
        }

        return node.as<std::string>();

    } catch (const std::exception& e) {
        std::string except = e.what();
        LogError("Error reading config value: " + except);
        return "";
    }
}