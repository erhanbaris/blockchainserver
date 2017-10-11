#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_

#include <iostream>

#include <Config.h>
#include <blockchainserver.h>
#include <WebSocketServer.h>
#include <WebSocketPPServer.h>
#include <string>
#include <sstream>
#include <Tools.h>

using namespace std;

int main(int argc, char *argv[])
{
    INFO << "WebSocket Server Port : " << WEBSOCKET_SERVER_PORT;
	auto result = MineHash(1, "merhaba dünya", strlen("merhaba dünya"));
    WebSocketServer * socket = new WebSocketPPServer;
    socket->Init();
    socket->Start(WEBSOCKET_SERVER_PORT);

    INFO << "Http Server Port : " << HTTP_SERVER_PORT;
    BlockChainServer server;
    server.Start(HTTP_SERVER_PORT);


    return 0;
}
