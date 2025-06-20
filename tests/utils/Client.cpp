#include "Client.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <spdlog/spdlog.h>
#include <boost/asio/ip/tcp.hpp>

using namespace boost::asio;
using namespace boost::beast;

Client::Client(boost::asio::io_context& ioContext) 
    : ioContext_(ioContext), resolver_(ioContext_), ws_(ioContext_), logger_(spdlog::get("Client")) {
    if (!logger_) {
        logger_ = spdlog::default_logger();
    }
}

Client::~Client() {
    disconnect();
}

void Client::connect(std::string address) {
    using boost::asio::ip::tcp;
    auto pos = address.find(':');
    if (pos == std::string::npos) {
        throw std::invalid_argument("Invalid address format");
    }

    std::string host = address.substr(0, pos);
    std::string port = address.substr(pos + 1);

    resolver_.async_resolve(host, port, [this, address](boost::system::error_code ec, auto results) {
        if (ec) {
            logger_->error("Resolve error: {}", ec.message());
            return;
        }

        boost::asio::async_connect(ws_.next_layer(), results, [this, address](boost::system::error_code ec, const tcp::endpoint& endpoint) {
            if (ec) {
                logger_->error("Connect error: {}", ec.message());
                return;
            }

            // В handshake нужно передавать имя хоста, а не IP
            ws_.async_handshake(address, "/", [this, address](boost::system::error_code ec) {
                if (ec) {
                    logger_->error("Handshake error: {}", ec.message());
                    return;
                }

                logger_->info("Connected to server at {}", address);
                doRead(); // Start reading after connection
            });
    });
});
}

void Client::disconnect() {
    ws_.async_close(websocket::close_code::normal, [this](boost::system::error_code ec) {
        if (ec) {
            logger_->warn("Error during disconnect: {}", ec.message());
        }
        logger_->info("Disconnected from server");
    });
}

void Client::send(std::vector<std::pair<uint32_t, uint32_t>> changes) {
    std::vector<std::byte> message;
    for (const auto& [x, y] : changes) {
        for (int i = 3; i >= 0; --i) { // Send x in little-endian order
            message.push_back(static_cast<std::byte>((x >> (i * 8)) & 0xFF));
        }
        for (int i = 3; i >= 0; --i) { // Send y in little-endian order
            message.push_back(static_cast<std::byte>((y >> (i * 8)) & 0xFF));
        }
    }

    ws_.async_write(buffer(message), [this, changes](boost::system::error_code ec, std::size_t bytes_transferred) {
        if (ec) {
            logger_->error("Write error: {}", ec.message());
            return;
        }
        logger_->info("Sent {} changes to server", changes.size());
    });
}

void Client::setCallback(OnMessageCallback cb) {
    callback_ = std::move(cb);
}

void Client::doRead() {
    ws_.async_read(buffer_, [this](boost::system::error_code ec, std::size_t bytes_transferred) {
        if (ec) {
            if (ec != websocket::error::closed) {
                logger_->warn("Read error: {}", ec.message());
            }
            return;
        }

        auto ptr = static_cast<std::byte*>(buffer_.data().data());
        std::vector<std::byte> data(ptr, ptr + buffer_.size());

        if (callback_) {
            callback_(logger_, std::move(data));
        }

        buffer_.consume(bytes_transferred); // Clear the buffer
        doRead(); // Continue reading
    });
}
