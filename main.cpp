#include <iostream>

#include <blockchainserver.h>
#include <WebSocketServer.h>
#include <WebSocketPPServer.h>

using namespace std;

int main(int argc, char *argv[])
{
    WebSocketServer * socket = new WebSocketPPServer;
    socket->Init();
    socket->Start(4445);

	BlockChainServer server;
	server.Start(4444);

    return 0;
}
