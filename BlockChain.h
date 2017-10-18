#pragma once

#include <Block.h>
#include <string>
#include <vector>

class BlockChain {
public:
	enum class AddStatus { ADDED, BLOCK_IS_NEWER, BLOCK_IS_OLDER, SKIPPED, INVALID_BLOCK };

	BlockChain();
	Block* NewBlock(std::string&);
	Block* Get(size_t index);
	size_t TotalBlocks();
	std::string SerializeChain();
	std::string SerializeChain(size_t startBlock);
	Block* GetLastBlock();
	bool SetChain(std::vector<Block*>&);
	bool Merge(std::vector<Block*>&);
	AddStatus AddBlock(Block*);
	bool Validate(int index, std::string hash, std::string previousHash, int nonce, int timeStamp);

	static bool isValidNewBlock(Block * newBlock, Block * previousBlock);

private:
	std::vector<Block*> blocks;
};
