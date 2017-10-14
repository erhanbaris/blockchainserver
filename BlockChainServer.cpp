#include <BlockChainServer.h>
#include <BlockChain.h>
#include <map>
#include <uv.h>
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <Block.h>
#include <HttpClient.h>
#include "http_parser.h"
#include "Tools.h"
#include <json11.hpp>
#include <sstream>

namespace
{
	static http_parser_settings * parser_settings;
}

/* BLOCK CHAIN SERVER PIMPL*/
struct BlockChainServerPimpl
{
	WebSocketServer* webSocket;
	BlockChainServer* server; // server
	uv_tcp_t* tcpServer;
	uv_timer_t elapsedTimer;
	BlockChain* chain;

	BlockChainServerPimpl(BlockChainServer * pServer)
	{
		server = pServer;
		parser_settings = new http_parser_settings;
		tcpServer = new uv_tcp_t;
		tcpServer->data = this;
		chain = new BlockChain();
		webSocket = NULL;
	}

	~BlockChainServerPimpl()
	{
		if (chain != NULL)
			delete chain;

		if (parser_settings != NULL)
			delete parser_settings;

		if (tcpServer != NULL)
			delete tcpServer;
	}

	static void alloc_cb(uv_handle_t * /*handle*/, size_t suggested_size, uv_buf_t* buf) {
		*buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
	}

	static void on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t * buf) {
		ssize_t parsed;
		HttpClient* client = (HttpClient*)tcp->data;
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
		free(buf->base);
	}

	static int on_message_begin(http_parser* /*parser*/) {
		return 0;
	}

	static int on_headers_complete(http_parser* /*parser*/) {
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

	static int on_body(http_parser* parser, const char* at, size_t length) {
		HttpClient* client = (HttpClient*)parser->data;
		client->RequestBuffer = std::string(at, length);
		return 0;
	}

	void addBlockCall(HttpClient* client)
	{
		Block * block = chain->NewBlock(client->RequestBuffer.c_str());
		if (block != NULL)
        {
            client->ResponseBuffer << "{\"Status\":true,\"Block\":" << block->Encode() << "\"}";
            
            if (webSocket != NULL)
                webSocket->BroadcastMessage("{\"Status\":true,\"Type\":" + std::to_string((int)MessageType::RES_LAST_BLOCK) + ",\"Block\":" + block->Encode() + "}");
        }
		else
			client->ResponseBuffer << "{\"Status\":false,\"Message\":\"Block not added.\"}";
	}

	void addNodeCall(HttpClient* client)
	{
		auto status = client->ServerPimpl->webSocket->ConnectToNode(client->RequestBuffer.c_str());
		if (status == WebSocketServer::ConnectToBlockStatus::ADDED)
			client->ResponseBuffer << "{\"Status\":true}";
		else
			client->ResponseBuffer << "{\"Status\":false,\"Message\":\"Node already added.\"}";
	}

	void nodeListCall(HttpClient* client)
	{
		auto nodes = client->ServerPimpl->webSocket->ConnectedNodes();
		auto nodesEnd = nodes.end();
		auto it = nodes.begin();

		client->ResponseBuffer << "{\"Status\":true,\"Nodes\":[";

		if (it != nodesEnd)
		{
			client->ResponseBuffer << "\"" << *it << "\"";
			++it;

			for (; it != nodesEnd; ++it)
				client->ResponseBuffer << "," << "\"" << *it << "\"";
		}

		client->ResponseBuffer << "]}";
	}

	void totalBlockCall(HttpClient* client)
	{
		client->ResponseBuffer << "{\"Status\":true,\"TotalBlock\":" << std::to_string(chain->TotalBlocks()) << "}";
	}

	static int on_message_complete(http_parser* parser) {
		HttpClient* client = (HttpClient*)parser->data;

		if (client->Url == "/createblock")
			client->ServerPimpl->addBlockCall(client);
		else if (client->Url == "/count")
			client->ServerPimpl->totalBlockCall(client);
		else if (client->Url == "/allblocks")
			client->ResponseBuffer << client->ServerPimpl->chain->SerializeChain();
		else if (client->Url == "/addnode")
			client->ServerPimpl->addNodeCall(client);
		else if (client->Url == "/nodes")
			client->ServerPimpl->nodeListCall(client);
		else if (client->Url == "/removenode")
			client->ServerPimpl->webSocket->DisconnectFromNode(client->RequestBuffer.c_str());
		else if (client->Url == "/testclient")
		{
			client->RequestBuffer = "ws://127.0.0.1:" + std::to_string(client->ServerPimpl->webSocket->GetPort());
			client->ServerPimpl->addNodeCall(client);
		}
		else
			client->ResponseBuffer << "{\"Status\":false,\"Message\":\"Page not found\"}";

		client->Send();
		return 0;
	}

	static void on_connect(uv_stream_t* server_handle, int status) {
		HttpClient *client = new HttpClient;
		client->Handle = new uv_tcp_t;
		client->Parser = new http_parser;
		client->Async = new uv_async_t;
		client->ServerPimpl = (BlockChainServerPimpl *)server_handle->data;

		uv_tcp_init(loop, (uv_tcp_t*)client->Handle);
		uv_async_init(loop, ((uv_async_t*)client->Async), HttpClient::sendAsync);
		http_parser_init((http_parser*)client->Parser, HTTP_REQUEST);

		((http_parser*)client->Parser)->data = client;
		((uv_tcp_t*)client->Handle)->data = client;
		((uv_async_t*)client->Async)->data = client;

		int r = uv_accept(server_handle, (uv_stream_t*)client->Handle);

		uv_read_start((uv_stream_t*)client->Handle, alloc_cb, on_read);
	}

	/* Block information received from remote node*/
	void onMessageReceived(std::string const& message, WebSocketClient& webSocketClient)
	{
		std::string err;
		const json11::Json json = json11::Json::parse(message, err);

		if (err.empty())
		{
			MessageType messageType = (MessageType)json["Type"].int_value();
			switch (messageType)
			{
                case MessageType::RES_LAST_BLOCK:
                {
                    Block* block = NULL;
                    Block::Decode(json["Block"], block);
                    break;
                }

                case MessageType::RES_PARTIAL_BLOCKCHAIN:
                case MessageType::RES_FULL_BLOCKCHAIN:
                {
                    size_t totalBlocks = 0;
                    Block* block = NULL;
                    Block::Decode(json["Block"], block, &totalBlocks);
                    break;
                }

                case MessageType::REQ_FULL_BLOCKCHAIN:
                {
                    std::string responseMessage = chain->SerializeChain();
                    webSocketClient.Send(responseMessage);
                    break;
                }

                case MessageType::REQ_LAST_BLOCK:
                {
                    std::string responseMessage = chain->GetLastBlock()->Encode();
                    webSocketClient.Send(responseMessage);
                    break;
                }

                case MessageType::REQ_PARTIAL_BLOCKCHAIN:
                {
                    size_t index = (size_t)json["Index"].int_value();
                    std::string responseMessage = chain->SerializeChain(index);
                    webSocketClient.Send(responseMessage);
                    break;
                }
                    
                case MessageType::RES_NODE_LIST:
                {
                    std::vector<json11::Json> nodes = json["Nodes"].array_items();
                    auto nodesEnd = nodes.end();
                    
                    for (auto it = nodes.begin(); it != nodesEnd; ++it)
                        webSocket->ConnectToNode(it->string_value());
                    
                    break;
                }

                case MessageType::REQ_NODE_LIST:
                {
                    auto nodes = webSocket->ConnectedNodes();
                    auto nodesEnd = nodes.end();
                    auto it = nodes.begin();

                    std::stringstream responseBuffer;
                    responseBuffer << "{\"Status\":true,\"Nodes\":[";

                    if (it != nodesEnd)
                    {
                        responseBuffer << "\"" << *it << "\"";
                        ++it;

                        for (; it != nodesEnd; ++it)
                            responseBuffer << "," << "\"" << *it << "\"";
                    }

                    responseBuffer << "]}";

                    std::string message = responseBuffer.str();
                    webSocketClient.Send(message);
                    break;
                }
			}
		}
	}
};

