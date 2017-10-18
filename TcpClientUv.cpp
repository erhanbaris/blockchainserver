#include <TcpClient.h>
#include <TcpClientUv.h>

#include <Config.h>

#include <stdlib.h>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <Tools.h>

using namespace std;
using namespace blockchain::tcp;

namespace {
	static char* _strndup(char *str, int chars)
	{
		char *buffer;
		int n;

		buffer = (char *)malloc(chars + 1);
		if (buffer)
		{
			for (n = 0; ((n < chars) && (str[n] != 0)); n++) buffer[n] = str[n];
			buffer[n] = 0;
		}

		return buffer;
	}
}

struct TcpClientUvPimpl
{
	enum class AsyncType { UNDEFINED, SEND, CLOSE, SEND_AND_CLOSE };

	std::string address;
	size_t port;
	bool isConnected;
	std::string sendMessage;
	bool largeBufferStarted;
	std::stringstream tmpBuffer;

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
			pimpl->disconnectCallback(pimpl->tcpClient);
	}

	static void allocCb(uv_handle_t* handle, size_t size, uv_buf_t* buf) {
		*buf = uv_buf_init((char*)malloc(size), size);
	}

	static void readCb(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf)
	{
		TcpClientUvPimpl * pimpl = (TcpClientUvPimpl*)tcp->data;

		if (nread >= 0) {
			char* tmpData = _strndup(buf->base, nread);
			pimpl->tmpBuffer << tmpData;

			if ((size_t)nread == buf->len && 65536 == nread)
				pimpl->largeBufferStarted = true;

			else if (pimpl->largeBufferStarted && (size_t)nread != buf->len)
				pimpl->largeBufferStarted = false;

			if (!pimpl->largeBufferStarted && pimpl->messageCallback)
			{
				auto message = pimpl->tmpBuffer.str();
				pimpl->messageCallback(message, pimpl->tcpClient);
				pimpl->tmpBuffer.str(std::string());
			}

			delete tmpData;
		}
		else {
			uv_close((uv_handle_t*)tcp, closeCb);
		}

		free(buf->base);
	}

	static void afterSend(uv_write_t* handle, int status) {
		TcpClientUvPimpl * pimpl = (TcpClientUvPimpl*)handle->data;
		pimpl->sendMessage = "";
	}

	static void afterSendAndClose(uv_write_t* handle, int status) {
		TcpClientUvPimpl * pimpl = (TcpClientUvPimpl*)handle->data;
		pimpl->sendMessage = "";
		uv_close((uv_handle_t*)pimpl->client, closeCb);
	}

	static void async(uv_async_t *handle) {
		TcpClientUvPimpl * pimpl = (TcpClientUvPimpl*)handle->data;

		if (pimpl->asyncType == AsyncType::SEND ||
			pimpl->asyncType == AsyncType::SEND_AND_CLOSE) {
			//pimpl->sendMessage.append("\n");

			uv_buf_t resbuf;
			resbuf.base = const_cast<char *>(pimpl->sendMessage.c_str());
			resbuf.len = pimpl->sendMessage.size();

			uv_write_t *write_req = new uv_write_t;
			write_req->data = pimpl;

			if (pimpl->asyncType == AsyncType::SEND_AND_CLOSE)
				uv_write(write_req, (uv_stream_t *)pimpl->client, &resbuf, 1, afterSendAndClose);
			else
				uv_write(write_req, (uv_stream_t *)pimpl->client, &resbuf, 1, afterSend);
		}

		if (pimpl->asyncType == AsyncType::CLOSE) {
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
			pimpl->connectCallback(pimpl->tcpClient);
	}
};

void TcpClientUv::Connect(std::string address, size_t port)
{
	pimpl->client = new uv_tcp_t;
	pimpl->port = port;
	pimpl->address = address;

	uv_tcp_init(loop, pimpl->client);

	struct sockaddr_in dest;
	uv_ip4_addr(address.c_str(), (int)pimpl->port, &dest);

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

	struct sockaddr_in name;
	int namelen = sizeof(name);
	if (uv_tcp_getpeername(pimpl->client, (struct sockaddr*) &name, &namelen))
		INFO << "uv_tcp_getpeername";

	char addr[16];
	static char buf[32];
	uv_inet_ntop(AF_INET, &name.sin_addr, addr, sizeof(addr));

	pimpl->address = std::string(addr);
	pimpl->port = name.sin_port;


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

void TcpClientUv::SendAndClose(std::string const && message)
{
	pimpl->sendMessage = message;
	pimpl->asyncType = TcpClientUvPimpl::AsyncType::SEND_AND_CLOSE;
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
