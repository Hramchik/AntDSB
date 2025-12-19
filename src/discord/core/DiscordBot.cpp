//
// Created by sanek on 05/12/2025.
//

#include "DiscordBot.h"

#include <sstream>
#include <thread>

#include <dpp/dpp.h>

#include "discord/core/DiscordCluster.h"
#include "discord/commands/CommandRegistry.h"
#include "discord/callbacks/CallbackRegistry.h"
#include "discord/commands/BuiltInCommands.h"
#include "discord/callbacks/BuiltInCallbacks.h"
#include "discord/logging/DiscordLogging.h"
#include "logger/Logger.h"

DiscordBot::DiscordBot(const std::string& token)
    : running(false)
{
    try {
        unsigned int hw = std::thread::hardware_concurrency();
        if (hw == 0) hw = 4;
        LogDebug("[DiscordBot] HW threads: " + std::to_string(hw));

        uint32_t intents = dpp::i_default_intents
                         | dpp::i_message_content
                         | dpp::i_direct_messages
                         | dpp::i_guild_voice_states;

        cluster = std::make_unique<dpp::cluster>(token, intents);
        DiscordCluster::SetCluster(cluster.get());

        LogInfo("[DiscordBot] Initialized successfully");
    } catch (const std::exception& e) {
        std::string except = e.what();
        LogError("[DiscordBot] Error initializing: " + except);
    }
}

DiscordBot::~DiscordBot() {
    Stop();
}

bool DiscordBot::Start() {
    try {
        if (!cluster) {
            LogError("[DiscordBot] Cluster is not initialized");
            return false;
        }

        running = true;
        RegisterEventHandlers();
        cluster->start(dpp::st_wait);

        LogInfo("[DiscordBot] Started (sync)");
        return true;
    } catch (const std::exception& e) {
        std::string except = e.what();
        LogError("[DiscordBot] Error starting: " + except);
        return false;
    }
}

void DiscordBot::StartAsync() {
    if (running) {
        LogInfo("[DiscordBot] Already running");
        return;
    }

    running = true;
    botThread = std::thread(&DiscordBot::BotThreadFunction, this);

    std::ostringstream oss;
    oss << botThread.get_id();
    LogInfo("[DiscordBot] Started async (thread: " + oss.str() + ")");
}

void DiscordBot::BotThreadFunction() {
    try {
        if (!cluster) {
            LogError("[DiscordBot] Cluster is not initialized");
            return;
        }

        RegisterEventHandlers();
        cluster->start(dpp::st_wait);
    } catch (const std::exception& e) {
        std::string except = e.what();
        LogError("[DiscordBot] Error in bot thread: " + except);
    }
}

void DiscordBot::Wait() {
    if (botThread.joinable()) {
        botThread.join();
        LogInfo("[DiscordBot] Bot thread finished");
    }
}

void DiscordBot::Stop() {
    if (!running) {
        return;
    }

    running = false;
    CommandRegistry::StopCommandProcessor();

    if (cluster) {
        cluster->shutdown();
        LogInfo("[DiscordBot] Stopped");
    }

    if (botThread.joinable()) {
        botThread.join();
    }
}

dpp::cluster& DiscordBot::GetCluster() {
    return *cluster;
}

void DiscordBot::RegisterEventHandlers() {
    try {
        BuiltInCommands::SetCluster(cluster.get());
        BuiltInCommands::RegisterAll();
        BuiltInCallbacks::RegisterAll();
        CommandRegistry::StartCommandProcessor();

        RegisterCoreHandlers();
        DiscordLogging::RegisterMessageLoggingHandlers(*cluster);
        DiscordLogging::RegisterVoiceLoggingHandlers(*cluster);

        std::ostringstream oss;
        oss << std::this_thread::get_id();
        LogInfo("[DiscordBot] Handlers registered (thread: " + oss.str() + ")");
    } catch (const std::exception& e) {
        std::string except = e.what();
        LogError("[DiscordBot] Error registering handlers: " + except);
    }
}

void DiscordBot::RegisterCoreHandlers() {
    cluster->on_ready([this](const dpp::ready_t&) {
        if (dpp::run_once<struct register_bot_commands>()) {
            std::ostringstream oss;
            oss << std::this_thread::get_id();
            LogInfo("[DiscordBot] Bot is ready (thread: " + oss.str() + ")");
            CommandRegistry::RegisterAllCommands(cluster.get());
        }
    });

    cluster->on_slashcommand([this](const dpp::slashcommand_t& event) {
        CommandRegistry::HandleCommand(event, cluster.get());
    });

    cluster->on_button_click([this](const dpp::button_click_t& event) {
        LogInfo(std::string("[DiscordBot] Button clicked: ") + event.custom_id);
        CallbackRegistry::HandleButtonClick(event);
    });

    cluster->on_form_submit([this](const dpp::form_submit_t& event) {
        LogInfo(std::string("[DiscordBot] Form submitted: ") + event.custom_id);
        CallbackRegistry::HandleFormSubmit(event);
    });

    cluster->on_guild_member_add([this](const dpp::guild_member_add_t&) {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        LogInfo("[DiscordBot][" + oss.str() + "] New member joined");
    });
}
