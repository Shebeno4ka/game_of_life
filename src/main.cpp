#include "game_runner.hpp"
#include "ws_controller.hpp"

int main(int argc, char* argv[]) {
  auto game_runner = std::make_shared<GameRunner>(GameState(50, 40), 8);
  WSServer ws_server(game_runner);
  ws_server.startListening("127.0.0.1", 8080);
}
