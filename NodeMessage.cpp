#include <NodeMessage.h>

NodeMessage::NodeMessage()
{

}

NodeMessage::NodeMessage(std::string const& message)
{
    std::string err;
    json11::Json const json = json11::Json::parse(message, err);

    if (err.empty()) {
        Type = (MessageType) json["Type"].int_value();

        switch (Type) {
            case MessageType::RES_LAST_BLOCK: {
                Block::Decode(json["Block"], Block);
                break;
            }

            case MessageType::RES_FULL_BLOCKCHAIN: {
                Blocks = new std::vector<struct Block*>();
                Block::Decode(json["Blocks"], *Blocks);
                break;
            }

            case MessageType::RES_PARTIAL_BLOCKCHAIN: {
                Blocks = new std::vector<struct Block*>();
                Block::Decode(json["Blocks"], *Blocks);
                break;
            }

            case MessageType::REQ_FULL_BLOCKCHAIN:
            case MessageType::REQ_LAST_BLOCK:
                break;

            case MessageType::REQ_PARTIAL_BLOCKCHAIN:
                Index = (size_t) json["Index"].int_value();
                break;

            case MessageType::REQ_NODE_LIST:
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

            case MessageType::RES_INFO:
                break;
        }
    }
}

NodeMessage::~NodeMessage()
{

}
