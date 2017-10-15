#include <BlockChain.h>
#include <string>
#include <sstream>

BlockChain::BlockChain()
{
    // Genesis block
    Block* genesisBlock = new Block;
    genesisBlock->Index = 1;
    genesisBlock->Nonce = 1;
    genesisBlock->PreviousHash = "8bb60170a7a13686c3c651dac9038ce96eed1ff208cd5e29b4b16bbfec5c9c20";
    genesisBlock->TimeStamp = 1508090809;
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
    block->PreviousHash = lastBlock->Hash;
    block->TimeStamp = getTimestamp();
    block->Data = strdup(data);
    block->SetHash();
    
    ++totalBlocks;
    lastBlock = block;
    blocks.push_back(block);
    
    return block;
}

Block* BlockChain::Get(size_t index)
{
    if (index > blocks.size())
        return NULL;
    
    return blocks[index - 1];
}

bool BlockChain::SetChain(std::vector<Block*>& newBlocks)
{
    auto blocksEnd = blocks.end();
    for(auto it = blocks.begin(); it != blocksEnd; ++it)
        delete *it;
    
    blocks.clear();
    
    auto blocksBegin = blocks.begin();
    blocksEnd = newBlocks.end();
    
    Block* previousBlock = NULL;
    
    for (auto it = newBlocks.begin(); it != blocksEnd; ++it)
    {
        if (!isValidNewBlock((*it), previousBlock))
        {
            INFO << "Block chain is invalid.";
            return false;
        }
        else
            previousBlock = *it;
    }
    
    
    for (auto it = newBlocks.begin(); it != blocksEnd; ++it)
        blocks.push_back(*it);
    
    totalBlocks = blocks.size();
    return true;
}

bool BlockChain::Merge(std::vector<Block*>& newBlocks)
{
    Block* previousBlock = GetLastBlock();
    auto blocksEnd = newBlocks.end();
    
    for (auto it = newBlocks.begin(); it != blocksEnd; ++it)
    {
        if (!isValidNewBlock((*it), previousBlock))
        {
            INFO << "Blockchain is invalid.";
            return false;
        }
        else
        {
            blocks.push_back(*it);
            previousBlock = *it;
        }
    }
    
    INFO << "Blockchain merged" << std::endl;
    totalBlocks = blocks.size();
    return true;
}

size_t BlockChain::TotalBlocks()
{
    return totalBlocks;
}

bool BlockChain::isValidNewBlock(Block * newBlock, Block * previousBlock)
{
    if (previousBlock == NULL)
        return newBlock->CalculateHash().Hash == newBlock->Hash;
    
    if (previousBlock->Index + 1 != newBlock->Index)
        return false;
    else if (previousBlock->Hash != newBlock->PreviousHash)
        return false;
    else if (newBlock->CalculateHash().Hash != newBlock->Hash)
        return false;
    return true;
}

std::string BlockChain::SerializeChain(size_t startBlock)
{
    std::stringstream stream;
    auto end = blocks.end();
    auto it = blocks.begin() + startBlock;
    
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

BlockChain::AddStatus BlockChain::AddBlock(Block* block)
{
    if (block == NULL)
        return AddStatus::INVALID_BLOCK;
    
    if (blocks.size() == block->Index - 1)
    {
        if (isValidNewBlock(block, Get(block->Index - 1)))
        {
            ++totalBlocks;
            blocks.push_back(block);
            return AddStatus::ADDED;
        }
        else
            return AddStatus::INVALID_BLOCK;
    }
    else if (blocks.size() > block->Index)
    {
        return AddStatus::BLOCK_IS_OLDER;
    }
    else if (blocks.size() < block->Index)
    {
        return AddStatus::BLOCK_IS_NEWER;
    }
    
    return AddStatus::SKIPPED;
}

std::string BlockChain::SerializeChain()
{
    return SerializeChain(0);
}

Block* BlockChain::GetLastBlock()
{
    return lastBlock;
}
