#pragma once

#include <string>
#include <vector>
#include <sha256.h>
#include <Tools.h>

struct Block
{
	std::size_t Index;
	std::string Hash;
	Block* PreviousHash;
	long int TimeStamp;
	std::size_t Nonce;
	char* Data;

	Block();
	void SetHash();
	MineInfo CalculateHash();
	std::string Encode();
	static Block* Decode(std::string);
};