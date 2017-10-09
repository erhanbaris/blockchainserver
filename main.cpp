#include <iostream>

#include <blockchainserver.h>

using namespace std;

int main(int argc, char *argv[])
{
	BlockChainServer server;
	server.Start(4444);
    return 0;
}
