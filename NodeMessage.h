#pragma once

#include <Config.h>
#include <Block.h>
#include <json11.hpp>
#include <string>

enum class MessageType
{
    REQ_LAST_BLOCK = 0,
    REQ_FULL_BLOCKCHAIN = 1,
    REQ_PARTIAL_BLOCKCHAIN = 2,
    REQ_NODE_LIST = 6,

    RES_LAST_BLOCK = 3,
    RES_FULL_BLOCKCHAIN = 4,
    RES_PARTIAL_BLOCKCHAIN = 5,
    RES_NODE_LIST = 7,

    REQ_NODE_INFO = 10,
    RES_NODE_INFO = 8,

    INVALID_MESSAGE = 9
};

struct NodeMessage {

    NodeMessage();
    NodeMessage(std::string const& message);
    ~NodeMessage();

    MessageType Type;

    struct Block* Block;
    std::vector<struct Block*>* Blocks;
    std::vector<std::string>* Nodes;
    size_t Index;
};
