#pragma once

#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <blockchainserver.h>
#include <uv.h>

struct HttpClient
{
	void * Handle; // uv_tcp_t
	void * Parser; // http_parser
	void * Async; // uv_async_t

	std::map<std::string, std::string> Headers;
	std::string LastHeaderItem;
	std::string Url;
	std::string RequestBuffer;
	std::stringstream ResponseBuffer;
	BlockChainServerPimpl *ServerPimpl;

	HttpClient();
	void Send();

	static void onClose(uv_handle_t*);
	static void sendAsync(uv_async_t *);
	static void afterWrite(uv_write_t*, int);
};