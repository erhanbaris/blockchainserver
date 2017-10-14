#define ASIO_STANDALONE

#include <TcpClient.h>
#include <TcpClientUv.h>

#include <Config.h>

#include <stdlib.h>
#include <thread>

using namespace std;
using namespace blockchain::tcp;

struct TcpClientUvPimpl
{
    enum class AsyncType { UNDEFINED, SEND, CLOSE };

    std::string address;
    size_t port;
    bool isConnected;
    std::string sendMessage;

    TcpClient *tcpClient;
    AsyncType asyncType;

    // uv
    uv_async_t* asyncOperation;
    uv_async_t* closeAsync;
    uv_tcp_t* client;
    uv_connect_t* connect;

    // Callbacks
    TcpClient::MessageCallback messageCallback;
    TcpClient::DisconnectCallback disconnectCallback;
    TcpClient::ConnectCallback connectCallback;

    TcpClientUvPimpl()
    {
        asyncOperation = NULL;
        asyncType = AsyncType::UNDEFINED;
        client = NULL;
        connect = NULL;
        tcpClient = NULL;
        isConnected = false;
    }

    ~TcpClientUvPimpl()
    {
        if (asyncOperation != NULL)
            delete asyncOperation;

        if (client != NULL)
            delete client;

        if (connect != NULL)
            delete connect;
    }

    static void closeCb(uv_handle_t* handle)
    {
        TcpClientUvPimpl * pimpl = (TcpClientUvPimpl*)handle->data;

        if (pimpl->disconnectCallback)
            pimpl->disconnectCallback(*pimpl->tcpClient);
    }

    static void allocCb(uv_handle_t* handle, size_t size, uv_buf_t* buf) {
        *buf = uv_buf_init((char*)malloc(size), size);
    }

    static void readCb(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf)
    {
        TcpClientUvPimpl * pimpl = (TcpClientUvPimpl*)tcp->data;

        if(nread >= 0) {
            if (pimpl->messageCallback)
            {
                std::string message(buf->base, nread);
                pimpl->messageCallback(message, *pimpl->tcpClient);
            }
        }
        else {
            uv_close((uv_handle_t*)tcp, closeCb);
        }
    }

    static void afterSend(uv_write_t* handle, int status) {
        TcpClientUvPimpl * pimpl = (TcpClientUvPimpl*)handle->data;
        pimpl->sendMessage = "";
        delete handle->bufs;
    }

    static void async(uv_async_t *handle) {
        TcpClientUvPimpl * pimpl = (TcpClientUvPimpl*)handle->data;

        if (pimpl->asyncType == AsyncType::SEND) {
            uv_buf_t resbuf;
            resbuf.base = const_cast<char *>(pimpl->sendMessage.c_str());
            resbuf.len = pimpl->sendMessage.size();

            uv_write_t *write_req = new uv_write_t;
            write_req->data = pimpl;
            uv_write(write_req, (uv_stream_t *) pimpl->client, &resbuf, 1, afterSend);
        }
        else if (pimpl->asyncType == AsyncType::CLOSE) {
            uv_close((uv_handle_t*)handle, closeCb);
        }

        pimpl->asyncType = AsyncType::UNDEFINED;
    }

    static void onConnect(uv_connect_t* connection, int status)
    {
        if (status < 0) {
            return;
        }

        TcpClientUvPimpl * pimpl = (TcpClientUvPimpl*)connection->data;

        pimpl->asyncOperation = new uv_async_t;
        pimpl->asyncOperation->data = pimpl;
        pimpl->isConnected = true;

        uv_stream_t* stream = connection->handle;
        stream->data = pimpl;
        uv_async_init(loop, ((uv_async_t*)pimpl->asyncOperation), TcpClientUvPimpl::async);
        uv_read_start(stream, allocCb, readCb);

        if (pimpl->connectCallback)
            pimpl->connectCallback(*pimpl->tcpClient);
    }
};

void TcpClientUv::Connect(std::string address, size_t port)
{
    pimpl->client = new uv_tcp_t;
    pimpl->port = port;
    pimpl->address = address;

    uv_tcp_init(loop, pimpl->client);

    struct sockaddr_in dest;
    uv_ip4_addr(address.c_str(), (int) pimpl->port, &dest);

    pimpl->connect = new uv_connect_t;
    pimpl->connect->data = pimpl;
    uv_tcp_connect(pimpl->connect, pimpl->client, (const struct sockaddr*)&dest, TcpClientUvPimpl::onConnect);
}

void TcpClientUv::Disconnect()
{
    if (IsConnected())
    {
        pimpl->asyncType = TcpClientUvPimpl::AsyncType::CLOSE;
        uv_async_send(pimpl->asyncOperation);
    }
}

TcpClientUv::TcpClientUv()
{
    pimpl = new TcpClientUvPimpl;
    pimpl->tcpClient = this;
}

TcpClientUv::TcpClientUv(uv_stream_t* serverHandle)
{
    pimpl = new TcpClientUvPimpl;
    pimpl->tcpClient = this;
    pimpl->asyncOperation = new uv_async_t;
    pimpl->asyncOperation->data = pimpl;
    pimpl->isConnected = true;

    pimpl->client = new uv_tcp_t;
    pimpl->client->data = pimpl;

    uv_tcp_init(loop, pimpl->client);
    uv_accept(serverHandle, (uv_stream_t*)pimpl->client);

    uv_async_init(loop, ((uv_async_t*)pimpl->asyncOperation), TcpClientUvPimpl::async);
    uv_read_start((uv_stream_t*)pimpl->client, TcpClientUvPimpl::allocCb, TcpClientUvPimpl::readCb);
}

TcpClientUv::~TcpClientUv()
{
    delete pimpl;
}

std::string TcpClientUv::GetRemoteAddress()
{
    return pimpl->address;
}

size_t TcpClientUv::GetRemotePort()
{
    return pimpl->port;
}

bool TcpClientUv::IsConnected()
{
    return pimpl->isConnected;
}

void TcpClientUv::Send(std::string const&& message)
{
    pimpl->sendMessage = message;
    pimpl->asyncType = TcpClientUvPimpl::AsyncType::SEND;
    uv_async_send(pimpl->asyncOperation);
}

void TcpClientUv::SetOnConnect(ConnectCallback cb)
{
    pimpl->connectCallback = cb;
}

void TcpClientUv::SetOnMessage(MessageCallback cb)
{
    pimpl->messageCallback = cb;
}

void TcpClientUv::SetOnDisconnect(DisconnectCallback cb)
{
    pimpl->disconnectCallback = cb;
}
