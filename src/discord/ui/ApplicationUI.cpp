//
// Created by sanek on 06/12/2025.
//

#include "ApplicationUI.h"
#include "logger/Logger.h"

namespace ApplicationUI {

    dpp::interaction_modal_response CreateApplicationModal() {
        dpp::interaction_modal_response modal;
        modal.custom_id = "modal_apply";
        modal.title = "Заявка на вступление";
        modal.components.clear();

        auto create_text_input = [](const std::string& label,
                                    const std::string& custom_id,
                                    int min_len, int max_len,
                                    bool is_paragraph = false) -> std::vector<dpp::component> {
            dpp::component c;
            c.type = dpp::cot_text;
            c.label = label;
            c.custom_id = custom_id;
            c.required = true;
            c.min_length = min_len;
            c.max_length = max_len;
            c.text_style = is_paragraph ? dpp::text_paragraph : dpp::text_short;

            std::vector<dpp::component> row;
            row.push_back(c);
            return row;
        };

        // Добавляем 5 полей
        modal.components.push_back(create_text_input("Реальное имя", "field_name", 2, 50, false));
        modal.components.push_back(create_text_input("Возраст", "field_age", 1, 3, false));
        modal.components.push_back(create_text_input("Игровой ник (точно как в игре)", "field_nickname", 2, 32, false));
        modal.components.push_back(create_text_input("О себе", "field_about", 10, 500, true));
        modal.components.push_back(create_text_input("Откуда узнали о проекте?", "field_source", 5, 300, true));

        LogInfo("[ApplicationUI] Modal created with " + std::to_string(modal.components.size()) + " rows");

        return modal;
    }

    dpp::message CreateApplicationEntryMessageV2() {
        dpp::component container;
        container.set_type(dpp::cot_container);

        // Image in header
        {
            dpp::component section;
            section.set_type(dpp::cot_media_gallery);
            section.add_media_gallery_item(
                dpp::component()
                    .set_type(dpp::cot_thumbnail)
                    .set_thumbnail("https://cdn.discordapp.com/attachments/1447024668627308625/1447024737095127080/start_game.png?ex=693d5e8d&is=693c0d0d&hm=fa0c8c62107ad7b0b4e5167a935f734785849cfdfd02ffebd2623f12d14a1638&")
            );

            container.add_component_v2(section);
        }

        // Main text
        {
            dpp::component text;
            text.set_type(dpp::cot_text_display)
                .set_content(
                    "**Заявка на вступление в проект**\n\n"
                    "__Приветствуем тебя!__\n\n"
                    "Хочешь стать частью нашего дружного сообщества?\n"
                    "Нажми кнопку ниже и заполни необходимую анкету — это займет всего пару минут.\n\n"
                    "Мы ждали именно тебя!\n\n"
                );

            container.add_component_v2(text);
        }

        // Separator
        {
            dpp::component separator;
            separator.set_type(dpp::cot_separator)
                .set_spacing(dpp::sep_large)
                .set_divider(true);
            container.add_component_v2(separator);
        }

        // Button for open modal
        {
            dpp::component section;
            section.set_type(dpp::cot_section);

            dpp::component text;
            text.set_type(dpp::cot_text_display)
                .set_content("Для подачи заявки нужно нажать сюда");

            dpp::component button;
            button.set_type(dpp::cot_button)
                .set_style(dpp::cos_success)
                .set_label("Подать заявку")
                .set_id("btn_apply_modal");

            section.add_component_v2(text);
            section.accessory = std::make_shared<dpp::component>(button);

            container.add_component_v2(section);
        }

        dpp::message msg;
        msg.set_flags(dpp::m_using_components_v2)
           .add_component_v2(container)
           .set_allowed_mentions(false, false, false, false);

        return msg;
    }

} // namespace ApplicationUI
