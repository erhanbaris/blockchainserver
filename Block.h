#pragma once

#include <string>
#include <vector>
#include <sha256.h>
#include <Tools.h>
#include <json11.hpp>

struct Block
{
	std::size_t Index;
	std::string Hash;
	std::string PreviousHash;
	long int TimeStamp;
	std::size_t Nonce;
	std::string Data;

	Block();
	void SetHash();
	MineInfo CalculateHash();
	std::string Encode();

	static void Decode(std::string const& message, Block*& newBlock);
	static void Decode(std::string const& message, std::vector<Block*>& blocks);

	static void Decode(json11::Json const& json, Block*& newBlock);
	static void Decode(json11::Json const& json, std::vector<Block*>& blocks);
};
