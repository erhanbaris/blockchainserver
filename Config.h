#pragma once
#include <vector>
#include <string>

#define HTTP_SERVER_PORT 8080
#define TCP_SERVER_PORT 8081

#ifdef MINING_ACTIVE
	#define DIFFICULTY 4	
	#define NOUNCE_ZEROS "0000"
	#define MAX_NOUNCE 999999999999
#else 
	#define DIFFICULTY 0	
	#define NOUNCE_ZEROS ""
	#define MAX_NOUNCE 1
#endif

#define INFO std::cout << std::endl
#define ALERT std::cout << std::endl

#include <uv.h>

extern uv_loop_t* loop;
extern size_t HTTP_PORT;
extern size_t TCP_PORT;
