#ifndef BLOCKCHAINSERVER_H
#define BLOCKCHAINSERVER_H


#include <string>
#include <vector>

#include <sha256.h>

struct Block
{
        std::size_t Index;
        std::string Hash;
        Block* PreviousHash;
        std::size_t TimeStamp;
        char* Data;

        Block();
        void SetHash();
        std::string CalculateHash();
};

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

#endif // BLOCKCHAINSERVER_H
