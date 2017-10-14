#pragma once

#include <vector>
#include <string>
#include <Block.h>
#include <WebSocketClient.h>

typedef std::function<void(std::string const&, WebSocketClient&)> OnMessageReceivedCallback;

class WebSocketServer {
public:

	enum class ConnectToBlockStatus { ADDED = 0, ALREADY_ADDED = 1 };

    virtual void Init() = 0;

    virtual void Start(size_t port) = 0;
    virtual void Stop() = 0;
    virtual size_t GetPort() = 0;

	virtual void BroadcastMessage(std::string const&) = 0;
	virtual ConnectToBlockStatus ConnectToNode(std::string) = 0;
	virtual void DisconnectFromNode(std::string) = 0;
	virtual const std::vector<std::string> ConnectedNodes() = 0;
	virtual void SetMessageReceived(OnMessageReceivedCallback cb) = 0;
};
