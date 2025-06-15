#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

namespace utils {

inline void setupLogging() {
    try {
        // Console sink with colors
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);
        console_sink->set_pattern("[%H:%M:%S] [%^%l%$] [%n] %v");

        // File sink with rotation
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "logs/game_of_life.log", 1024 * 1024 * 5, 3);
        file_sink->set_level(spdlog::level::debug);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v");

        std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
        
        // Create loggers
        auto game_logger = std::make_shared<spdlog::logger>("GameServer", sinks.begin(), sinks.end());
        auto network_logger = std::make_shared<spdlog::logger>("WebSocketServer", sinks.begin(), sinks.end());
        
        game_logger->set_level(spdlog::level::debug);
        network_logger->set_level(spdlog::level::debug);
        
        spdlog::register_logger(game_logger);
        spdlog::register_logger(network_logger);
        
        spdlog::set_default_logger(game_logger);
        spdlog::flush_every(std::chrono::seconds(1));
        
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    }
}

} // namespace utils
