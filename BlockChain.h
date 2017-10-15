#pragma once

#include <Block.h>
#include <vector>

class BlockChain {
public:
    enum class AddStatus { ADDED, BLOCK_IS_NEWER, BLOCK_IS_OLDER, SKIPPED, INVALID_BLOCK };
    
    BlockChain();
    Block* NewBlock(char const* data);
    Block* Get(size_t index);
    size_t TotalBlocks();
	std::string SerializeChain();
	std::string SerializeChain(size_t startBlock);
	Block* GetLastBlock();
    bool SetChain(std::vector<Block*>&);
    AddStatus AddBlock(Block*);
    
    static bool isValidNewBlock(Block * newBlock, Block * previousBlock);

private:
    Block* lastBlock;
    size_t totalBlocks;
    std::vector<Block*> blocks;
};
