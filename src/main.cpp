#include <iostream>
#include <thread>

#include "discord/DiscordBot.h"
#include "config/ConfigManager.h"
#include "utils/TokenValidator.h"
#include "logger/Logger.h"

int main() {
    LogInit("logs");
    LogInfo("AntDSB starting!");

    const std::string CONFIG_PATH = "config/config.yml";

    if (!ConfigManager::ConfigExists(CONFIG_PATH)) {
        LogInfo("Loading configuration from: " + CONFIG_PATH);
        ConfigManager::CreateDefaultConfig(CONFIG_PATH);
        LogInfo("Please configure config/config.yml and restart the application\n");
        return 0;
    }

    std::string token = ConfigManager::ReadConfigValue(CONFIG_PATH, "discord.token");
    if (!TokenValidator::ValidateDiscordBotToken(token)) {
        LogError("Discord bot token is invalid. Check config/config.yml\n");
        return 1;
    }

    std::ostringstream oss;
    oss << std::this_thread::get_id();
    LogInfo("[main] Thread ID: " + oss.str());

    DiscordBot bot(token);
    bot.StartAsync();

    LogInfo("Bot is running in background thread. Type 'exit' to stop.");

    std::string command;
    while (std::getline(std::cin, command)) {
        if (command == "exit" || command == "quit") {
            LogInfo("Shutting down bot...");
            bot.Stop();
            break;
        }
        if (command == "status") {
            LogInfo("Bot is running");
        } else {
            LogInfo("Unknown command");
        }
    }

    bot.Wait();
    LogInfo("Bot stopped...");
    return 0;
}