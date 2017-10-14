#include <iostream>

#include <Config.h>
#include <blockchainserver.h>
#include <string>
#include <sstream>
#include <Tools.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <TcpServer.h>
#include <TcpServerUv.h>

#include <TcpClient.h>
#include <TcpClientUv.h>

using namespace std;

inline static bool isInvalidParameter(int index, int argc, char *argv[])
{
	return (index + 2) > argc || !isInteger(argv[index + 1]);
}
uv_loop_t* loop = NULL;

int main(int argc, char *argv[])
{
	size_t webSocketPort = TCP_SERVER_PORT;
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
		else if (strcmp(argv[i], "--tcp-port") == 0)
		{
			if (isInvalidParameter(i, argc, argv))
			{
				ERROR << "Tcp server port is invalid.";
				return 1;
			}
			webSocketPort = stoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			ERROR << " Blockchain Server setting parameters." << std::endl;
			ERROR << "  --http-port       Http server port. Default port: " << HTTP_SERVER_PORT;
			ERROR << "  --tcp-port  Tcp server port. Default port: " << TCP_SERVER_PORT << std::endl;
			return 0;
		}
	}

    loop = uv_default_loop();

    TcpServer* tcpServer = new TcpServerUv;
    tcpServer->Start(webSocketPort);

    INFO << "Http Server Port : " << httpPort;
    BlockChainServer server;
	server.SetTcpServer(tcpServer);
    server.Start(httpPort);


    return 0;
}
