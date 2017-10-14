#pragma once

#include <TcpClient.h>
#include <string>
#include <uv.h>

using namespace blockchain::tcp;

struct TcpClientUvPimpl;
class TcpClientUv : public TcpClient {
public:
    TcpClientUv();

    void Connect(std::string, size_t port);

    void Disconnect();

    bool IsConnected();

    void Send(std::string &);

    std::string GetRemoteAddress();

    void SetOnMessage(MessageCallback);

    void SetOnDisconnect(DisconnectCallback);

    void SetOnConnect(ConnectCallback);

    void SetOnClose(CloseCallback);

private:
    TcpClientUvPimpl *pimpl;
};