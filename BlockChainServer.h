#pragma once

#include <string>
#include <vector>

#include <sha256.h>
#include <TcpServer.h>
using namespace blockchain::tcp;

struct BlockChainServerPimpl;
class BlockChainServer
{
    public:
        BlockChainServer();
        ~BlockChainServer();

        void Start(size_t port);
        void Stop();
		void SetTcpServer(TcpServer*);

    private:
        BlockChainServerPimpl* pimpl;
};