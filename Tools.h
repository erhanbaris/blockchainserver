#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <chrono>

#include <Config.h>

static long int getTimestamp()
{
	time_t t = std::time(0);
	long int now = static_cast<long int> (t);
	return now;
}


struct MineInfo {
	size_t Nounce;
	char * Hash;
};

static MineInfo MineHash(size_t index, char const * data, size_t length)
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
		sha256.add(data, length);
		auto hash = sha256.getHash();
		if (hash.substr(0, DIFFICULTY) == NOUNCE_ZEROS)
		{
			result.Hash = strdup(hash.c_str());
			result.Nounce = i;
			break;
		}
	}

	return result;
}