#define ASIO_STANDALONE

#include <TcpServer.h>
#include <TcpServerUv.h>

#include <TcpClient.h>
#include <TcpClientUv.h>

#include <Config.h>

#include <vector>

#include <json11.hpp>
#include <thread>

using namespace blockchain::tcp;

struct TcpServerUvPimpl
{
    size_t port;
    thread* mainThread;

    // LibUv things
    uv_tcp_t* tcpServer;
    
    TcpServerUvPimpl()
    {
        mainThread = NULL;
        tcpServer = new uv_tcp_t;
    }

    ~TcpServerUvPimpl()
    {

    }
    
    void Start(){
        INFO << "Thread Started";
        uv_tcp_init(loop, tcpServer);
        struct sockaddr_in address;
        uv_ip4_addr("0.0.0.0", port, &address);
        uv_tcp_bind(tcpServer, (const struct sockaddr*)&address, 0);
        uv_listen((uv_stream_t*)tcpServer, 1000, onConnect);
    }

    static void allocCb(uv_handle_t * /*handle*/, size_t suggested_size, uv_buf_t* buf) {
        *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
    }

    static void onRead(uv_stream_t* tcp, ssize_t nread, const uv_buf_t * buf) {
        /*ssize_t parsed;
        if (nread >= 0) {
            parsed = (ssize_t)http_parser_execute((http_parser*)client->Parser, parser_settings, buf->base, nread);

            if (((http_parser*)client->Parser)->upgrade) {
                uv_close((uv_handle_t*)client->Handle, HttpClient::onClose);
            }
            else if (parsed < nread) {
                uv_close((uv_handle_t*)client->Handle, HttpClient::onClose);
            }
        }
        else {
            if (nread != UV_EOF) {
            }
            //uv_close((uv_handle_t*)client->Handle, HttpClient::on_close);
        }
        free(buf->base);*/
    }

    static void onConnect(uv_stream_t* server_handle, int status) {
        /*uv_tcp_init(loop, (uv_tcp_t*)client->Handle);
        uv_async_init(loop, ((uv_async_t*)client->Async), HttpClient::sendAsync);

        ((uv_tcp_t*)client->Handle)->data = client;
        ((uv_async_t*)client->Async)->data = client;

        int r = uv_accept(server_handle, (uv_stream_t*)client->Handle);

        uv_read_start((uv_stream_t*)client->Handle, allocCb, onRead);*/
        INFO << "asdasd";
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

}


TcpServer::ConnectToBlockStatus TcpServerUv::ConnectToNode(std::string address)
{
    return TcpServer::ConnectToBlockStatus::ADDED;
}

void TcpServerUv::SetMessageReceived(MessageReceivedCallback cb)
{
}

void TcpServerUv::DisconnectFromNode(std::string)
{

}

const std::vector<std::string> TcpServerUv::ConnectedNodes()
{
    std::vector<std::string> nodes;
    return nodes;
}

void TcpServerUv::Stop()
{

}
