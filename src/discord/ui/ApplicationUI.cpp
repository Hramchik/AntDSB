//
// Created by sanek on 06/12/2025.
//

#include "ApplicationUI.h"

namespace ApplicationUI {

dpp::embed CreateApplicationEmbed() {
    dpp::embed embed;
    embed.set_title("Заявка на вступление в проект");
    embed.set_description("Приветствуем тебя!\n\nХочешь стать частью нашего дружного сообщества?\n"
                          "Нажми кнопку ниже и заполни необходимую анкету — это займет всего пару минут");
    embed.set_color(0x00AA00);
    embed.set_footer(dpp::embed_footer().set_text("Мы ждали именно тебя!"));

    return embed;
}

dpp::component CreateApplicationButton() {
    dpp::component button;
    button.type = dpp::cot_button;
    button.label = "Подать заявку";
    button.style = dpp::cos_success;
    button.custom_id = "btn_apply_modal";
    button.emoji.name = "✅";

    return button;
}

dpp::interaction_modal_response CreateApplicationModal() {
    dpp::interaction_modal_response modal;
    modal.custom_id = "modal_apply";
    modal.title = "Заявка на вступление";
    modal.components.clear();

    // Lambda для создания text input row
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
        if (is_paragraph) {
            c.text_style = dpp::text_paragraph;
        } else {
            c.text_style = dpp::text_short;  // явно указываем short
        }

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

    std::cout << "[ApplicationUI] Modal created with " << modal.components.size() << " rows\n";

    return modal;
}

} // namespace ApplicationUI
