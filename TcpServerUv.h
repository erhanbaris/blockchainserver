#pragma once

#include <TcpServer.h>
#include <uv.h>

#include <iostream>
#include <set>
#include <functional>

using namespace std;
using namespace blockchain::tcp;

struct TcpServerUvPimpl;
class TcpServerUv : public TcpServer {
public:
    TcpServerUv();
	void Start(size_t port);
	void Stop();
    size_t GetPort();
    void BroadcastMessage(std::string const&);
    void BroadcastMessageExpect(std::string const &, TcpClient&);
    TcpClient* CreateClient();
    
    void SetMessageReceived(MessageReceivedCallback);
	void SetClientConnected(ClientConnectedCallback);
	void SetClientDisconnected(ClientDisconnectedCallback);
    
private:
    TcpServerUvPimpl *pimpl;
};
