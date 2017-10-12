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
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

using namespace std;


inline static bool isInvalidParameter(int index, int argc, char *argv[])
{
	return (index + 2) > argc || !isInteger(argv[index + 1]);
}

int main(int argc, char *argv[])
{
	size_t webSocketPort = WEBSOCKET_SERVER_PORT;
	size_t httpPort = HTTP_SERVER_PORT;

	for (size_t i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "--http-port") == 0)
		{
			if (isInvalidParameter(i, argc, argv))
			{
				ERROR << "Http server port is invalid.";
				return 1;
			}
			httpPort = stoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "--websocket-port") == 0)
		{
			if (isInvalidParameter(i, argc, argv))
			{
				ERROR << "WebSocket server port is invalid.";
				return 1;
			}
			webSocketPort = stoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			ERROR << " Blockchain Server setting parameters." << std::endl;
			ERROR << "  --http-port       Http server port. Default port is " << HTTP_SERVER_PORT;
			ERROR << "  --websocket-port  Websocket server port. Default port is " << WEBSOCKET_SERVER_PORT << std::endl;
			return 0;
		}
	}

    INFO << "WebSocket Server Port : " << webSocketPort;
    WebSocketServer * socket = new WebSocketPPServer;
    socket->Init();
    socket->Start(webSocketPort);

    INFO << "Http Server Port : " << httpPort;
    BlockChainServer server;
	server.SetWebSocket(socket);
    server.Start(httpPort);


    return 0;
}
