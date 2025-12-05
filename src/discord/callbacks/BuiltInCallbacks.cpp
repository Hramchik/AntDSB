//
// Created by sanek on 06/12/2025.
//

#include <iostream>
#include <variant>

#include "BuiltInCallbacks.h"
#include "../CallbackRegistry.h"
#include "../ui/ApplicationUI.h"

void BuiltInCallbacks::RegisterAll() {
    // ===== BUTTON CALLBACKS =====

    CallbackRegistry::RegisterButtonCallback({
    "btn_apply_modal",
    [](const dpp::button_click_t& event) {
        std::cout << "[BuiltInCallbacks] Button clicked, creating modal...\n";

        try {
            dpp::interaction_modal_response modal = ApplicationUI::CreateApplicationModal();

            std::cout << "[BuiltInCallbacks] Modal created successfully\n";
            std::cout << "[BuiltInCallbacks] Modal custom_id: " << modal.custom_id << "\n";
            std::cout << "[BuiltInCallbacks] Modal title: " << modal.title << "\n";
            std::cout << "[BuiltInCallbacks] Modal components: " << modal.components.size() << " rows\n";

            for (size_t i = 0; i < modal.components.size(); i++) {
                std::cout << "  Row " << i << ": " << modal.components[i].size() << " components\n";
                for (size_t j = 0; j < modal.components[i].size(); j++) {
                    std::cout << "    - " << modal.components[i][j].label
                              << " (id: " << modal.components[i][j].custom_id << ")\n";
                }
            }

            std::cout << "[BuiltInCallbacks] Calling event.dialog()...\n";
            event.dialog(modal);
            std::cout << "[BuiltInCallbacks] Dialog sent successfully!\n";

        } catch (const std::exception& e) {
            std::cerr << "[BuiltInCallbacks] EXCEPTION: " << e.what() << "\n";
        } catch (...) {
            std::cerr << "[BuiltInCallbacks] UNKNOWN EXCEPTION\n";
        }
    }
});

    // ===== FORM CALLBACKS =====

    CallbackRegistry::RegisterFormCallback({
        "modal_apply",
        [](const dpp::form_submit_t& event) {
            std::cout << "[BuiltInCallbacks] Application form submitted\n";

            // Достаём значения из variant через std::get<std::string>()
            std::string name      = std::get<std::string>(event.components[0].components[0].value);
            std::string age_str   = std::get<std::string>(event.components[1].components[0].value);
            std::string nickname  = std::get<std::string>(event.components[2].components[0].value);
            std::string about     = std::get<std::string>(event.components[3].components[0].value);
            std::string source    = std::get<std::string>(event.components[4].components[0].value);

            int age = 0;
            try { age = std::stoi(age_str); } catch (...) { age = 0; }

            std::cout << "[BuiltInCallbacks] Application details:\n"
                      << "  Name: " << name << "\n"
                      << "  Age: " << age << "\n"
                      << "  Nickname: " << nickname << "\n"
                      << "  About: " << about << "\n"
                      << "  Source: " << source << "\n";

            dpp::embed response_embed;
            response_embed.set_title("✅ Заявка принята!");
            response_embed.set_description("Спасибо за заявку! Модераторы рассмотрят её в ближайшее время.");
            response_embed.set_color(0x00AA00);

            response_embed.add_field("Имя", name, true);
            response_embed.add_field("Возраст", std::to_string(age), true);
            response_embed.add_field("Ник", nickname, true);
            response_embed.add_field("О себе",
                                     about.substr(0, 100) + (about.size() > 100 ? "..." : ""), false);
            response_embed.add_field("Источник",
                                     source.substr(0, 100) + (source.size() > 100 ? "..." : ""), false);

            dpp::message response;
            response.add_embed(response_embed);

            event.reply(response);
        }
    });
}
