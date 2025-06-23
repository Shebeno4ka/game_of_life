#pragma once

#include "clients/IClient.h"
#include "clients/PassiveClient.h"

class PassiveClientGenerator {
private:
    std::chrono::seconds genInterval_;
public:
    explicit PassiveClientGenerator(std::chrono::seconds genInterval) : genInterval_(genInterval) {}

    std::unique_ptr<IClient> next(boost::asio::io_context& ioContext) {
        std::this_thread::sleep_for(genInterval_);
        auto client = std::make_unique<PassiveClient>(ioContext);
        return client;
    }
};