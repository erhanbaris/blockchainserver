#include <Block.h>
#include <Base64.h>
#include <string>
#include <sstream>

Block::Block()
{
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
			<< "\"PreviousHash\":\"" << (PreviousHash == NULL ? "0000000000000000000000000000000" : PreviousHash->Hash) << "\","
			<< "\"TimeStamp\":" << TimeStamp << ","
			<< "\"Nonce\":" << Nonce << ","
			<< "\"Data\":\"" << encodedData << "\""
		<< "}";

	return stream.str();
}

Block* Decode(std::string)
{
	return NULL;
}