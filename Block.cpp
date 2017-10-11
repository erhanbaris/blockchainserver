#include "Block.h"

Block::Block()
{
	Data = NULL;
	PreviousHash = NULL;
}

void Block::SetHash()
{
	auto miningResult = CalculateHash();
	Hash = std::string(miningResult.Hash);
	Nonce = miningResult.Nounce;
}

MineInfo Block::CalculateHash()
{
	std::string data = (PreviousHash == NULL ? "" : PreviousHash->Hash) + std::to_string(TimeStamp) + Data;
	MineInfo miningResult = MineHash(Index, data.c_str(), data.size());
	return miningResult;
}