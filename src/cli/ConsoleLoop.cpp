//
// Created by sanek on 13/12/2025.
//

#include "ConsoleLoop.h"
#include "discord/DiscordBot.h"
#include "logger/Logger.h"

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
                auto st = bot.GetStatus();
                LogInfo(std::string("Bot running: ") + (st.running ? "yes" : "no"));
                if (!st.last_error.empty()) {
                    LogInfo("Last error: " + st.last_error);
                }
            } else if (command.rfind("say ", 0) == 0) {
                // say <channel_id> <text...>
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
                bot.SendMessage(static_cast<dpp::snowflake>(cid_ll), text);
            } else {
                LogInfo("Unknown command");
            }
        }

        bot.Wait();
        LogInfo("Bot stopped...");
    }

} // namespace cli