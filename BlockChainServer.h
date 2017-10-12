#pragma once

#include <string>
#include <vector>

#include <sha256.h>
#include <WebSocketServer.h>

struct BlockChainServerPimpl;
class BlockChainServer
{
    public:
        BlockChainServer();
        ~BlockChainServer();

        void Start(size_t port);
        void Stop();
		void SetWebSocket(WebSocketServer*);

    private:
        BlockChainServerPimpl* pimpl;
};