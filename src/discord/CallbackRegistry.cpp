//
// Created by sanek on 06/12/2025.
//

#include <iostream>

#include "CallbackRegistry.h"

std::map<std::string, CallbackRegistry::ButtonCallbackInfo>& CallbackRegistry::GetButtonCallbacks() {
    static std::map<std::string, ButtonCallbackInfo> callbacks;
    return callbacks;
}

std::map<std::string, CallbackRegistry::FormCallbackInfo>& CallbackRegistry::GetFormCallbacks() {
    static std::map<std::string, FormCallbackInfo> callbacks;
    return callbacks;
}

void CallbackRegistry::RegisterButtonCallback(const ButtonCallbackInfo& callback) {
    GetButtonCallbacks()[callback.custom_id] = callback;
    std::cout << "[CallbackRegistry] Button callback registered: " << callback.custom_id << std::endl;
}

void CallbackRegistry::RegisterFormCallback(const FormCallbackInfo& callback) {
    GetFormCallbacks()[callback.custom_id] = callback;
    std::cout << "[CallbackRegistry] Form callback registered: " << callback.custom_id << std::endl;
}

void CallbackRegistry::HandleButtonClick(const dpp::button_click_t& event) {
    try {
        auto& callbacks = GetButtonCallbacks();
        auto it = callbacks.find(event.custom_id);

        if (it != callbacks.end()) {
            it->second.callback(event);
        } else {
            std::cerr << "[CallbackRegistry] Unknown button ID: " << event.custom_id << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "[CallbackRegistry] Error handling button click: " << e.what() << std::endl;
    }
}

void CallbackRegistry::HandleFormSubmit(const dpp::form_submit_t& event) {
    try {
        auto& callbacks = GetFormCallbacks();
        auto it = callbacks.find(event.custom_id);

        if (it != callbacks.end()) {
            it->second.callback(event);
        } else {
            std::cerr << "[CallbackRegistry] Unknown form ID: " << event.custom_id << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "[CallbackRegistry] Error handling form submit: " << e.what() << std::endl;
    }
}