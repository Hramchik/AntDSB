//
// Created by sanek on 13/12/2025.
//

#include "ConsoleLoop.h"
#include "../discord/core/DiscordBot.h"
#include "logger/Logger.h"
#include <sstream>
#include <iostream>

namespace cli {

    void RunConsoleLoop(DiscordBot& bot) {
        LogInfo("Bot is running in background thread. Type 'exit' to stop.");
        std::string command;

        while (std::getline(std::cin, command)) {
            if (command == "exit" || command == "quit") {
                LogInfo("Shutting down bot...");
                bot.Stop();
                break;
            } else if (command == "status") {
                LogInfo("Status: running (CLI only, no detailed state)");
            } else if (command.rfind("say ", 0) == 0) {
                std::istringstream iss(command.substr(4));
                long long cid_ll;
                if (!(iss >> cid_ll)) {
                    LogInfo("Usage: say <channel_id> <text>");
                    continue;
                }
                std::string text;
                std::getline(iss, text);
                if (!text.empty() && text[0] == ' ')
                    text.erase(0, 1);

                try {
                    dpp::cluster& cl = bot.GetCluster();
                    dpp::message msg;
                    msg.set_content(text);
                    msg.channel_id = static_cast<dpp::snowflake>(cid_ll);
                    cl.message_create(msg);
                    LogInfo("Message sent from CLI");
                } catch (const std::exception& e) {
                    LogError(std::string("Error sending message from CLI: ") + e.what());
                }
            } else {
                LogInfo("Unknown command");
            }
        }

        bot.Wait();
        LogInfo("Bot stopped...");
    }

} // namespace cli
