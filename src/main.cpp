#include "server/GameServer.h"

int main(int argc, char* argv[]) {
    LifeGame::GameServer server;
    server.start();
    server.stop();
    return 0;
}
