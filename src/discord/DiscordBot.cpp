//
// Created by sanek on 05/12/2025.
//

#include <iostream>
#include <thread>

#include "DiscordBot.h"
#include "CommandRegistry.h"
#include "CallbackRegistry.h"
#include "BuiltInCommands.h"
#include "callbacks/BuiltInCallbacks.h"
#include "utils/ThreadUtils.h"

DiscordBot::DiscordBot(const std::string& token)
    : running(false) {
    try {
        unsigned int hw = std::thread::hardware_concurrency();
        if (hw == 0) hw = 4;
        std::cout << "[DiscordBot] HW threads: " << hw << std::endl;

        uint32_t intents = dpp::i_default_intents
                        | dpp::i_message_content
                        | dpp::i_direct_messages;

        cluster = std::make_unique<dpp::cluster>(token, intents);

        std::cout << "[DiscordBot] Initialized successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[DiscordBot] Error initializing: " << e.what() << std::endl;
    }
}

DiscordBot::~DiscordBot() {
    Stop();
}

bool DiscordBot::Start() {
    try {
        if (!cluster) {
            std::cerr << "[DiscordBot] Cluster is not initialized" << std::endl;
            return false;
        }

        running = true;
        RegisterEventHandlers();
        cluster->start(dpp::st_wait);

        std::cout << "[DiscordBot] Started (sync)" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "[DiscordBot] Error starting: " << e.what() << std::endl;
        return false;
    }
}

void DiscordBot::StartAsync() {
    if (running) {
        std::cout << "[DiscordBot] Already running" << std::endl;
        return;
    }

    running = true;
    botThread = std::thread(&DiscordBot::BotThreadFunction, this);
    std::cout << "[DiscordBot] Started async (thread: " << botThread.get_id() << ")\n";
}

void DiscordBot::BotThreadFunction() {
    try {
        if (!cluster) {
            std::cerr << "[DiscordBot] Cluster is not initialized" << std::endl;
            return;
        }

        RegisterEventHandlers();
        cluster->start(dpp::st_wait);

    } catch (const std::exception& e) {
        std::cerr << "[DiscordBot] Error in bot thread: " << e.what() << std::endl;
    }
}

void DiscordBot::Wait() {
    if (botThread.joinable()) {
        botThread.join();
        std::cout << "[DiscordBot] Bot thread finished\n";
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
        std::cout << "[DiscordBot] Stopped\n";
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
        BuiltInCommands::SetCluster(cluster.get());  // устанавливаем глобальный кластер

        BuiltInCommands::RegisterAll();

        BuiltInCallbacks::RegisterAll();

        CommandRegistry::StartCommandProcessor();

        cluster->on_ready([this](const dpp::ready_t& event) {
            if (dpp::run_once<struct register_bot_commands>()) {
                std::cout << "[DiscordBot] Bot is ready (thread: "
                          << std::this_thread::get_id() << ")\n";
                CommandRegistry::RegisterAllCommands(cluster.get());
            }
        });

        cluster->on_slashcommand([this](const dpp::slashcommand_t& event) {
            CommandRegistry::HandleCommand(event, cluster.get());  // передаём кластер
        });


        cluster->on_button_click([this](const dpp::button_click_t& event) {
            std::cout << "[DiscordBot] Button clicked: " << event.custom_id << "\n";
                CallbackRegistry::HandleButtonClick(event);
        });

        cluster->on_form_submit([this](const dpp::form_submit_t& event) {
            std::cout << "[DiscordBot] Form submitted: " << event.custom_id << "\n";
                CallbackRegistry::HandleFormSubmit(event);
        });

        cluster->on_message_create([this](const dpp::message_create_t& event) {
            if (event.msg.author.is_bot()) {
                return;
            }

            std::cout << "[DiscordBot][" << std::this_thread::get_id() << "] "
                      << event.msg.author.username << ": "
                      << event.msg.content << std::endl;
        });

        cluster->on_guild_member_add([this](const dpp::guild_member_add_t& event) {
            std::cout << "[DiscordBot][" << std::this_thread::get_id() << "] "
                      << "New member joined\n";
        });

        std::cout << "[DiscordBot] Handlers registered (thread: "
                  << std::this_thread::get_id() << ")\n";

    } catch (const std::exception& e) {
        std::cerr << "[DiscordBot] Error registering handlers: " << e.what() << std::endl;
    }
}