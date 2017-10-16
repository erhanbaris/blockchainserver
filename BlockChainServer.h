#pragma once

#include <string>
#include <vector>

#include <sha256.h>
#include <TcpServer.h>
#include <sstream>
#include <iostream>

using namespace blockchain::tcp;


typedef std::function<void(std::string&, std::stringstream &)> OperationCallback;

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