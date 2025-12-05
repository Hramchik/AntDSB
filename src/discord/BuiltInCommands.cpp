//
// Created by sanek on 05/12/2025.
//

#include <iostream>
#include <dpp/dpp.h>

#include "BuiltInCommands.h"
#include "CommandRegistry.h"
#include "ui/ApplicationUI.h"


// Глобальный указатель на кластер
static dpp::cluster* g_cluster = nullptr;

void BuiltInCommands::SetCluster(dpp::cluster* cluster) {
    g_cluster = cluster;
}

void BuiltInCommands::RegisterAll() {

    // POST-APPLICATION команда
    CommandRegistry::RegisterCommand({
"post-application",
"Отправить сообщение с заявкой в канал",
[](const dpp::slashcommand_t& event) {
    try {
        if (!g_cluster) {
            event.reply("❌ Кластер не инициализирован");
            return;
        }

        dpp::embed embed = ApplicationUI::CreateApplicationEmbed();
        dpp::component button = ApplicationUI::CreateApplicationButton();

        dpp::message msg_to_send;
        msg_to_send.channel_id = event.command.channel_id;
        msg_to_send.add_embed(embed);

        // Создаём ACTION ROW для кнопки
        dpp::component action_row;
        action_row.type = dpp::cot_action_row;
        action_row.add_component(button);
        msg_to_send.add_component(action_row);

        std::cout << "[post-application] Sending message to channel "
                  << event.command.channel_id << std::endl;

        event.reply("✅ Сообщение отправлено");

        g_cluster->message_create(msg_to_send,
            [](const dpp::confirmation_callback_t& callback) {
                if (callback.is_error()) {
                    std::cerr << "[post-application] ERROR: "
                              << callback.get_error().human_readable << std::endl;
                }
            }
        );

    } catch (const std::exception& e) {
            std::cerr << "[post-application] EXCEPTION: " << e.what() << std::endl;
            }
        }
    });
}
