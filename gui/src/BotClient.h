//
// Created by sanek on 13/12/2025.
//

#ifndef ANTDSB_BOTCLIENT_H
#define ANTDSB_BOTCLIENT_H

#include <memory>
#include <string>
#include <vector>
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

    bool ListChannels(std::vector<antdsb::ChannelInfo>& out, std::string& error);

    bool ListMessages(uint64_t channel_id,
                      uint32_t limit,
                      std::vector<antdsb::MessageInfo>& out,
                      std::string& error);

private:
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<antdsb::BotService::Stub> stub_;
};

#endif //ANTDSB_BOTCLIENT_H
