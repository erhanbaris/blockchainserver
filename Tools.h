#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <ctime>

#include <Config.h>

static long int getTimestamp()
{
	time_t t = std::time(0);
	long int now = static_cast<long int> (t);
	return now;
}

struct MineInfo {
	size_t Nounce;
	std::string Hash;
};

static MineInfo MineHash(size_t index, std::string & data)
{
	struct MineInfo result;

	SHA256 sha256;
	std::stringstream stream;

	for (size_t i = 0; i < MAX_NOUNCE; ++i)
	{
		stream.str(std::string());

		stream << index << i;
		std::string tmp = stream.str();
		sha256.reset();

		sha256.add(tmp.c_str(), tmp.size());
		sha256.add(data.c_str(), data.size());
		auto hash = sha256.getHash();
		if (hash.substr(0, DIFFICULTY) == NOUNCE_ZEROS)
		{
			result.Hash = hash;
			result.Nounce = i;
			break;
		}
	}

	return result;
}

inline static bool isInteger(char *str)
{
	return strlen(str) != 0 && strspn(str, "0123456789") == strlen(str);
}