#include <chrono>
#include <thread>
#include <string>
#include <atomic>
#include <thread>

#include "discord/core/DiscordBot.h"
#include "config/ConfigManager.h"
#include "utils/TokenValidator.h"
#include "logger/Logger.h"
#include "cli/ConsoleControl.h"

static std::atomic_bool g_running{true};

int main(int argc, char** argv) {
    LogInit("logs");
    LogInfo("AntDSB starting!");

    const std::string CONFIG_PATH = "config/config.yml";

    if (!ConfigManager::ConfigExists(CONFIG_PATH)) {
        LogInfo("Config not found. Creating default config: " + CONFIG_PATH);
        ConfigManager::CreateDefaultConfig(CONFIG_PATH);
        LogInfo("Please configure config/config.yml and restart the application");
        return 0;
    }

    std::string token = ConfigManager::ReadConfigValue(CONFIG_PATH, "discord.token");
    if (!TokenValidator::ValidateDiscordBotToken(token)) {
        LogError("Discord bot token is invalid. Check config/config.yml");
        return 1;
    }

    LogInfo("Configuration loaded successfully");

    DiscordBot bot(token);
    bot.StartAsync();

    LogInfo("Discord bot is running. Type 'exit' or 'quit' to stop.");

    std::thread consoleThread(ConsoleControlThread, std::ref(g_running));

    while (g_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    bot.Stop();
    LogInfo("AntDSB stopped.");

    if (consoleThread.joinable()) {
        consoleThread.join();
    }

    return 0;
}
