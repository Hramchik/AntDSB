//
// Created by sanek on 13/12/2025.
//

#ifndef ANTDSB_BOTCLIENT_H
#define ANTDSB_BOTCLIENT_H

#include <memory>
#include <string>
#include <cstdint>

#include <grpcpp/grpcpp.h>
#include <google/protobuf/empty.pb.h>

#include "bot.grpc.pb.h"

class BotClient {
public:
    explicit BotClient(const std::string& address);

    bool TryGetStatus(antdsb::StatusReply& out);

    bool SendMessage(uint64_t channel_id,
                     const std::string& text,
                     std::string& error);

    bool StartBot(antdsb::StatusReply& out, std::string& error);
    bool StopBot(antdsb::StatusReply& out, std::string& error);
    bool RestartBot(antdsb::StatusReply& out, std::string& error);

private:
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<antdsb::BotService::Stub> stub_;
};

#endif // ANTDSB_BOTCLIENT_H
