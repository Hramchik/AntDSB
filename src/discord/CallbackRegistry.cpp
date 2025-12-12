//
// Created by sanek on 06/12/2025.
//

#include <iostream>

#include "CallbackRegistry.h"
#include "logger/Logger.h"

CallbackRegistry::ButtonMap& CallbackRegistry::GetButtonCallbacks() {
    static ButtonMap callbacks;
    return callbacks;
}

CallbackRegistry::FormMap& CallbackRegistry::GetFormCallbacks() {
    static FormMap callbacks;
    return callbacks;
}

void CallbackRegistry::RegisterButtonCallback(const ButtonCallbackInfo& callback) {
    GetButtonCallbacks()[callback.custom_id] = callback;
    LogDebug("[CallbackRegistry] Button callback registered: " + callback.custom_id);
}

void CallbackRegistry::RegisterFormCallback(const FormCallbackInfo& callback) {
    GetFormCallbacks()[callback.custom_id] = callback;
    LogDebug("[CallbackRegistry] Form callback registered: " + callback.custom_id);
}

void CallbackRegistry::HandleButtonClick(const dpp::button_click_t& event) {
    try {
        auto& callbacks = GetButtonCallbacks();

        // поддержка custom_id с параметрами: "id:...:..."
        std::string key = event.custom_id;
        auto pos = key.find(':');
        if (pos != std::string::npos) {
            key = key.substr(0, pos);  // берём только префикс
        }

        auto it = callbacks.find(key);
        if (it != callbacks.end()) {
            it->second.callback(event);
        } else {
            LogError("[CallbackRegistry] Unknown button ID: " + event.custom_id);
        }

    } catch (const std::exception& e) {
        std::string except = e.what();
        LogError("[CallbackRegistry] Error handling button click: " + except);
    }
}

void CallbackRegistry::HandleFormSubmit(const dpp::form_submit_t& event) {
    try {
        auto& callbacks = GetFormCallbacks();
        auto it = callbacks.find(event.custom_id);

        if (it != callbacks.end()) {
            it->second.callback(event);
        } else {
            LogError("[CallbackRegistry] Unknown form ID: " + event.custom_id);
        }
    } catch (const std::exception& e) {
        std::string except = e.what();
        LogError("[CallbackRegistry] Error handling form submit: " + except);
    }
}
