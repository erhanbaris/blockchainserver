#include <BlockChain.h>
#include <string>
#include <sstream>

BlockChain::BlockChain()
{
	// Genesis block
	Block* genesisBlock = new Block;
	genesisBlock->Index = 1;
	genesisBlock->Nonce = 1;
	genesisBlock->PreviousHash = NULL;
	genesisBlock->TimeStamp = 1507493846081;
	genesisBlock->Data = "Genesis block";
	genesisBlock->SetHash();

	++totalBlocks;
	lastBlock = genesisBlock;

	totalBlocks = 1;

	blocks.push_back(genesisBlock);
}

Block* BlockChain::NewBlock(char const * data)
{
	Block * block = new Block;
	block->Index = lastBlock->Index + 1;
	block->PreviousHash = lastBlock;
	block->TimeStamp = getTimestamp();
	block->Data = strdup(data);
	block->SetHash();

	++totalBlocks;
	lastBlock = block;
	blocks.push_back(block);

	return block;
}

size_t BlockChain::TotalBlocks()
{
	return totalBlocks;
}

bool BlockChain::isValidNewBlock(Block * newBlock, Block * previousBlock)
{
	if (previousBlock == NULL)
		return true;

	if (previousBlock->Index + 1 != newBlock->Index)
		return false;
	else if (previousBlock->Hash != newBlock->PreviousHash->Hash)
		return false;
	else if (newBlock->CalculateHash().Hash != newBlock->Hash)
		return false;
	return true;
}


bool BlockChain::IsBlockChainValid() {
	auto end = blocks.rend();
	std::vector<Block*>::reverse_iterator it = blocks.rbegin();

	for (; it != end; ++it)
		if (isValidNewBlock(*it, (*it)->PreviousHash) == false)
			return false;

	return true;
}


std::string BlockChain::SerializeChain()
{
	std::stringstream stream;
	auto end = blocks.end();
	auto it = blocks.begin();

	stream << "[";
	if (end != it)
	{
		stream << (*it)->Encode();
		++it;

		while (it != end)
		{
			stream << "," << (*it)->Encode();
			++it;
		}
	}

	stream << "]";

	return stream.str();
}