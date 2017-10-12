#define ASIO_STANDALONE

#include <WebSocketServer.h>
#include <WebSocketPPServer.h>
#include <Config.h>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/server.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using websocketpp::lib::thread;
using websocketpp::lib::mutex;
using websocketpp::lib::lock_guard;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;

typedef std::set<connection_hdl,std::owner_less<connection_hdl> > con_list;

enum action_type {
    SUBSCRIBE,
    UNSUBSCRIBE,
    MESSAGE
};

struct action {
    action(action_type t, connection_hdl h) : type(t), hdl(h) {}
    action(action_type t, connection_hdl h, server::message_ptr m)
    : type(t), hdl(h), msg(m) {}
    
    action_type type;
    websocketpp::connection_hdl hdl;
    server::message_ptr msg;
};

struct WebSocketPPServerPimpl
{
    thread* mainThread;
    thread* executionThread;
    WebSocketPPServer* socketServer;
    int port;
    server server;
    con_list connections;
    std::queue<action> actions;
    
    mutex action_lock;
    mutex connection_lock;
    condition_variable action_cond;

    WebSocketPPServerPimpl()
    {
        socketServer = NULL;
        mainThread = NULL;
        executionThread = NULL;
    }
    
    void onOpen(connection_hdl hdl) {
        {
            lock_guard<mutex> guard(action_lock);
            actions.push(action(SUBSCRIBE,hdl));
        }
        action_cond.notify_one();
    }
    
    void onClose(connection_hdl hdl) {
        {
            lock_guard<mutex> guard(action_lock);
            actions.push(action(UNSUBSCRIBE,hdl));
        }
        action_cond.notify_one();
    }
    
    void onMessage(connection_hdl hdl, server::message_ptr msg)
    {
        {
            lock_guard<mutex> guard(action_lock);
            actions.push(action(MESSAGE,hdl,msg));
        }
        action_cond.notify_one();
    }

    void Start() {
        executionThread = new thread(bind(&WebSocketPPServerPimpl::internalStart, this));

        server.listen(port);
        server.start_accept();

        try {
            server.run();
        } catch (const std::exception & e) {
            std::cout << e.what() << std::endl;
        }
    }

private:
    void internalStart() {
        while(1) {
            unique_lock<mutex> lock(action_lock);
            
            while(actions.empty()) {
                action_cond.wait(lock);
            }
            
            action a = actions.front();
            actions.pop();
            
            lock.unlock();
            
            if (a.type == SUBSCRIBE) {
                lock_guard<mutex> guard(connection_lock);
                connections.insert(a.hdl);
            } else if (a.type == UNSUBSCRIBE) {
                lock_guard<mutex> guard(connection_lock);
                connections.erase(a.hdl);
            } else if (a.type == MESSAGE) {
                lock_guard<mutex> guard(connection_lock);
                
                con_list::iterator it;
                for (it = connections.begin(); it != connections.end(); ++it) {
                    server.send(*it,a.msg);
                }
            } else {
            }
        }
    }
};

void WebSocketPPServer::Start(int port) {
    pimpl->port = port;
    pimpl->mainThread = new thread(bind(&WebSocketPPServerPimpl::Start, pimpl));

	INFO << "WebSocket Server Started";
}

void WebSocketPPServer::Init() {
    pimpl = new WebSocketPPServerPimpl;
    pimpl->socketServer = this;

    pimpl->server.init_asio();

    pimpl->server.set_open_handler(bind(&WebSocketPPServerPimpl::onOpen,pimpl,::_1));
    pimpl->server.set_close_handler(bind(&WebSocketPPServerPimpl::onClose,pimpl,::_1));
    pimpl->server.set_message_handler(bind(&WebSocketPPServerPimpl::onMessage, pimpl,::_1,::_2));
}

void WebSocketPPServer::BroadcastBlock(Block*block)
{
	if (block != NULL)
		INFO << block->Encode();
	else
		INFO << "Block is empty";
}

void WebSocketPPServer::Stop()
{

}
