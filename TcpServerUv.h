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
    TcpServer::ConnectToBlockStatus ConnectToNode(std::string);
    void DisconnectFromNode(std::string);
    const std::vector<std::string> ConnectedNodes();
    void SetMessageReceived(MessageReceivedCallback cb);

private:
    TcpServerUvPimpl *pimpl;
};
