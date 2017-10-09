#include "Block.h"

Block::Block()
{
	Data = NULL;
	PreviousHash = NULL;
}

void Block::SetHash()
{
	std::string data = std::to_string(Index) + (PreviousHash == NULL ? "" : PreviousHash->Hash) + std::to_string(TimeStamp) + Data;

	SHA256 sha256;
	sha256.add(&data, data.size());
	Hash = CalculateHash();
}

std::string Block::CalculateHash()
{
	std::string data = std::to_string(Index) + (PreviousHash == NULL ? "" : PreviousHash->Hash) + std::to_string(TimeStamp) + Data;

	SHA256 sha256;
	sha256.add(&data, data.size());
	return sha256.getHash();
}