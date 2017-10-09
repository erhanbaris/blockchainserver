#pragma once

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