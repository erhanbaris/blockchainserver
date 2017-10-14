#pragma once

#include <WebSocketClient.h>
#include <string>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/client.hpp>

typedef websocketpp::client<websocketpp::config::asio> client;

struct WebSocketPPClientPimpl;
class WebSocketPPClient : public WebSocketClient {
public:
    WebSocketPPClient();
    void SetClient(client*);
    void Connect(std::string&);
    void Disconnect();
    bool IsConnected();
    void Send(std::string&);
    std::string GetRemoteAddress();

    void SetOnMessage(OnMessageCallback);
    void SetOnDisconnect(OnDisconnectCallback);
    void SetOnConnect(OnConnectCallback);
    void SetOnClose(OnCloseCallback);

private:
    WebSocketPPClientPimpl* pimpl;
};
