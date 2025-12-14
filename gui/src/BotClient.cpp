//
// Created by sanek on 13/12/2025.
//

#include "BotClient.h"

BotClient::BotClient(const std::string& address) {
    channel_ = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
    stub_ = antdsb::BotService::NewStub(channel_);
}

antdsb::StatusReply BotClient::GetStatus() {
    antdsb::StatusRequest req;
    antdsb::StatusReply rep;
    grpc::ClientContext ctx;

    auto status = stub_->GetStatus(&ctx, req, &rep);
    if (!status.ok()) {
        rep.set_running(false);
        rep.set_last_error(status.error_message());
    }
    return rep;
}

bool BotClient::SendMessage(uint64_t channel_id, const std::string& text, std::string& error) {
    antdsb::SendMessageRequest req;
    req.set_channel_id(channel_id);
    req.set_text(text);

    antdsb::SendMessageReply rep;
    grpc::ClientContext ctx;

    auto status = stub_->SendMessage(&ctx, req, &rep);
    if (!status.ok() || !rep.ok()) {
        if (!rep.error().empty())
            error = rep.error();
        else
            error = status.error_message();
        return false;
    }
    return true;
}
