#include "discord/DiscordBot.h"
#include "config/ConfigManager.h"
#include "utils/TokenValidator.h"
#include "logger/Logger.h"
#include "cli/ConsoleLoop.h"
#include "api/GrpcServer.h"

int main(int argc, char** argv) {
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

    DiscordBot bot(token);
    bot.StartAsync();

    GrpcServer grpc_server(bot);
    grpc_server.Start("0.0.0.0:50051");

    // твой CLI-режим
    cli::RunConsoleLoop(bot);

    grpc_server.Shutdown();
    return 0;
}
