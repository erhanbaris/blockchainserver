#include <HttpServer.h>
#include <map>
#include <uv.h>
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <Block.h>
#include <HttpClient.h>
#include "http_parser.h"
#include <json11.hpp>
#include <sstream>
#include <NodeMessage.h>
#include <TcpServerUv.h>

namespace
{
	static http_parser_settings * parser_settings;
}

struct HttpServerPimpl
{
	uv_tcp_t* tcpServer;
	uv_timer_t elapsedTimer;
	HttpServer* httpServer;
    TcpServerUv* server;

    size_t port;

	// callbacks
	HttpServer::MessageReceivedCallback messageReceivedCallback;
    HttpServer::ClientConnectedCallback clientConnectedCallback;
    HttpServer::ClientDisconnectedCallback clientDisconnectedCallback;

	HttpServerPimpl(HttpServer* pHttpServer)
	{
		httpServer = pHttpServer;
		parser_settings = new http_parser_settings;
		tcpServer = new uv_tcp_t;
		tcpServer->data = this;
        server = NULL;
	}

	~HttpServerPimpl()
	{
		if (parser_settings != NULL)
			delete parser_settings;
	}

	void messageReceived(std::string const& message, TcpClient* client)
	{
		auto parsedSize = http_parser_execute((http_parser*)((HttpClient*)client->Data1)->Parser, parser_settings, message.c_str(), message.size());
		if (parsedSize == message.size())
		{
            auto parser = (http_parser*)((HttpClient*)client->Data1)->Parser;
            if (messageReceivedCallback)
                messageReceivedCallback((HttpClient*)client->Data1);
		}
	}

	void clientConnected(TcpClient* client)
	{
		HttpClient* httpClient = new HttpClient;
		http_parser* httpParser = new http_parser;

		httpClient->Data1 = client;
		httpClient->Parser = httpParser;
		httpParser->data = httpClient;
		client->Data1 = httpClient;

		http_parser_init(httpParser, HTTP_REQUEST);
        
        if (clientConnectedCallback)
            clientConnectedCallback(httpClient);
	}

	void clientDisconnected(TcpClient* client)
	{
        HttpClient * httpClient = (HttpClient*)client->Data1;
        
        delete (http_parser*)httpClient->Parser;
        delete client;
        
        if (clientDisconnectedCallback)
            clientDisconnectedCallback(httpClient);
	}


	static int on_message_begin(http_parser* /*parser*/) {
		return 0;
	}

	static int on_url(http_parser* parser, const char* url, size_t length) {

		HttpClient* client = (HttpClient*)parser->data;
		struct http_parser_url u;
		int result = http_parser_parse_url(url, length, 0, &u);
		if (result) {
			fprintf(stderr, "\n\n*** failed to parse URL %s ***\n\n", url);
			return -1;
		}
		else {
			if ((u.field_set & (1 << UF_PATH))) {
				const char * data = url + u.field_data[UF_PATH].off;
				client->Url = std::string(data, u.field_data[UF_PATH].len);
			}
		}
		return 0;
	}

	static int on_header_field(http_parser* /*parser*/, const char* at, size_t length) {
		return 0;
	}

	static int on_header_value(http_parser* /*parser*/, const char* at, size_t length) {
		return 0;
	}

	static int on_headers_complete(http_parser* /*parser*/) {
		return 0;
	}

	static int on_body(http_parser* parser, const char* at, size_t length) {
		HttpClient* client = (HttpClient*)parser->data;
		client->RequestBuffer = std::string(at, length);
		return 0;
	}

	static int on_message_complete(http_parser* parser) {
		return 0;
	}
};

/* BLOCK CHAIN SERVER */
HttpServer::HttpServer()
	:pimpl(new HttpServerPimpl(this))
{
    pimpl->server = new TcpServerUv;

	parser_settings->on_url = pimpl->on_url;
	parser_settings->on_message_begin = pimpl->on_message_begin;
	parser_settings->on_message_complete = pimpl->on_message_complete;
	parser_settings->on_header_field = pimpl->on_header_field;
	parser_settings->on_header_value = pimpl->on_header_value;
	parser_settings->on_headers_complete = pimpl->on_headers_complete; 
	parser_settings->on_body = pimpl->on_body;

    pimpl->server->SetMessageReceived(std::bind(&HttpServerPimpl::messageReceived, pimpl, std::placeholders::_1, std::placeholders::_2));
    pimpl->server->SetClientConnected(std::bind(&HttpServerPimpl::clientConnected, pimpl, std::placeholders::_1));
    pimpl->server->SetClientDisconnected(std::bind(&HttpServerPimpl::clientDisconnected, pimpl, std::placeholders::_1));
}

void HttpServer::Start(size_t port) {
    pimpl->port = port;
    pimpl->server->Start(port);
}

void HttpServer::Stop()
{

}

size_t HttpServer::GetPort()
{
    return pimpl->port;
}

void HttpServer::SetMessageReceived(MessageReceivedCallback cb)
{
	pimpl->messageReceivedCallback = cb;
}

void HttpServer::SetClientConnected(ClientConnectedCallback cb)
{
	pimpl->clientConnectedCallback = cb;
}
void HttpServer::SetClientDisconnected(ClientDisconnectedCallback cb)
{
	pimpl->clientDisconnectedCallback = cb;
}

HttpServer::~HttpServer()
{
	delete pimpl;
}
