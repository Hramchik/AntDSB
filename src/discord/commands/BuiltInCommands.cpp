//
// Created by sanek on 05/12/2025.
//

#include <iostream>
#include <dpp/dpp.h>

#include "BuiltInCommands.h"
#include "CommandRegistry.h"
#include "logger/Logger.h"
#include "../ui/ApplicationUI.h"

static dpp::cluster* g_cluster = nullptr;

void BuiltInCommands::SetCluster(dpp::cluster* cluster) {
    g_cluster = cluster;
}

void BuiltInCommands::RegisterAll() {
    CommandRegistry::RegisterCommand({
        "post-application",
        "Отправить сообщение с заявкой в канал",
        [](const dpp::slashcommand_t& event) {
            try {
                if (!g_cluster) {
                    event.reply("❌ Кластер не инициализирован");
                    return;
                }

                dpp::message msg_to_send = ApplicationUI::CreateApplicationEntryMessageV2();
                msg_to_send.channel_id = event.command.channel_id;

                LogInfo("[post-application] Sending v2 message to channel " +
                        std::to_string(event.command.channel_id));

                event.reply("✅ Сообщение отправлено");

                g_cluster->message_create(msg_to_send, [](const dpp::confirmation_callback_t& callback) {
                    if (callback.is_error()) {
                        LogError("[post-application] ERROR: " + callback.get_error().human_readable);
                    }
                });
            } catch (const std::exception& e) {
                LogError(std::string("[post-application] EXCEPTION: ") + e.what());
            }
        }
    });
}
