#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <HttpClient.h>
#include <functional>


struct HttpServerPimpl;
class HttpServer
{
public:
	typedef std::function<void(HttpClient *)> MessageReceivedCallback;
	typedef std::function<void(HttpClient *)> ClientConnectedCallback;
	typedef std::function<void(HttpClient *)> ClientDisconnectedCallback;

	HttpServer();
	~HttpServer();
	void Start(size_t port);
	void Stop();
	size_t GetPort();

	void SetMessageReceived(MessageReceivedCallback);
	void SetClientConnected(ClientConnectedCallback);
	void SetClientDisconnected(ClientDisconnectedCallback);

private:
	HttpServerPimpl* pimpl;
};
