#pragma once

#include <string>
#include <functional>

namespace blockchain {
    namespace tcp {

        class TcpClient;

        typedef std::function<void(std::string const &, TcpClient &)> MessageCallback;
        typedef std::function<void(TcpClient &)> DisconnectCallback;
        typedef std::function<void(TcpClient &)> ConnectCallback;
        typedef std::function<void(TcpClient &)> CloseCallback;

        class TcpClient {
        public:
            virtual void Connect(std::string, size_t port) = 0;

            virtual void Disconnect() = 0;

            virtual bool IsConnected() = 0;

            virtual void Send(std::string &) = 0;

            virtual std::string GetRemoteAddress() = 0;

            virtual void SetOnMessage(MessageCallback) = 0;

            virtual void SetOnDisconnect(DisconnectCallback) = 0;

            virtual void SetOnConnect(ConnectCallback) = 0;

            virtual void SetOnClose(CloseCallback) = 0;
        };
    }
}