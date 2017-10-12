#pragma once

#include <WebSocketClient.h>
#include <string>

struct WebSocketPPClientPimpl;
class WebSocketPPClient : public WebSocketClient {
public:
    WebSocketPPClient();
    void Connect(std::string&);
    void Disconnect();
    bool IsConnected();
    void Send(std::string&);
    std::string GetRemoteAddress();

    void setOnMessage(OnMessageCallback);
    void setOnDisconnect(OnDisconnectCallback);
    void setOnConnect(OnConnectCallback);
    void setOnClose(OnCloseCallback);

private:
    WebSocketPPClientPimpl* pimpl;
};
