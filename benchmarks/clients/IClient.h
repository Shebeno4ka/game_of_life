#pragma once

#include <string>

class IClient {
public:
    virtual void start(std::string serverAddress) = 0;
    virtual void stop() = 0;
    virtual ~IClient() = default;
    virtual std::string_view name() const = 0;
};