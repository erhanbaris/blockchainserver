#pragma once

#include <Block.h>

class WebSocketServer {
public:
    virtual void Init() = 0;

    virtual void Start(size_t port) = 0;
    virtual void Stop() = 0;
    virtual size_t GetPort() = 0;

	virtual void BroadcastBlock(Block*) = 0;
	virtual void ConnectToNode(std::string) = 0;
};
