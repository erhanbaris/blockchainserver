#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <chrono>

static long getUtcMilliseconds()
{
	using namespace std::chrono;
	milliseconds::rep repTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	time_t t = time(0);
	struct tm lt = { 0 };

#ifdef _WIN32
	lt = *localtime(&t);
#else
	localtime_r(&t, &lt);
#endif
	return repTime;
}