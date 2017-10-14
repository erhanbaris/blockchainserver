#include <Block.h>
#include <Base64.h>
#include <string>
#include <sstream>

Block::Block()
{
	PreviousHash = "0";
	PreviousBlock = NULL;
}

void Block::SetHash()
{
	auto miningResult = CalculateHash();
	Hash = std::string(miningResult.Hash);
	Nonce = miningResult.Nounce;
}

MineInfo Block::CalculateHash()
{
	std::string data = PreviousHash + std::to_string(TimeStamp) + Data;
	MineInfo miningResult = MineHash(Index, data);
	return miningResult;
}

std::string Block::Encode()
{
	std::stringstream stream;

	std::string encodedData;
	Base64::Encode(Data, &encodedData);
	stream << "{"
			<< "\"Index\":" << Index << ","
			<< "\"Hash\":\"" << Hash << "\","
			<< "\"PreviousHash\":\"" << PreviousHash << "\","
			<< "\"TimeStamp\":" << TimeStamp << ","
			<< "\"Nonce\":" << Nonce << ","
			<< "\"Data\":\"" << encodedData << "\""
		<< "}";

	return stream.str();
}

namespace
{
	static Block* fetchBlocksFromJson(json11::Json const& json)
	{
		Block* newBlock = new Block;
		newBlock->Hash = json["Hash"].string_value();
		newBlock->Index = (size_t) json["Index"].int_value();
		newBlock->Nonce = (size_t) json["Nonce"].int_value();
		newBlock->TimeStamp = json["TimeStamp"].int_value();
		newBlock->PreviousHash = json["PreviousHash"].string_value();
		auto data = json["Data"].string_value();

		if (!data.empty())
			Base64::Decode(data, &newBlock->Data);

		return newBlock;
	}
}

void Block::Decode(json11::Json const& json, Block* newBlock)
{
	newBlock = fetchBlocksFromJson(json);
}

void Block::Decode(std::string const& message, Block* newBlock)
{
	std::string err;
	const json11::Json json = json11::Json::parse(message, err);

	if (err.empty())
		Block::Decode(json, newBlock);
	else
		newBlock = NULL;
}

void Block::Decode(json11::Json const& json, Block* newBlock, size_t * totalBlocks)
{
	if (json.is_array())
	{
		auto jsonArray = json.array_items();
		auto end = jsonArray.end();
		for (auto it = jsonArray.begin(); it != end; ++it)
		{
			auto block = fetchBlocksFromJson(*it);
			block->PreviousBlock = newBlock;
			newBlock = block;
		}

		(*totalBlocks) = jsonArray.size();
	}
	else
	{
		newBlock = fetchBlocksFromJson(json);
		(*totalBlocks) = 1;
	}
}

void Block::Decode(std::string const& message, Block* newBlock, size_t * totalBlocks)
{
	std::string err;
	const json11::Json json = json11::Json::parse(message, err);

	if (err.empty())
		Block::Decode(json, newBlock, totalBlocks);
}