#pragma once

#include <TcpClient.h>
#include <string>
#include <uv.h>

using namespace blockchain::tcp;

struct TcpClientUvPimpl;
class TcpClientUv : public TcpClient {
public:
    TcpClientUv();
    TcpClientUv(uv_stream_t*);
    ~TcpClientUv();

    void Connect(std::string, size_t port);
    void Disconnect();
    bool IsConnected();
    void Send(std::string const &&);
	void SendAndClose(std::string const &&);
    std::string GetRemoteAddress();
    size_t GetRemotePort();
    
    void SetOnMessage(MessageCallback);
    void SetOnDisconnect(DisconnectCallback);
    void SetOnConnect(ConnectCallback);

private:
    TcpClientUvPimpl *pimpl;
};