/* BLOCK CHAIN SERVER */
BlockChainServer::BlockChainServer()
	:pimpl(new BlockChainServerPimpl(this))
{ }

BlockChainServer::~BlockChainServer()
{
	delete pimpl;
}

void BlockChainServer::Start(size_t port)
{
	parser_settings->on_url = pimpl->on_url;

	parser_settings->on_message_begin = pimpl->on_message_begin;
	parser_settings->on_headers_complete = pimpl->on_headers_complete;
	parser_settings->on_message_complete = pimpl->on_message_complete;

	parser_settings->on_header_field = pimpl->on_header_field;
	parser_settings->on_header_value = pimpl->on_header_value;
	parser_settings->on_body = pimpl->on_body;

	uv_tcp_init(loop, pimpl->tcpServer);
	struct sockaddr_in address;
	uv_ip4_addr("0.0.0.0", port, &address);
	uv_tcp_bind(pimpl->tcpServer, (const struct sockaddr*)&address, 0);
	uv_listen((uv_stream_t*)pimpl->tcpServer, 1000, BlockChainServerPimpl::on_connect);

	INFO << "Http Server Started" << std::endl;
	uv_run(loop, UV_RUN_DEFAULT);
}

void BlockChainServer::Stop()
{

}

void BlockChainServer::SetWebSocket(WebSocketServer* socket)
{
	pimpl->webSocket = socket;
	pimpl->webSocket->SetMessageReceived(bind(&BlockChainServerPimpl::onMessageReceived, pimpl, std::placeholders::_1, std::placeholders::_2));
}
