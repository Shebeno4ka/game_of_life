#pragma once

#include "IClient.h"
#include "client/Client.h"


// клиент, который не отправляет никаких запросов к серверу, а просто получает от него данные ничего не делая с ними
class PassiveClient : public IClient {
private:
    static constexpr const char* kName_ = "PassiveClient";
    Client::Loger logger_;
    Client client_;
    std::atomic<bool> running_{false};
public:
    explicit PassiveClient(boost::asio::io_context& ioContext) : client_(ioContext), logger_(spdlog::get("Client")) {
        if (!logger_) {
            logger_ = spdlog::default_logger();
        }
    }

    ~PassiveClient() override {
        assert(!running_.load() && "PassiveClient: клиент не был остановлен перед уничтожением");
    }

    void start(std::string address) override {
        bool expected = false;
        if (running_.compare_exchange_strong(expected, true)) {
            client_.connect(std::move(address));
        }
    }

    void stop() override {
        bool expected = true;
        if (running_.compare_exchange_strong(expected, false)) {
            client_.disconnect();
        }
    }

    std::string_view name() const override {return {kName_};}
};