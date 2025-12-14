//
// Created by sanek on 13/12/2025.
//

#ifndef ANTDSB_BOTCLIENT_H
#define ANTDSB_BOTCLIENT_H

#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "bot.grpc.pb.h"

class BotClient {
public:
    explicit BotClient(const std::string& address);

    antdsb::StatusReply GetStatus();
    bool SendMessage(uint64_t channel_id, const std::string& text, std::string& error);

private:
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<antdsb::BotService::Stub> stub_;
};

#endif //ANTDSB_BOTCLIENT_H