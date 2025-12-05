#include <iostream>
#include <thread>

#include "discord/DiscordBot.h"
#include "config/ConfigManager.h"
#include "utils/TokenValidator.h"

int main() {
    const std::string CONFIG_PATH = "config/config.yml";

    if (!ConfigManager::ConfigExists(CONFIG_PATH)) {
        std::cout << "First launch detected. Creating default config...\n";
        ConfigManager::CreateDefaultConfig(CONFIG_PATH);
        std::cout << "Please configure config/config.yml and restart the application\n";
        return 0;
    }

    std::string token = ConfigManager::ReadConfigValue(CONFIG_PATH, "discord.token");
    if (!TokenValidator::ValidateDiscordBotToken(token)) {
        std::cerr << "Discord bot token is invalid. Check config/config.yml\n";
        return 1;
    }

    std::cout << "[main] Thread ID: " << std::this_thread::get_id() << std::endl;

    DiscordBot bot(token);
    bot.StartAsync();

    std::cout << "Bot is running in background thread. Type 'exit' to stop.\n";

    std::string command;
    while (std::getline(std::cin, command)) {
        if (command == "exit" || command == "quit") {
            std::cout << "Shutting down bot...\n";
            bot.Stop();
            break;
        }
        if (command == "status") {
            std::cout << "Bot is running\n";
        } else {
            std::cout << "Unknown command\n";
        }
    }

    bot.Wait();
    std::cout << "Bot stopped\n";
    return 0;
}