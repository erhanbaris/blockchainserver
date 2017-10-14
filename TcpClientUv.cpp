#define ASIO_STANDALONE

#include <TcpClient.h>
#include <TcpClientUv.h>

#include <Config.h>

#include <stdlib.h>
#include <thread>
#include <vector>

using namespace std;
using namespace blockchain::tcp;

struct TcpClientUvPimpl
{
    uv_async_t* sendAsync;
    uv_tcp_t* client;
    uv_connect_t* connect;

    TcpClientUvPimpl()
    {
        sendAsync = NULL;
        client = NULL;
        connect = NULL;
    }

    ~TcpClientUvPimpl()
    {

    }

    static void closeCb(uv_handle_t* handle)
    {
        printf("closed.");
    }

    static void allocCb(uv_handle_t* handle, size_t size, uv_buf_t* buf) {
        *buf = uv_buf_init((char*)malloc(size), size);
    }

    static void readCb(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf)
    {
        printf("on_read. %p\n",tcp);
        if(nread >= 0) {
            //printf("read: %s\n", tcp->data);
            printf("read: %s\n", buf->base);
        }
        else {
            //we got an EOF
            uv_close((uv_handle_t*)tcp, closeCb);
        }
    }

    static void onConnect(uv_connect_t* connection, int status)
    {
        if (status < 0) {
            printf("failed to connect\n"); return;
        }
        printf("connected. %p %d\n",connection, status);

        ((TcpClientUvPimpl*)connection->data)->sendAsync = new uv_async_t;

        uv_stream_t* stream = connection->handle;
        uv_read_start(stream, allocCb, readCb);
    }
};

void TcpClientUv::Connect(std::string address, size_t port)
{
    pimpl = new TcpClientUvPimpl;
    pimpl->client = new uv_tcp_t;
    uv_tcp_init(loop, pimpl->client);

    struct sockaddr_in dest;
    uv_ip4_addr(address.c_str(), port, &dest);

    uv_connect_t *pConn = new uv_connect_t;
    pConn->data = pimpl;
    uv_tcp_connect(pConn, pimpl->client, (const struct sockaddr*)&dest, TcpClientUvPimpl::onConnect);
}

void TcpClientUv::Disconnect()
{

}

TcpClientUv::TcpClientUv()
{
}

std::string TcpClientUv::GetRemoteAddress()
{
    return "";
}

bool TcpClientUv::IsConnected()
{
    return false;
}

void TcpClientUv::Send(std::string& message)
{
}

void TcpClientUv::SetOnConnect(ConnectCallback cb)
{
}

void TcpClientUv::SetOnMessage(MessageCallback cb)
{
}

void TcpClientUv::SetOnDisconnect(DisconnectCallback cb)
{
}

void TcpClientUv::SetOnClose(CloseCallback cb)
{
}
