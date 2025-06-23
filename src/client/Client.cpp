#include "Client.h"
#include "utils/DebugUtils.h"

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <spdlog/spdlog.h>
#include <boost/asio/ip/tcp.hpp>

using namespace boost::asio;
using namespace boost::beast;

Client::Client(io_context& ioContext)
    : ioContext_(ioContext), resolver_(ioContext_), ws_(ioContext_), logger_(spdlog::get("Client")) {
    if (!logger_) {
        logger_ = spdlog::default_logger();
    }
}

Client::~Client() {
    assert(isClosed_.load() && "Client: соединение не было остановлено перед уничтожением");
    assert(ioContext_.stopped() && "Client: io_context не был остановлен перед уничтожением");
}

void Client::connect(std::string address) {
    handleResolve(std::move(address));
}

void Client::disconnect() {
    if (isClosed_.load()) {
        return;
    }
    ws_.async_close(websocket::close_code::normal, [this](boost::system::error_code ec) {
        if (ec) {
            logger_->warn("Error during disconnect: {}", ec.message());
        }
        isClosed_.store(true);
        logger_->info("Disconnected from server");
    });
}

void Client::send(std::vector<std::pair<uint32_t, uint32_t>> changes) {
    auto message = new std::vector<std::byte>;
    message->reserve(changes.size() * sizeof(uint32_t) * 2);
    for (const auto& [x, y] : changes) {
        for (int i = 0; i <= 3; ++i) { // little-endian
            message->push_back(static_cast<std::byte>((x >> (i * 8)) & 0xFF));
        }
        for (int i = 0; i <= 3; ++i) { // little-endian
            message->push_back(static_cast<std::byte>((y >> (i * 8)) & 0xFF));
        }
    }

    logger_->debug("Send updates: {}", bytesToBitString(*message));

    ws_.async_write(
        boost::asio::buffer(*message),
        [this, changes = std::move(changes), message](boost::system::error_code ec, std::size_t bytes_transferred) mutable {
            delete message;
            if (ec) {
                logger_->error("Write error: {}", ec.message());
                return;
            }
            logger_->info("Sent {} changes to server", changes.size());
        }
    );
}

void Client::setOnServerMessageCallback(OnMessageCallback cb) {
    callback_ = std::move(cb);
}

void Client::doRead() {
    ws_.async_read(buffer_, [this](boost::system::error_code ec, std::size_t bytes_transferred) mutable  {
        if (ec) {
            if (ec != websocket::error::closed) {
                logger_->warn("Read error: {}", ec.message());
            }
            return;
        }

        auto ptr = static_cast<std::byte*>(buffer_.data().data());
        std::vector<std::byte> data(ptr, ptr + buffer_.size());

        if (callback_) {
            callback_(std::move(data));
        }

        buffer_.consume(bytes_transferred); // Clear the buffer
        doRead();                           // Continue reading
    });
}

void Client::handleResolve(std::string address) {
    using boost::asio::ip::tcp;
    auto pos = address.find(':');
    if (pos == std::string::npos) {
        throw std::invalid_argument("Invalid address format");
    }

    std::string host = address.substr(0, pos);
    std::string port = address.substr(pos + 1);
    resolver_.async_resolve(host, port,
        [this, address = std::move(address)](ErrorCode ec, auto results) mutable  {
            if (ec) {
                logger_->error("Resolve error: {}", ec.message());
                return;
            }
            handleConnect(std::move(address), std::move(results));
        }
    );
}

void Client::handleConnect(std::string address, ResolveResults results) {
    using ip::tcp;
    async_connect(ws_.next_layer(), results,
       [this, address = std::move(address)](ErrorCode ec, const tcp::endpoint& endpoint) mutable  {
           if (ec) {
               logger_->error("Connect error: {}", ec.message());
               return;
           }
           handleHandshake(std::move(address));
       }
    );
}

void Client::handleHandshake(std::string address) {
    ws_.async_handshake(address, "/",
    [this, address=std::move(address)](ErrorCode ec) mutable  {
            if (ec) {
                logger_->error("Handshake error: {}", ec.message());
                return;
            }
            isClosed_.store(false);
            logger_->info("Connected to server at {}", address);
            doRead();
        }
    );
}
