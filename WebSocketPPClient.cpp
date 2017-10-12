#define ASIO_STANDALONE

#include <WebSocketClient.h>
#include <WebSocketPPClient.h>

#include <WebSocketClient.h>
#include <WebSocketPPClient.h>

#include <Config.h>

#include <vector>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/client.hpp>

typedef websocketpp::client<websocketpp::config::asio> client;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using websocketpp::lib::thread;
using websocketpp::lib::mutex;
using websocketpp::lib::lock_guard;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;


struct WebSocketPPClientPimpl
{
    thread* mainThread;
    thread* executionThread;
    
    bool executionThreadRunning;
    bool isConnected = false;
    std::string address;
    
    client::connection_ptr connection;
    WebSocketPPClient* socketClient;
    client client;
    
    mutex action_lock;
    mutex connection_lock;
    condition_variable action_cond;
    
    OnConnectCallback onConnectCallback;
    OnMessageCallback onMessageCallback;
    OnDisconnectCallback onDisconnectCallback;
    OnCloseCallback onCloseCallback;
    
    WebSocketPPClientPimpl()
    {
        executionThreadRunning = true;
        
        socketClient = NULL;
        mainThread = NULL;
        executionThread = NULL;
    }
    
    ~WebSocketPPClientPimpl()
    {
        /* Stop execution thread */
        executionThreadRunning = false;
        action_cond.notify_one();
        
        /* Stop server accept and close port */
        client.stop_listening();
        client.stop();
        
        /* Bye bye thread */
        delete mainThread;
    }
    
    void onOpen(connection_hdl hdl) {
        if (onConnectCallback)
            onConnectCallback(*socketClient);
    }
    
    void onClose(connection_hdl hdl) {
        if (onCloseCallback)
            onCloseCallback(*socketClient);
    }
    
    void onMessage(connection_hdl hdl, client::message_ptr msg)
    {
        if (onMessageCallback)
        {
            if (msg->get_opcode() == websocketpp::frame::opcode::text) {
                onMessageCallback(msg->get_payload(), *socketClient);
            } else {
                onMessageCallback(websocketpp::utility::to_hex(msg->get_payload()), *socketClient);
            }
        }
    }
    
    void Start() {
        client.run();
        // executionThread = new thread(bind(&WebSocketPPClientPimpl::internalStart, this));
    }
    
private:
    void internalStart() {
        while(executionThreadRunning) {
            unique_lock<mutex> lock(action_lock);
        }
        
        delete executionThread;
    }
};


void WebSocketPPClient::Connect(std::string& address)
{
    pimpl->address = address;
    
    
    pimpl->client.clear_access_channels(websocketpp::log::alevel::frame_header);
    pimpl->client.clear_access_channels(websocketpp::log::alevel::frame_payload);
    //c.set_error_channels(websocketpp::log::elevel::none);
    
    pimpl->client.init_asio();
    
    pimpl->client.set_open_handler(bind(&WebSocketPPClientPimpl::onOpen, pimpl, ::_1));
    pimpl->client.set_fail_handler(bind(&WebSocketPPClientPimpl::onClose, pimpl, ::_1));
    pimpl->client.set_message_handler(bind(&WebSocketPPClientPimpl::onMessage, pimpl, ::_1, ::_2));
    pimpl->client.set_close_handler(bind(&WebSocketPPClientPimpl::onClose, pimpl, ::_1));
    
    websocketpp::lib::error_code ec;
    pimpl->connection = pimpl->client.get_connection(pimpl->address, ec);
    pimpl->client.connect(pimpl->connection);
    
    
    pimpl->mainThread = new thread(bind(&WebSocketPPClientPimpl::Start, pimpl));
    
    INFO << "WebSocket Client Started";
}

void WebSocketPPClient::Disconnect()
{
    if (IsConnected())
    {
        
    }
}

WebSocketPPClient::WebSocketPPClient()
{
    pimpl = new WebSocketPPClientPimpl;
    pimpl->socketClient = this;
}

std::string WebSocketPPClient::GetRemoteAddress()
{
    return pimpl->address;
}

bool WebSocketPPClient::IsConnected()
{
    return pimpl->isConnected;
}

void WebSocketPPClient::Send(std::string& message)
{
    
}

void WebSocketPPClient::setOnConnect(OnConnectCallback cb)
{
    pimpl->onConnectCallback = cb;
}

void WebSocketPPClient::setOnMessage(OnMessageCallback cb)
{
    pimpl->onMessageCallback = cb;
}

void WebSocketPPClient::setOnDisconnect(OnDisconnectCallback cb)
{
    pimpl->onDisconnectCallback = cb;
}

void WebSocketPPClient::setOnClose(OnCloseCallback cb)
{
    pimpl->onCloseCallback = cb;
}
