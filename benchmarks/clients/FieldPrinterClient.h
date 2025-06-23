#pragma once
#include "IClient.h"
#include "client/Client.h"
#include "utils/DebugUtils.h"

// клиент, который переодически печатает в консоль поле полученное от сервера
template <uint32_t EveryNthPrint>
class FieldPrinterClient : public IClient {
private:
    static constexpr const char* kName_ = "FieldPrinterClient";
    Client::Loger logger_;
    Client client_;
    std::atomic<bool> running_{false};
public:
    explicit FieldPrinterClient(boost::asio::io_context& ioContext) : client_(ioContext), logger_(spdlog::get("Client")) {
        if (!logger_) {
            logger_ = spdlog::default_logger();
        }
        client_.setOnServerMessageCallback([this, step = 0](std::vector<std::byte> data) mutable {
            if (step % EveryNthPrint == 0) {
                logger_->trace("Received {} response: \n{}", step, stringField(data));
            }
            step++;
        });
    }

    ~FieldPrinterClient() override {
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