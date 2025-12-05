//
// Created by sanek on 06/12/2025.
//

#ifndef ANTDSB_CALLBACKREGISTRY_H
#define ANTDSB_CALLBACKREGISTRY_H

#include <dpp/dpp.h>
#include <string>
#include <map>
#include <functional>

class CallbackRegistry {
public:
    struct ButtonCallbackInfo {
        std::string custom_id;
        std::function<void(const dpp::button_click_t&)> callback;
    };

    struct FormCallbackInfo {
        std::string custom_id;
        std::function<void(const dpp::form_submit_t&)> callback;
    };

    static void RegisterButtonCallback(const ButtonCallbackInfo& callback);

    static void RegisterFormCallback(const FormCallbackInfo& callback);

    static void HandleButtonClick(const dpp::button_click_t& event);

    static void HandleFormSubmit(const dpp::form_submit_t& event);

private:
    static std::map<std::string, ButtonCallbackInfo>& GetButtonCallbacks();

    static std::map<std::string, FormCallbackInfo>& GetFormCallbacks();
};

#endif //ANTDSB_CALLBACKREGISTRY_H