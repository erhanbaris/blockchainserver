#include <Config.h>

#include <TcpServer.h>
#include <TcpServerUv.h>

#include <TcpClient.h>
#include <TcpClientUv.h>

#include <vector>
#include <thread>

#include <json11.hpp>

using namespace blockchain::tcp;

struct TcpServerUvPimpl
{
    size_t port;
    std::vector<TcpClient*> clients;

    // uv
    uv_tcp_t* tcpServer;

    // callbacks
    TcpServer::MessageReceivedCallback messageReceivedCallback;
    TcpServer::ClientConnectedCallback clientConnectedCallback;
	TcpServer::ClientDisconnectedCallback clientDisconnectedCallback;

    TcpServerUvPimpl()
    {
        tcpServer = new uv_tcp_t;
        tcpServer->data = this;
    }

    ~TcpServerUvPimpl()
    {

    }
    
    void Start(){
        uv_tcp_init(loop, tcpServer);
        struct sockaddr_in address;
        uv_ip4_addr("0.0.0.0", port, &address);
        uv_tcp_bind(tcpServer, (const struct sockaddr*)&address, 0);
        uv_listen((uv_stream_t*)tcpServer, 1000, onConnect);
    }

    void onMessage(std::string const& message, TcpClient& client)
    {
        if (messageReceivedCallback)
            messageReceivedCallback(message, client);
    }

    void onDisconnect(TcpClient& client)
    {
		if (clientDisconnectedCallback)
			clientDisconnectedCallback(client);
    }

    static void onConnect(uv_stream_t* serverHandle, int status) {
        TcpServerUvPimpl* pimpl = (TcpServerUvPimpl*) serverHandle->data;

        TcpClient* tcpClient = new TcpClientUv(serverHandle);
        tcpClient->SetOnMessage(std::bind(&TcpServerUvPimpl::onMessage, pimpl, std::placeholders::_1, std::placeholders::_2));

        if (pimpl->clientConnectedCallback)
            pimpl->clientConnectedCallback(*tcpClient);
        
        pimpl->clients.push_back(tcpClient);
    }
};

TcpServerUv::TcpServerUv() {
    pimpl = new TcpServerUvPimpl;
}

void TcpServerUv::Start(size_t port) {
    pimpl->port = port;
    pimpl->Start();
}

size_t TcpServerUv::GetPort()
{
    return pimpl->port;
}

void TcpServerUv::BroadcastMessage(std::string const& message)
{
    auto end = pimpl->clients.end();
    for(auto it = pimpl->clients.begin(); it != end; ++it)
        (*it)->Send(std::move(message));
}

void TcpServerUv::BroadcastMessageExpect(std::string const &message, TcpClient& tcpClient)
{
    auto end = pimpl->clients.end();
    for(auto it = pimpl->clients.begin(); it != end; ++it)
        if (tcpClient.GetRemotePort() != (*it)->GetRemotePort() && tcpClient.GetRemoteAddress() != (*it)->GetRemoteAddress())
            (*it)->Send(std::move(message));
}

void TcpServerUv::SetMessageReceived(MessageReceivedCallback cb)
{
    pimpl->messageReceivedCallback = cb;
}

void TcpServerUv::SetClientConnected(ClientConnectedCallback cb)
{
	pimpl->clientConnectedCallback = cb;
}
void TcpServerUv::SetClientDisconnected(ClientDisconnectedCallback cb)
{
	pimpl->clientDisconnectedCallback = cb;
}

void TcpServerUv::Stop()
{

}

TcpClient *TcpServerUv::CreateClient() {
    return new TcpClientUv();
}
