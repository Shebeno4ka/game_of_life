#pragma once
#include "core/GameEvent.h"

#include <functional>
#include <future>

namespace LifeGame::utils {

class SandNetworkDriver {
    using MessageCallback = std::function<void(std::vector<CellChange>)>;
    using FromClientData = std::vector<std::vector<CellChange>>;
    using FromServerData = std::vector<std::vector<std::byte>>;

    MessageCallback messageCallback_;
    FromClientData fromClientData_;
    FromServerData fromServerData_;

   public:
    class Handle {
        SandNetworkDriver* driver_;

    public:
        Handle() : driver_(nullptr) {}
        explicit Handle(SandNetworkDriver* driver) : driver_(driver) {}
        [[nodiscard]] const FromClientData& fromClientData() const {
            return driver_->fromClientData_;
        }
        [[nodiscard]] const FromServerData& fromServerData() const {
            return driver_->fromServerData_;
        }
        void sendToServer(std::vector<CellChange> cells) {
            this->driver_->sendToServer(std::move(cells));
        }
    };

    void start() const {}
    void stop() const {}

    void setMessageCallback(MessageCallback cb) {
        messageCallback_ = std::move(cb);
    }

    std::future<void> sendToAllClients(std::vector<std::byte> data, std::chrono::milliseconds timeout) {
        fromServerData_.emplace_back(std::move(data));
        return std::async(std::launch::deferred, []() {});
    }

    Handle getHandle() {
        return Handle(this);
    }

   private:
    void sendToServer(std::vector<CellChange> data) {
        fromClientData_.emplace_back(data);
        messageCallback_(std::move(data));
    }
};

} // namespace LifeGame::utils