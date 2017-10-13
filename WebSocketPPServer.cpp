#define ASIO_STANDALONE

#include <WebSocketServer.h>
#include <WebSocketPPServer.h>

#include <WebSocketClient.h>
#include <WebSocketPPClient.h>

#include <Config.h>

#include <vector>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/server.hpp>

#include <json11.hpp>

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

typedef std::set<connection_hdl, std::owner_less<connection_hdl> > con_list;

enum action_type {
	SUBSCRIBE,
	UNSUBSCRIBE,
	MESSAGE
};

struct action {
	action(action_type t, std::string m) : type(t), str(m) {}
	action(action_type t, server::message_ptr m) : type(t), msg(m) {}
	action(action_type t, connection_hdl h) : type(t), hdl(h) {}
	action(action_type t, connection_hdl h, server::message_ptr m) : type(t), hdl(h), msg(m) {}

	action_type type;
	websocketpp::connection_hdl hdl;
	server::message_ptr msg;
	std::string str;
};

struct WebSocketPPServerPimpl
{
	thread* mainThread;
	thread* executionThread;

	bool executionThreadRunning;

	WebSocketPPServer* socketServer;
	size_t port;
	server server;
	con_list connections;
	std::queue<action> actions;

	mutex action_lock;
	mutex connection_lock;
	condition_variable action_cond;

	std::vector<WebSocketClient*> remoteNodeConnections;

	//Callbacks
	OnMessageReceivedCallback onMessageReceivedCallback;

	WebSocketPPServerPimpl()
	{
		executionThreadRunning = true;

		socketServer = NULL;
		mainThread = NULL;
		executionThread = NULL;
	}

	~WebSocketPPServerPimpl()
	{
		/* Stop execution thread */
		executionThreadRunning = false;
		action_cond.notify_one();

		/* Close all connections */
		auto connectionsEnd = connections.end();
		for (auto it = connections.begin(); it != connectionsEnd; ++it)
		{
			server.pause_reading(*it);
			server.close(*it, websocketpp::close::status::going_away, "");
		}
		connections.clear();

		/* Stop server accept and close port */
		server.stop_listening();
		server.stop();

		/* Bye bye thread */
		delete mainThread;
	}

	void onOpen(connection_hdl hdl) {
		{
			lock_guard<mutex> guard(action_lock);
			actions.push(action(SUBSCRIBE, hdl));
		}
		action_cond.notify_one();
	}

	void onClose(connection_hdl hdl) {
		{
			lock_guard<mutex> guard(action_lock);
			actions.push(action(UNSUBSCRIBE, hdl));
		}
		action_cond.notify_one();
	}

	void onMessage(connection_hdl hdl, server::message_ptr msg)
	{
		{
			lock_guard<mutex> guard(action_lock);
			actions.push(action(MESSAGE, hdl, msg));
		}
		action_cond.notify_one();
	}

	void Start() {
		executionThread = new thread(bind(&WebSocketPPServerPimpl::internalStart, this));

		server.listen(port);
		server.start_accept();

		try {
			server.run();
		}
		catch (const std::exception & e) {
			std::cout << e.what() << std::endl;
		}
	}

	void onClientMessage(std::string const& message, WebSocketClient& client)
	{
		if (onMessageReceivedCallback)
			onMessageReceivedCallback(message, client);
	}

	void onClientConnect(WebSocketClient& client)
	{
		INFO << "Connected";
	}

private:
	void internalStart() {
		while (executionThreadRunning) {
			unique_lock<mutex> lock(action_lock);

			while (actions.empty()) {
				action_cond.wait(lock);
			}

			action a = actions.front();
			actions.pop();

			lock.unlock();

			if (a.type == SUBSCRIBE) {
				lock_guard<mutex> guard(connection_lock);
				connections.insert(a.hdl);
			}
			else if (a.type == UNSUBSCRIBE) {
				lock_guard<mutex> guard(connection_lock);
				connections.erase(a.hdl);
			}
			else if (a.type == MESSAGE) {
				lock_guard<mutex> guard(connection_lock);

				con_list::iterator it;
				auto end = connections.end();
				for (it = connections.begin(); it != end; ++it) {
					websocketpp::lib::error_code ec;
					server.get_alog().write(websocketpp::log::alevel::app, a.str);
					server.send(*it, a.str, websocketpp::frame::opcode::text, ec);
				}
			}
			else {
			}
		}

		delete executionThread;
	}
};

void WebSocketPPServer::Start(size_t port) {
	pimpl->port = port;
	pimpl->mainThread = new thread(bind(&WebSocketPPServerPimpl::Start, pimpl));

	INFO << "WebSocket Server Started";
}

size_t WebSocketPPServer::GetPort()
{
	return pimpl->port;
}

void WebSocketPPServer::Init() {
	pimpl = new WebSocketPPServerPimpl;
	pimpl->socketServer = this;

	pimpl->server.clear_access_channels(websocketpp::log::alevel::all);

	pimpl->server.init_asio();

	pimpl->server.set_open_handler(bind(&WebSocketPPServerPimpl::onOpen, pimpl, ::_1));
	pimpl->server.set_close_handler(bind(&WebSocketPPServerPimpl::onClose, pimpl, ::_1));
	pimpl->server.set_message_handler(bind(&WebSocketPPServerPimpl::onMessage, pimpl, ::_1, ::_2));
}

void WebSocketPPServer::BroadcastBlock(Block*block)
{
	if (block != NULL)
	{
		{
			lock_guard<mutex> guard(pimpl->action_lock);
			pimpl->actions.push(action(MESSAGE, block->Encode()));
		}

		pimpl->action_cond.notify_one();
	}
	else
		INFO << "Block is empty";
}


WebSocketServer::ConnectToBlockStatus WebSocketPPServer::ConnectToNode(std::string address)
{
	auto end = pimpl->remoteNodeConnections.end();
	for (auto it = pimpl->remoteNodeConnections.begin(); it != end; ++it)
		if ((*it)->GetRemoteAddress() == address)
			return WebSocketServer::ConnectToBlockStatus::ALREADY_ADDED;

	WebSocketClient *webClient = new WebSocketPPClient;
	webClient->setOnMessage(std::bind(&WebSocketPPServerPimpl::onClientMessage, pimpl, std::placeholders::_1, std::placeholders::_2));
	// webClient->setOnConnect(onConnect);
	webClient->Connect(address);
	pimpl->remoteNodeConnections.push_back(webClient);
	INFO << "Connecting to node : " << address;
	return WebSocketServer::ConnectToBlockStatus::ADDED;
}

void WebSocketPPServer::SetMessageReceived(OnMessageReceivedCallback cb)
{
	pimpl->onMessageReceivedCallback = cb;
}


void WebSocketPPServer::DisconnectFromNode(std::string)
{

}

const std::vector<std::string> WebSocketPPServer::ConnectedNodes()
{
	std::vector<std::string> nodes;
	auto end = pimpl->remoteNodeConnections.end();

	for (auto it = pimpl->remoteNodeConnections.begin(); it != end; ++it)
		nodes.push_back((*it)->GetRemoteAddress());

	return nodes;
}

void WebSocketPPServer::Stop()
{

}
