#pragma once

#include <string>
#include <functional>

class WebSocketClient;
typedef std::function<void (std::string const &, WebSocketClient&)> OnMessageCallback;
typedef std::function<void (WebSocketClient&)> OnDisconnectCallback;
typedef std::function<void (WebSocketClient&)> OnConnectCallback;
typedef std::function<void (WebSocketClient&)> OnCloseCallback;

class WebSocketClient {
public:
    virtual void Connect(std::string&) = 0;
    virtual void Disconnect() = 0;
    virtual bool IsConnected() = 0;
    virtual void Send(std::string&) = 0;
    virtual std::string GetRemoteAddress() = 0;

    virtual void SetOnMessage(OnMessageCallback) = 0;
    virtual void SetOnDisconnect(OnDisconnectCallback) = 0;
    virtual void SetOnConnect(OnConnectCallback) = 0;
    virtual void SetOnClose(OnCloseCallback) = 0;
};
