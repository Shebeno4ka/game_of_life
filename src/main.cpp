#include "ws_controller.hpp"

int main(int argc, char* argv[]) {
  WSServer ws_server;
  ws_server.startListening("0.0.0.0", 8080);
}
