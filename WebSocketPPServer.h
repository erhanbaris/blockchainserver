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
	WebSocketServer::ConnectToBlockStatus ConnectToNode(std::string);
	void DisconnectFromNode(std::string);
	const std::vector<std::string> ConnectedNodes();
	void SetMessageReceived(OnMessageReceivedCallback cb);

private:
    WebSocketPPServerPimpl *pimpl;
};
