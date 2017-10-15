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
#include <NodeMessage.h>

namespace
{
    static http_parser_settings * parser_settings;
}

enum class ConnectToBlockStatus { ADDED = 0, ALREADY_ADDED = 1 };

/* BLOCK CHAIN SERVER PIMPL*/
struct BlockChainServerPimpl
{
    TcpServer* tcpSocket;
    BlockChainServer* server; // server
    uv_tcp_t* tcpServer;
    uv_timer_t elapsedTimer;
    BlockChain* chain;
    
    bool largeContentReceiving;
    std::stringstream buffer;
    
    std::vector<TcpClient*> connectedNodes;
    
    BlockChainServerPimpl(BlockChainServer * pServer)
    {
        server = pServer;
        parser_settings = new http_parser_settings;
        tcpServer = new uv_tcp_t;
        tcpServer->data = this;
        chain = new BlockChain();
        tcpSocket = NULL;
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
            
            INFO << "Block Sending : " << block->Encode() << std::endl;
            
            client->ResponseBuffer << "{\"Status\":true,\"Index\":" << block->Index << "\"}";
            
            std::string messageData = "{\"Status\":true,\"Type\":" + std::to_string((int)MessageType::RES_LAST_BLOCK) + ",\"Block\":" + block->Encode() + "}";
            
            if (tcpSocket != NULL)
                tcpSocket->BroadcastMessage("{\"Status\":true,\"Type\":" + std::to_string((int)MessageType::RES_LAST_BLOCK) + ",\"Block\":" + block->Encode() + "}");
            
            auto connectedNodesEnd = connectedNodes.end();
            for(auto it = connectedNodes.begin(); it != connectedNodesEnd; ++it)
                (*it)->Send(std::move(messageData));
        }
        else
            client->ResponseBuffer << "{\"Status\":false,\"Message\":\"Block not added.\"}";
    }
    
    void getBlockCall(HttpClient* client)
    {
        if (isInteger(client->RequestBuffer.c_str()))
        {
            Block * block = chain->Get(atoi(client->RequestBuffer.c_str()));
            if (block != NULL)
                client->ResponseBuffer << "{\"Status\":true,\"Data\":" << block->Data << "\"}";
            else
                client->ResponseBuffer << "{\"Status\":false,\"Message\":\"Block not found.\"}";
        }
        else
            client->ResponseBuffer << "{\"Status\":false,\"Message\":\"Please send block index.\"}";
    }
    
    void addNodeCall(HttpClient* client)
    {
        std::stringstream ss(client->RequestBuffer.c_str());
        std::string item;
        std::vector<std::string> tokens;
        while (std::getline(ss, item, ':'))
            tokens.push_back(item);
        
        if (tokens.size() == 2)
        {
            auto status = client->ServerPimpl->ConnectToNode(tokens[0], (size_t) stoi(tokens[1]));
            if (status == ConnectToBlockStatus::ADDED)
                client->ResponseBuffer << "{\"Status\":true:\"Message\":\"Node will be added.\"}";
            else
                client->ResponseBuffer << "{\"Status\":false,\"Message\":\"Node already added.\"}";
        }
        else
            client->ResponseBuffer << "{\"Status\":false,\"Message\":\"Node connection info not valid.\"}";
        
        
    }
    
    void nodeListCall(HttpClient* client)
    {
        
        auto nodesEnd = connectedNodes.end();
        auto it = connectedNodes.begin();
        
        client->ResponseBuffer << "{\"Status\":true,\"Nodes\":[";
        
        if (it != nodesEnd)
        {
            client->ResponseBuffer << "\"" << (*it)->GetRemoteAddress() << ":" << (*it)->GetRemotePort() << "\"";
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
    
    void messageReceivedFromNode(std::string const& message, TcpClient& client)
    {
        INFO << "Message received from node : " << message << std::endl;
    }
    
    void connectedToNewNode(TcpClient& client)
    {
        INFO << "Connected to remote server." << std::endl;
        client.Send("{\"Status\":true,\"Type\":" + std::to_string((int)MessageType::REQ_FULL_BLOCKCHAIN) +"}");
    }
    
    ConnectToBlockStatus ConnectToNode(std::string address, size_t port)
    {
        auto end = connectedNodes.end();
        for (auto it = connectedNodes.begin(); it != end; ++it)
            if ((*it)->GetRemoteAddress() == address && (*it)->GetRemotePort() == port)
                return ConnectToBlockStatus::ALREADY_ADDED;
        
        TcpClient *client = tcpSocket->CreateClient();
        
        client->SetOnConnect(std::bind(&BlockChainServerPimpl::connectedToNewNode, this, std::placeholders::_1));
        client->SetOnMessage(std::bind(&BlockChainServerPimpl::onMessageReceived, this, std::placeholders::_1, std::placeholders::_2));
        
        client->Connect(address, port);
        connectedNodes.push_back(client);
        
        return ConnectToBlockStatus::ADDED;
    }
    
    void DisconnectFromNode(std::string address)
    {
        auto end = connectedNodes.end();
        for(auto it = connectedNodes.begin(); it != end; ++it)
            if ((*it)->GetRemoteAddress() == address)
                (*it)->Disconnect();
    }
    
    static int on_message_complete(http_parser* parser) {
        HttpClient* client = (HttpClient*)parser->data;
        
        if (client->Url == "/createblock")
            client->ServerPimpl->addBlockCall(client);
        else if (client->Url == "/getblock")
            client->ServerPimpl->getBlockCall(client);
        else if (client->Url == "/count")
            client->ServerPimpl->totalBlockCall(client);
        else if (client->Url == "/blocks")
            client->ResponseBuffer << client->ServerPimpl->chain->SerializeChain();
        else if (client->Url == "/addnode")
            client->ServerPimpl->addNodeCall(client);
        else if (client->Url == "/nodes")
            client->ServerPimpl->nodeListCall(client);
        else if (client->Url == "/removenode")
            client->ServerPimpl->DisconnectFromNode(client->RequestBuffer.c_str());
        else if (client->Url == "/testclient")
        {
            client->RequestBuffer = "ws://127.0.0.1:" + std::to_string(client->ServerPimpl->tcpSocket->GetPort());
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
    
    void onClientConnected(TcpClient& tcpClient)
    {
        
    }
    
    /* Block information received from remote node*/
    void onMessageReceived(std::string const& text, TcpClient& tcpClient)
    {
        struct NodeMessage message(text);

        switch (message.Type)
        {
            case MessageType::RES_LAST_BLOCK:
            {
                INFO << "Received Last Block : " << message.Block->Encode() << std::endl;
                auto addStatus = chain->AddBlock(message.Block);

                switch(addStatus)
                {
                    case BlockChain::AddStatus::ADDED:
                        break;

                    case BlockChain::AddStatus::BLOCK_IS_NEWER:
                    {
                        size_t lastIndex = chain->GetLastBlock()->Index;
                        //tcpClient.Send("");
                        break;
                    }

                    case BlockChain::AddStatus::BLOCK_IS_OLDER:
                        break;

                    case BlockChain::AddStatus::SKIPPED:
                        break;

                    case BlockChain::AddStatus::INVALID_BLOCK:
                        break;
                }

                break;
            }

            case MessageType::RES_FULL_BLOCKCHAIN:
            {
                bool chainReplaceStatus = chain->SetChain(*message.Blocks);

                break;
            }

            case MessageType::REQ_FULL_BLOCKCHAIN:
            {
                std::stringstream responseBuffer;
                responseBuffer << "{\"Status\":true,\"Type\":" << (int)MessageType::RES_FULL_BLOCKCHAIN << ",\"Blocks\":";
                responseBuffer << chain->SerializeChain();
                responseBuffer << "}";

                tcpClient.Send(responseBuffer.str());
                break;
            }

            case MessageType::REQ_LAST_BLOCK:
            {
                std::stringstream responseBuffer;
                responseBuffer << "{\"Status\":true,\"Type\":" << (int)MessageType::RES_LAST_BLOCK << ",\"Block\":";
                responseBuffer << chain->GetLastBlock()->Encode();
                responseBuffer << "}";

                tcpClient.Send(responseBuffer.str());
                break;
            }

            case MessageType::REQ_PARTIAL_BLOCKCHAIN:
            {
                std::stringstream responseBuffer;
                responseBuffer << "{\"Status\":true,\"Type\":" << (int)MessageType::RES_PARTIAL_BLOCKCHAIN << ",\"Blocks\":";
                responseBuffer << chain->SerializeChain(message.Index);
                responseBuffer << "}";

                tcpClient.Send(responseBuffer.str());
                break;
            }

            case MessageType::RES_NODE_LIST:
            {
                auto nodesEnd = message.Nodes->end();

                for (auto it = message.Nodes->begin(); it != nodesEnd; ++it)
                {
                    std::stringstream ss((*it));
                    std::string item;
                    std::vector<std::string> tokens;
                    while (std::getline(ss, item, ':'))
                        tokens.push_back(item);

                    if (tokens.size() == 2)
                        ConnectToNode(tokens[0], (size_t) stoi(tokens[1]));
                }

                break;
            }

            case MessageType::REQ_NODE_LIST:
            {
                auto nodesEnd = connectedNodes.end();
                auto it = connectedNodes.begin();

                std::stringstream responseBuffer;
                responseBuffer << "{\"Status\":true,\"Nodes\":[";

                if (it != nodesEnd)
                {
                    responseBuffer << "\"" << (*it)->GetRemoteAddress() << ":" << (*it)->GetRemotePort() << "\"";
                    ++it;

                    for (; it != nodesEnd; ++it)
                        responseBuffer << "," << "\"" << (*it)->GetRemoteAddress() << ":" << (*it)->GetRemotePort() << "\"";
                }

                responseBuffer << "]}";

                tcpClient.Send(responseBuffer.str());
                break;
            }

            case MessageType::RES_INFO:
                break;
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

void BlockChainServer::SetTcpServer(TcpServer* server)
{
    pimpl->tcpSocket = server;
    pimpl->tcpSocket->SetMessageReceived(std::bind(&BlockChainServerPimpl::onMessageReceived, pimpl, std::placeholders::_1, std::placeholders::_2));
    pimpl->tcpSocket->SetClientConnected(std::bind(&BlockChainServerPimpl::onClientConnected, pimpl, std::placeholders::_1));
}
