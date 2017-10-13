#pragma once
#include <vector>
#include <string>

#define HTTP_SERVER_PORT 4444
#define WEBSOCKET_SERVER_PORT 4445

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
#define ERROR std::cout << std::endl


struct Block;
enum class MessageType
{
	REQ_LAST_BLOCK = 0,
	REQ_FULL_BLOCKCHAIN = 1,
	REQ_PARTIAL_BLOCKCHAIN = 2,

	RES_LAST_BLOCK = 3,
	RES_FULL_BLOCKCHAIN = 4,
	RES_PARTIAL_BLOCKCHAIN = 5
};

/*struct NodeMessage {
	MessageType Type; 
	union Data {
		struct Block* Block;
		std::vector<struct Block*>* Blocks;
		size_t StartIndex;
	};
};*/