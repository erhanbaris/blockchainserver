#pragma once

#include <WebSocketServer.h>

#include <iostream>
#include <set>
#include <functional>

using namespace std;


struct WebSocketPPServerPimpl;
class WebSocketPPServer : public WebSocketServer {
public:
    void Init();

    void Start(size_t port);
    void Stop();
    size_t GetPort();
	void BroadcastBlock(Block*);
	void ConnectToNode(std::string);

private:
    WebSocketPPServerPimpl *pimpl;
};
