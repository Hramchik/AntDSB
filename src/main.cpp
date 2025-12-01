#include <dpp/dpp.h>
#include <iostream>
#include "DiscordBot/DiscordBot.h"
#include "configParser/configParser.h"

int main() {
    if (!configParser::configExists("config.yml")) {
        configParser::configInit();
        std::cout << "Config created. Please add your bot token and restart." << std::endl;
        return -1;
    }

    std::string BOT_TOKEN = configParser::configRead("token");

    std::cout << "Starting Discord bot with ThreadPool..." << std::endl;

    DiscordBot::botInit(BOT_TOKEN);

    return 0;
}
