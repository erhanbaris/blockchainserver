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

#include <HttpServer.h>

using namespace std;

inline static bool isInvalidParameter(int index, int argc, char *argv[])
{
	return (index + 2) > argc || !isInteger(argv[index + 1]);
}

uv_loop_t* loop = NULL;
size_t HTTP_PORT;
size_t TCP_PORT;

int main(int argc, char *argv[])
{
	TCP_PORT = TCP_SERVER_PORT;
	HTTP_PORT = HTTP_SERVER_PORT;

	for (size_t i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "--http-port") == 0)
		{
			if (isInvalidParameter(i, argc, argv))
			{
				ALERT << "Http server port is invalid.";
				return 1;
			}
			HTTP_PORT = (size_t) stoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "--tcp-port") == 0)
		{
			if (isInvalidParameter(i, argc, argv))
			{
				ALERT << "Tcp server port is invalid.";
				return 1;
			}
			TCP_PORT = (size_t) stoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			ALERT << " Blockchain Server setting parameters." << std::endl;
			ALERT << "  --http-port       Http server port. Default port: " << HTTP_SERVER_PORT;
			ALERT << "  --tcp-port  Tcp server port. Default port: " << TCP_SERVER_PORT << std::endl;
			return 0;
		}
	}

    loop = uv_default_loop();

    INFO << "Tcp Server Port : " << TCP_PORT;
    TcpServer* tcpServer = new TcpServerUv;
    tcpServer->Start(TCP_PORT);
    INFO << "Tcp Server Started";


    INFO << "Http Server Port : " << HTTP_PORT;
    BlockChainServer server;
	server.SetTcpServer(tcpServer);
    server.Start(HTTP_PORT);

	uv_run(loop, UV_RUN_DEFAULT);
    return 0;
}
