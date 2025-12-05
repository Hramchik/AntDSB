//
// Created by sanek on 05/12/2025.
//

#include "TokenValidator.h"
#include <curl/curl.h>
#include <iostream>

namespace TokenValidator {

    bool ValidateDiscordBotToken(const std::string& token) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cerr << "[TokenValidator] Failed to init CURL\n";
            return false;
        }

        std::string url = "https://discord.com/api/v10/oauth2/applications/@me";
        std::string authHeader = "Authorization: Bot " + token;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, authHeader.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "AnthemicBotTokenValidator/1.0");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                         +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
                             return size * nmemb;
                         });

        CURLcode res = curl_easy_perform(curl);
        long http_code = 0;

        if (res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        } else {
            std::cerr << "[TokenValidator] CURL error: " << curl_easy_strerror(res) << "\n";
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            return false;
        }

        if (http_code == 200) {
            std::cout << "[TokenValidator] Token is VALID (HTTP " << http_code << ")\n";
            return true;
        } else {
            std::cout << "[TokenValidator] Token is INVALID (HTTP " << http_code << ")\n";
            return false;
        }
    }

} // namespace TokenValidator