#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <variant>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include "game_event.hpp"

namespace beast = boost::beast;          // from <boost/beast.hpp>
namespace http = beast::http;            // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;  // from <boost/beast/websocket.hpp>
namespace net = boost::asio;             // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;        // from <boost/asio/ip/tcp.hpp>

class WSServer {
  std::vector<std::shared_ptr<websocket::stream<tcp::socket>>> clients_;

 public:
  WSServer() = default;
  ~WSServer() = default;

 public:
  void startListening(std::string&& address_str = "127.0.0.1",
                      unsigned short port = 8080) {
    try {
      net::ip::address address = net::ip::make_address(address_str);

      net::io_context ioc;

      // The acceptor receives incoming connections
      tcp::acceptor acceptor{ioc, {address, port}};

      // Set socket options to allow reuse of address
      acceptor.set_option(net::socket_base::reuse_address(true));

      std::cout << "WebSocket server listening on " << address << ":" << port
                << std::endl;

      for (;;) {
        tcp::socket socket{ioc};
        acceptor.accept(socket);
        std::thread(&WSServer::do_session, this, std::move(socket)).detach();
      }
    } catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << std::endl;
    }
  }

 private:
  // Parse incoming message to GameEvent
  std::optional<GameEvent> parseMessage(const std::string& message) {
    if (message == "pause") {
      return PauseEvent{};
    } else if (message == "unpause") {
      return UnPauseEvent{};
    } else {
      // Try to parse coordinates "x,y"
      std::istringstream iss(message);
      std::string x_str, y_str;

      if (std::getline(iss, x_str, ',') && std::getline(iss, y_str)) {
        try {
          uint32_t x = std::stoul(x_str);
          uint32_t y = std::stoul(y_str);
          return AddCellEvent{x, y};
        } catch (const std::exception&) {
          // Invalid format
          return std::nullopt;
        }
      }
    }

    return std::nullopt;
  }

  // Echoes back all received WebSocket messages
  void do_session(tcp::socket socket) {
    try {
      // Construct the stream by moving in the socket
      websocket::stream<tcp::socket> ws{std::move(socket)};

      // Set a decorator to change the Server of the handshake
      ws.set_option(
          websocket::stream_base::decorator([](websocket::response_type& res) {
            res.set(http::field::server,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-server-sync");
          }));

      // Accept the websocket handshake
      ws.accept();

      for (;;) {
        // This buffer will hold the incoming message
        beast::flat_buffer buffer;

        // Read a message
        ws.read(buffer);

        // Convert buffer to string
        std::string message = beast::buffers_to_string(buffer.data());

        // Parse message to GameEvent
        auto event = parseMessage(message);
        if (event.has_value()) {
          // Process the event
          processGameEvent(event.value());

          // Send acknowledgment or game state update
          std::string response = "Event processed: " + message;
          ws.text(true);
          ws.write(net::buffer(response));
        } else {
          // Invalid message format
          std::string error = "Invalid message format: " + message;
          ws.text(true);
          ws.write(net::buffer(error));
        }
      }
    } catch (beast::system_error const& se) {
      // This indicates that the session was closed
      if (se.code() != websocket::error::closed)
        std::cerr << "Error: " << se.code().message() << std::endl;
    } catch (std::exception const& e) {
      std::cerr << "Error: " << e.what() << std::endl;
    }
  }

  // Process GameEvent
  void processGameEvent(const GameEvent& event) {
    std::visit(
        [](const auto& e) {
          using T = std::decay_t<decltype(e)>;
          if constexpr (std::is_same_v<T, AddCellEvent>) {
            std::cout << "AddCell event: x=" << e.x << ", y=" << e.y
                      << std::endl;
            // TODO: Add cell to game state
          } else if constexpr (std::is_same_v<T, PauseEvent>) {
            std::cout << "Pause event received" << std::endl;
            // TODO: Pause game
          } else if constexpr (std::is_same_v<T, UnPauseEvent>) {
            std::cout << "Unpause event received" << std::endl;
            // TODO: Unpause game
          }
        },
        event);
  }

  void onMessage(const std::string& message);
};