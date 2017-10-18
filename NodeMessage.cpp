#include <NodeMessage.h>

NodeMessage::NodeMessage() : Block(NULL), Blocks(NULL), Nodes(NULL), HttpPort(0), TcpPort(0), Index(0)
{

}

NodeMessage::NodeMessage(std::string const& message) : Block(NULL), Blocks(NULL), Nodes(NULL), HttpPort(0), TcpPort(0), Index(0)
{
	std::string err;
	json11::Json const json = json11::Json::parse(message, err);

	if (err.empty()) {
		Type = (MessageType)json["Type"].int_value();

		switch (Type) {
		case MessageType::RES_LAST_BLOCK: {
			Block::Decode(json["Block"], Block);
			break;
		}

		case MessageType::RES_FULL_BLOCKCHAIN:
		case MessageType::RES_PARTIAL_BLOCKCHAIN: {
			Blocks = new std::vector<struct Block*>();
			Block::Decode(json["Blocks"], *Blocks);
			break;
		}

		case MessageType::REQ_FULL_BLOCKCHAIN:
		case MessageType::REQ_LAST_BLOCK:
			break;

		case MessageType::REQ_PARTIAL_BLOCKCHAIN:
			Index = (size_t)json["Index"].int_value();
			break;

		case MessageType::RES_NODE_LIST:
		{
			std::vector<json11::Json> nodesJson = json["Nodes"].array_items();
			auto nodesEnd = nodesJson.end();

			Nodes = new std::vector<std::string>();
			for (auto it = nodesJson.begin(); it != nodesEnd; ++it)
				Nodes->push_back(it->string_value());

			break;
		}

		case MessageType::REQ_NODE_LIST:
			break;

		case MessageType::RES_NODE_INFO:
		{
			Blocks = new std::vector<struct Block*>();
			Block::Decode(json["Blocks"], *Blocks);

			std::vector<json11::Json> nodesJson = json["Nodes"].array_items();
			auto nodesEnd = nodesJson.end();

			Nodes = new std::vector<std::string>();
			for (auto it = nodesJson.begin(); it != nodesEnd; ++it)
				Nodes->push_back(it->string_value());

			break;
		}

		case MessageType::REQ_NODE_INFO:
			Index = (size_t)json["Index"].int_value();
			TcpPort = (size_t)json["TcpPort"].int_value();
			HttpPort = (size_t)json["HttpPort"].int_value();
			break;
		}
	}
	else
		Type = MessageType::INVALID_MESSAGE;
}

NodeMessage::~NodeMessage()
{
	if (Type == MessageType::RES_FULL_BLOCKCHAIN ||
		Type == MessageType::RES_PARTIAL_BLOCKCHAIN)
		delete Blocks;

	if (Type == MessageType::RES_NODE_LIST)
		delete Nodes;
}
