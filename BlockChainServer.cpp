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
#include <HttpServer.h>
#include "http_parser.h"
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
    HttpServer* httpServer;
    BlockChainServer* server; // server
    uv_tcp_t* tcpServer;
    uv_timer_t elapsedTimer;
    BlockChain* chain;

	std::map<std::string, OperationCallback> operationCallbacks;
    
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
        httpServer = NULL;

		operationCallbacks["/help"] = std::bind(&BlockChainServerPimpl::helpOperation, this, std::placeholders::_1, std::placeholders::_2);
		operationCallbacks["/createblock"] = std::bind(&BlockChainServerPimpl::createBlockOperation, this, std::placeholders::_1, std::placeholders::_2);
		operationCallbacks["/getblock"] = std::bind(&BlockChainServerPimpl::getBlockOperation, this, std::placeholders::_1, std::placeholders::_2);
		operationCallbacks["/totalblocks"] = std::bind(&BlockChainServerPimpl::totalBlockOperation, this, std::placeholders::_1, std::placeholders::_2);
		operationCallbacks["/blocks"] = std::bind(&BlockChainServerPimpl::blocksOperation, this, std::placeholders::_1, std::placeholders::_2);
		operationCallbacks["/addnode"] = std::bind(&BlockChainServerPimpl::addNodeOperation, this, std::placeholders::_1, std::placeholders::_2);
		operationCallbacks["/nodes"] = std::bind(&BlockChainServerPimpl::nodeListOperation, this, std::placeholders::_1, std::placeholders::_2);
		operationCallbacks["/removenode"] = std::bind(&BlockChainServerPimpl::disconnectFromNodeOperation, this, std::placeholders::_1, std::placeholders::_2);
		operationCallbacks["/sync"] = std::bind(&BlockChainServerPimpl::syncOperation, this, std::placeholders::_1, std::placeholders::_2);
		operationCallbacks["/validate"] = std::bind(&BlockChainServerPimpl::validateOperation, this, std::placeholders::_1, std::placeholders::_2);
    }
    
    ~BlockChainServerPimpl()
    {
        if (chain != NULL)
            delete chain;
        
        if (parser_settings != NULL)
            delete parser_settings;
        
        if (tcpServer != NULL)
            delete tcpServer;
        
        if (httpServer)
            delete httpServer;
    }

	/* Operations */
	void helpOperation(std::string& request, std::stringstream & response)
	{
		response << "System functions:\r\n\r\n"
			<< " /createblock : Creating new block. Return a index number about block\r\n"
			<< " /getblock    : Get saved block via index\r\n"
			<< " /totalblocks : Total blocks count\r\n"
			<< " /blocks      : Download all blocks\r\n"
			<< " /addnode     : Add new node to system\r\n"
			<< " /nodes       : Get all connected nodes\r\n"
			<< " /removenode  : Disconnect from connected node\r\n"
			<< " /sync        : Synchronize local blocks with connected nodes. System automatically send last added block information for merging.\r\n"
			<< " /validate    : Validate block information.\r\n\r\n"
			<< "All functions return json data. You have to check 'Status' variable for operation validity.";
	}

	void disconnectFromNodeOperation(std::string& request, std::stringstream & response)
	{
		auto end = connectedNodes.end();
		for (auto it = connectedNodes.begin(); it != end; ++it)
			if ((*it)->GetRemoteAddress() == request)
			{
				(*it)->Disconnect();
				connectedNodes.erase(it);
			}
	}

	void syncOperation(std::string& request, std::stringstream & response)
	{
		auto message = "{\"Status\":true,\"Type\":" + std::to_string((int)MessageType::REQ_PARTIAL_BLOCKCHAIN) + ",\"Index\":" + std::to_string(chain->GetLastBlock()->Index) + "}";

		auto connectedNodesEnd = connectedNodes.end();
		for (auto it = connectedNodes.begin(); it != connectedNodesEnd; ++it)
			(*it)->Send(std::move(message));

		tcpSocket->BroadcastMessage(message);
	}

	void validateOperation(std::string& request, std::stringstream & response)
	{
		auto message = "{\"Status\":true,\"Type\":" + std::to_string((int)MessageType::REQ_PARTIAL_BLOCKCHAIN) + ",\"Index\":" + std::to_string(chain->GetLastBlock()->Index) + "}";

		std::string err;
		const json11::Json json = json11::Json::parse(request, err);

		if (err.empty())
		{
			auto index = json["Index"].int_value();
			auto hash = json["Hash"].string_value();
			auto previousHash = json["Hash"].string_value();
			auto nonce = json["Nonce"].int_value();
			auto timeSpan = json["Timespan"].int_value();

			auto validateStatus = chain->Validate(index, hash, previousHash, nonce, timeSpan);
			if (validateStatus)
				response << "{\"Status\":true}";
			else
				response << "{\"Status\":false}";
		}
		else
			response << "{\"Status\":false}";
	}

	void blocksOperation(std::string& request, std::stringstream & response)
	{
		response << chain->SerializeChain();
	}

	void createBlockOperation(std::string& request, std::stringstream & response)
	{
		Block * block = chain->NewBlock(request);
		if (block != NULL)
		{
			response << "{\"Status\":true,\"Index\":" << block->Index << "\"}";

			std::string messageData = "{\"Status\":true,\"Type\":" + std::to_string((int)MessageType::RES_LAST_BLOCK) + ",\"Block\":" + block->Encode() + "}";

			if (tcpSocket != NULL)
				tcpSocket->BroadcastMessage("{\"Status\":true,\"Type\":" + std::to_string((int)MessageType::RES_LAST_BLOCK) + ",\"Block\":" + block->Encode() + "}");

			auto connectedNodesEnd = connectedNodes.end();
			for (auto it = connectedNodes.begin(); it != connectedNodesEnd; ++it)
				(*it)->Send(std::move(messageData));
		}
		else
			response << "{\"Status\":false,\"Message\":\"Block not added.\"}";
	}

	void getBlockOperation(std::string& request, std::stringstream & response)
	{
		if (isInteger(request.c_str()))
		{
			Block * block = chain->Get((size_t)atoi(request.c_str()));
			if (block != NULL)
				response << "{\"Status\":true,\"Data\":" << block->Data << "\"}";
			else
				response << "{\"Status\":false,\"Message\":\"Block not found.\"}";
		}
		else
			response << "{\"Status\":false,\"Message\":\"Please send block index.\"}";
	}


	void addNodeOperation(std::string& request, std::stringstream & response)
	{
		std::stringstream ss(request.c_str());
		std::string item;
		std::vector<std::string> tokens;
		while (std::getline(ss, item, ':'))
			tokens.push_back(item);

		if (tokens.size() == 2)
		{
			auto status = ConnectToNode(tokens[0], (size_t)stoi(tokens[1]));
			if (status == ConnectToBlockStatus::ADDED)
				response << "{\"Status\":true:\"Message\":\"Node will be added.\"}";
			else
				response << "{\"Status\":false,\"Message\":\"Node already added.\"}";
		}
		else
			response << "{\"Status\":false,\"Message\":\"Node connection info not valid.\"}";
	}

	void nodeListOperation(std::string& request, std::stringstream & response)
	{

		auto nodesEnd = connectedNodes.end();
		auto it = connectedNodes.begin();

		response << "{\"Status\":true,\"Nodes\":[";
		if (it != nodesEnd)
		{
			response << "\"" << (*it)->GetRemoteAddress() << ":" << (*it)->GetRemotePort() << "\"";
			++it;

			for (; it != nodesEnd; ++it)
				response << "," << "\"" << *it << "\"";
		}
		response << "]}";
	}

	void totalBlockOperation(std::string& request, std::stringstream & response)
	{
		response << "{\"Status\":true,\"TotalBlock\":" << std::to_string(chain->TotalBlocks()) << "}";
	}
	/* Operations */
    
    
    /* Http Callbacks */
    void httpMessageReceived(HttpClient* client)
    {
        auto end = operationCallbacks.end();
        auto callback = operationCallbacks.find(client->Url);
        if (callback != end)
            callback->second(client->RequestBuffer, client->ResponseBuffer);
        else
            client->ResponseBuffer << "{\"Status\":false,\"Message\":\"Page not found\"}";
        
        client->Send();
    }
    
    void httpClientConnected(HttpClient* client)
    {
        
    }
    
    void httpClientDisconnected(HttpClient* client)
    {
        
    }
    /* Http Callbacks */
    
    
    
    void connectedToNewNode(TcpClient& client)
    {
        INFO << "Connected to remote server." << std::endl;
        // client.Send("{\"Status\":true,\"Type\":" + std::to_string((int)MessageType::REQ_FULL_BLOCKCHAIN) +"}");
    }
    
    ConnectToBlockStatus ConnectToNode(std::string address, size_t port)
    {
        auto end = connectedNodes.end();
        for (auto it = connectedNodes.begin(); it != end; ++it)
            if ((*it)->GetRemoteAddress() == address && (*it)->GetRemotePort() == port)
                return ConnectToBlockStatus::ALREADY_ADDED;
        
        TcpClient *client = tcpSocket->CreateClient();
        
        client->SetOnConnect(std::bind(&BlockChainServerPimpl::newNodeConnected, this, std::placeholders::_1));
        client->SetOnDisconnect(std::bind(&BlockChainServerPimpl::tcpClientDisconnected, this, std::placeholders::_1));
        client->SetOnMessage(std::bind(&BlockChainServerPimpl::tcpMessageReceived, this, std::placeholders::_1, std::placeholders::_2));
        
        client->Connect(address, port);
        connectedNodes.push_back(client);
        
        return ConnectToBlockStatus::ADDED;
    }
    
    void disconnectFromNode(std::string address)
    {
        auto end = connectedNodes.end();
        for(auto it = connectedNodes.begin(); it != end; ++it)
            if ((*it)->GetRemoteAddress() == address)
            {
                (*it)->Disconnect();
                connectedNodes.erase(it);
            }
    }
    
    /* Tcp Callbacks */
    void newNodeConnected(TcpClient& tcpClient)
    {
        tcpClient.Send("{\"Status\":true,\"Type\":" + std::to_string((int) MessageType::REQ_NODE_INFO) + ",\"Index\":" + std::to_string(chain->GetLastBlock()->Index) + "}");
    }
    
    void tcpClientConnected(TcpClient& tcpClient)
    {
        
    }
    
    void tcpClientDisconnected(TcpClient& tcpClient)
    {
        
    }
    
    /* Block information received from remote node*/
    void tcpMessageReceived(std::string const& text, TcpClient& tcpClient)
    {
        struct NodeMessage message(text);

        switch (message.Type)
        {
            case MessageType::RES_LAST_BLOCK:
            {
                INFO << "#RES_LAST_BLOCK " << message.Block->Index << std::endl;
                auto addStatus = chain->AddBlock(message.Block);

                switch(addStatus)
                {
                    case BlockChain::AddStatus::ADDED: {
                        std::stringstream responseBuffer;
                        responseBuffer << "{\"Status\":true,\"Type\":" << (int) MessageType::RES_LAST_BLOCK
                                       << ",\"Block\":";
                        responseBuffer << chain->GetLastBlock()->Encode();
                        responseBuffer << "}";

                        auto connectedNodesEnd = connectedNodes.end();
                        for (auto it = connectedNodes.begin(); it != connectedNodesEnd; ++it)
                            if (tcpClient.GetRemotePort() != (*it)->GetRemotePort() && tcpClient.GetRemoteAddress() != (*it)->GetRemoteAddress())
                                (*it)->Send(std::move(responseBuffer.str()));

                        tcpSocket->BroadcastMessageExpect(responseBuffer.str(), tcpClient);
                        break;
                    }

                    case BlockChain::AddStatus::BLOCK_IS_NEWER:
                    {
                        size_t lastIndex = chain->GetLastBlock()->Index;
                        
                        std::stringstream responseBuffer;
                        responseBuffer << "{\"Status\":true,\"Type\":" << (int)MessageType::REQ_PARTIAL_BLOCKCHAIN << ",\"Index\":";
                        responseBuffer << lastIndex;
                        responseBuffer << "}";
                        
                        tcpClient.Send(responseBuffer.str());
                        
                        break;
                    }

                    case BlockChain::AddStatus::BLOCK_IS_OLDER:
                    {
                        std::stringstream responseBuffer;
                        responseBuffer << "{\"Status\":true,\"Type\":" << (int)MessageType::RES_PARTIAL_BLOCKCHAIN << ",\"Blocks\":";
                        responseBuffer << chain->SerializeChain(message.Block->Index);
                        responseBuffer << "}";
                        
                        tcpClient.Send(responseBuffer.str());
                        break;
                    }

                    case BlockChain::AddStatus::SKIPPED:
                        break;

                    case BlockChain::AddStatus::INVALID_BLOCK:
                        break;
                }

                break;
            }
                
            case MessageType::RES_PARTIAL_BLOCKCHAIN:
            {
                INFO << "#RES_PARTIAL_BLOCKCHAIN" << std::endl;
                bool mergeStatus = chain->Merge(*message.Blocks);
                
                break;
            }

            case MessageType::RES_FULL_BLOCKCHAIN:
            {
                INFO << "#RES_FULL_BLOCKCHAIN" << std::endl;
                bool chainReplaceStatus = chain->SetChain(*message.Blocks);

                break;
            }

            case MessageType::REQ_FULL_BLOCKCHAIN:
            {
                INFO << "#REQ_FULL_BLOCKCHAIN" << std::endl;
                std::stringstream responseBuffer;
                responseBuffer << "{\"Status\":true,\"Type\":" << (int)MessageType::RES_FULL_BLOCKCHAIN << ",\"Blocks\":";
                responseBuffer << chain->SerializeChain();
                responseBuffer << "}";

                tcpClient.Send(responseBuffer.str());
                break;
            }

            case MessageType::REQ_LAST_BLOCK:
            {
                INFO << "#REQ_LAST_BLOCK" << std::endl;
                std::stringstream responseBuffer;
                responseBuffer << "{\"Status\":true,\"Type\":" << (int)MessageType::RES_LAST_BLOCK << ",\"Block\":";
                responseBuffer << chain->GetLastBlock()->Encode();
                responseBuffer << "}";

                tcpClient.Send(responseBuffer.str());
                break;
            }

            case MessageType::REQ_PARTIAL_BLOCKCHAIN:
            {
                INFO << "#REQ_PARTIAL_BLOCKCHAIN" << std::endl;
                std::stringstream responseBuffer;
                responseBuffer << "{\"Status\":true,\"Type\":" << (int)MessageType::RES_PARTIAL_BLOCKCHAIN << ",\"Blocks\":";
                responseBuffer << chain->SerializeChain(message.Index);
                responseBuffer << "}";

                tcpClient.Send(responseBuffer.str());
                break;
            }

            case MessageType::RES_NODE_LIST:
            {
                INFO << "#RES_NODE_LIST" << std::endl;
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
                INFO << "#REQ_NODE_LIST" << std::endl;
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
                
            case MessageType::REQ_NODE_INFO:
            {
                INFO << "#REQ_NODE_INFO" << std::endl;
                auto nodesEnd = connectedNodes.end();
                auto it = connectedNodes.begin();
                
                std::stringstream responseBuffer;
                responseBuffer << "{\"Status\":true,\"Type\":" + std::to_string((int) MessageType::RES_NODE_INFO) + ",\"Nodes\":[";
                
                if (it != nodesEnd)
                {
                    responseBuffer << "\"" << (*it)->GetRemoteAddress() << ":" << (*it)->GetRemotePort() << "\"";
                    ++it;
                    
                    for (; it != nodesEnd; ++it)
                        responseBuffer << "," << "\"" << (*it)->GetRemoteAddress() << ":" << (*it)->GetRemotePort() << "\"";
                }
                
                responseBuffer << "],\"Index\":" << chain->GetLastBlock()->Index << ",\"Blocks\":";
                responseBuffer << chain->SerializeChain(message.Index) << "}";
                tcpClient.Send(responseBuffer.str());
                break;
            }

            case MessageType::RES_NODE_INFO:
                INFO << "#RES_NODE_INFO" << std::endl;
                chain->Merge(*message.Blocks);
                
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
    }
    /* Tcp Callbacks */
};

/* BLOCK CHAIN SERVER */
BlockChainServer::BlockChainServer()
:pimpl(new BlockChainServerPimpl(this))
{
    pimpl->httpServer = new HttpServer;
    pimpl->httpServer->SetMessageReceived(std::bind(&BlockChainServerPimpl::httpMessageReceived, pimpl, std::placeholders::_1));
    pimpl->httpServer->SetClientConnected(std::bind(&BlockChainServerPimpl::httpClientConnected, pimpl, std::placeholders::_1));
    pimpl->httpServer->SetClientDisconnected(std::bind(&BlockChainServerPimpl::httpClientDisconnected, pimpl, std::placeholders::_1));
}

BlockChainServer::~BlockChainServer()
{
    delete pimpl;
}

void BlockChainServer::Start(size_t port)
{
    pimpl->httpServer->Start(port);
    INFO << "Http Server Started" << std::endl;
}

void BlockChainServer::Stop()
{
    
}

void BlockChainServer::SetTcpServer(TcpServer* server)
{
    pimpl->tcpSocket = server;
    pimpl->tcpSocket->SetMessageReceived(std::bind(&BlockChainServerPimpl::tcpMessageReceived, pimpl, std::placeholders::_1, std::placeholders::_2));
    pimpl->tcpSocket->SetClientConnected(std::bind(&BlockChainServerPimpl::tcpClientConnected, pimpl, std::placeholders::_1));
    pimpl->tcpSocket->SetClientDisconnected(std::bind(&BlockChainServerPimpl::tcpClientDisconnected, pimpl, std::placeholders::_1));
}
