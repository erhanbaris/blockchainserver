#pragma once

#include <string>
#include <functional>

namespace blockchain {
    namespace tcp {
        class TcpClient {
        public:

            typedef std::function<void(std::string const &, TcpClient &)> MessageCallback;
            typedef std::function<void(TcpClient &)> DisconnectCallback;
            typedef std::function<void(TcpClient &)> ConnectCallback;

            virtual void Connect(std::string, size_t port) = 0;
            virtual void Disconnect() = 0;
            virtual bool IsConnected() = 0;
            virtual void Send(std::string const &&) = 0;
            virtual std::string GetRemoteAddress() = 0;
            virtual size_t GetRemotePort() = 0;
            virtual void SetOnMessage(MessageCallback) = 0;
            virtual void SetOnDisconnect(DisconnectCallback) = 0;
            virtual void SetOnConnect(ConnectCallback) = 0;
        };
    }
}