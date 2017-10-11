#pragma once

#include <string>
#include <vector>

#include <sha256.h>

struct BlockChainServerPimpl;
class BlockChainServer
{
    public:
        BlockChainServer();
        ~BlockChainServer();

        void Start(size_t port);
        void Stop();

    private:
        BlockChainServerPimpl* pimpl;
};