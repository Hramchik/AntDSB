//
// Created by sanek on 06/12/2025.
//

#ifndef ANTDSB_APPLICATIONUI_H
#define ANTDSB_APPLICATIONUI_H

#include <dpp/dpp.h>

namespace ApplicationUI {
    dpp::embed CreateApplicationEmbed();

    dpp::component CreateApplicationButton();

    dpp::interaction_modal_response CreateApplicationModal();
}

#endif //ANTDSB_APPLICATIONUI_H