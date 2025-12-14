//
// Created by sanek on 13/12/2025.
//

#include "BotClient.h"

BotClient::BotClient(const std::string& address) {
    channel_ = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
    stub_ = antdsb::BotService::NewStub(channel_);
}

bool BotClient::TryGetStatus(antdsb::StatusReply& out) {
    antdsb::StatusRequest req;
    grpc::ClientContext ctx;

    auto status = stub_->GetStatus(&ctx, req, &out);
    if (!status.ok()) {
        out.set_running(false);
        out.set_last_error(status.error_message());
        return false;
    }
    return true;
}

bool BotClient::SendMessage(uint64_t channel_id,
                            const std::string& text,
                            std::string& error) {
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

bool BotClient::StartBot(antdsb::StatusReply& out, std::string& error) {
    google::protobuf::Empty req;
    grpc::ClientContext ctx;

    auto status = stub_->StartBot(&ctx, req, &out);
    if (!status.ok()) {
        error = status.error_message();
        return false;
    }

    error.clear();
    return true;
}

bool BotClient::StopBot(antdsb::StatusReply& out, std::string& error) {
    google::protobuf::Empty req;
    grpc::ClientContext ctx;

    auto status = stub_->StopBot(&ctx, req, &out);
    if (!status.ok()) {
        error = status.error_message();
        return false;
    }

    error.clear();
    return true;
}

bool BotClient::RestartBot(antdsb::StatusReply& out, std::string& error) {
    google::protobuf::Empty req;
    grpc::ClientContext ctx;

    auto status = stub_->RestartBot(&ctx, req, &out);
    if (!status.ok()) {
        error = status.error_message();
        return false;
    }

    error.clear();
    return true;
}
