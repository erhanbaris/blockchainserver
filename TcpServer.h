#pragma once

#include <vector>
#include <string>
#include <Block.h>
#include <TcpClient.h>

namespace blockchain {
    namespace tcp {

        class TcpServer {
        public:
            typedef std::function<void(std::string const &, TcpClient &)> MessageReceivedCallback;
            typedef std::function<void(TcpClient &)> ClientConnectedCallback;
			typedef std::function<void(TcpClient &)> ClientDisconnectedCallback;

            virtual void Start(size_t port) = 0;
            virtual void Stop() = 0;
            virtual size_t GetPort() = 0;
            virtual void BroadcastMessage(std::string const &) = 0;
            virtual void BroadcastMessageExpect(std::string const &, TcpClient&) = 0;
            virtual TcpClient* CreateClient() = 0;
            
            virtual void SetMessageReceived(MessageReceivedCallback) = 0;
            virtual void SetClientConnected(ClientConnectedCallback) = 0;
			virtual void SetClientDisconnected(ClientDisconnectedCallback) = 0;
        };
    }
}
