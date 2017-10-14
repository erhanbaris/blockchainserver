#pragma once

#include <Block.h>
#include <vector>

class BlockChain {
public:
    BlockChain();
    Block* NewBlock(char const* data);
    size_t TotalBlocks();
    bool IsBlockChainValid();
	std::string SerializeChain();
	std::string SerializeChain(size_t startBlock);
	Block* GetLastBlock();
    void SetChain(std::vector<Block*>&);

private:
    Block* lastBlock;
    size_t totalBlocks;
    std::vector<Block*> blocks;

    bool isValidNewBlock(Block * newBlock, Block * previousBlock);
};
