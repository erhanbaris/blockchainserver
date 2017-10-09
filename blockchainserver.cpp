#include "blockchainserver.h"
#include <map>
#include <uv.h>
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <Block.h>
#include <HttpClient.h>
#include "http_parser.h"
#include "Tools.h"

namespace
{
    static http_parser_settings * parser_settings;
    static uv_loop_t* uv_loop;
}




/* BLOCK CHAIN SERVER PIMPL*/
struct BlockChainServerPimpl
{
        BlockChainServer * server; // server
        Block* firstBlock; // firstBlock
        Block* lastBlock; // lastBlock
        size_t totalBlocks;
        uv_tcp_t* tcpServer;
        uv_timer_t elapsedTimer;

        BlockChainServerPimpl(BlockChainServer * pServer)
        {
            server = pServer;
            parser_settings = new http_parser_settings;
            tcpServer = new uv_tcp_t;
            tcpServer->data = this;
            totalBlocks = 0;
        }

        static void alloc_cb(uv_handle_t * /*handle*/, size_t suggested_size, uv_buf_t* buf) {
            *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
        }

        static void on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t * buf) {
            ssize_t parsed;
            HttpClient* client = (HttpClient*)tcp->data;
            if (nread >= 0) {
                parsed = (ssize_t)http_parser_execute((http_parser*)client->Parser, parser_settings, buf->base, nread);
                if (((http_parser*)client->Parser)->upgrade) {
                    uv_close((uv_handle_t*)client->Handle, HttpClient::onClose);
                }
                else if (parsed < nread) {
                    uv_close((uv_handle_t*)client->Handle, HttpClient::onClose);
                }
            }
            else {
                if (nread != UV_EOF) {
                }
                //uv_close((uv_handle_t*)client->Handle, HttpClient::on_close);
            }
            free(buf->base);
        }

        static int on_message_begin(http_parser* /*parser*/) {
            return 0;
        }

        static int on_headers_complete(http_parser* /*parser*/) {
            return 0;
        }

        static int on_url(http_parser* parser, const char* url, size_t length) {

            HttpClient* client = (HttpClient*)parser->data;
            struct http_parser_url u;
            int result = http_parser_parse_url(url, length, 0, &u);
            if (result) {
                fprintf(stderr, "\n\n*** failed to parse URL %s ***\n\n", url);
                return -1;
            }
            else {
                if ((u.field_set & (1 << UF_PATH))) {
                    const char * data = url + u.field_data[UF_PATH].off;
                    client->Url = std::string(data, u.field_data[UF_PATH].len);
                }
            }
            return 0;
        }

        static int on_header_field(http_parser* /*parser*/, const char* at, size_t length) {
            return 0;
        }

        static int on_header_value(http_parser* /*parser*/, const char* at, size_t length) {
            return 0;
        }

        static int on_body(http_parser* parser, const char* at, size_t length) {
            HttpClient* client = (HttpClient*)parser->data;
            client->RequestBuffer = std::string(at, length);
            return 0;
        }

        Block * generateNewBlock(char const * data)
        {
            Block * block = new Block;
            block->Index = lastBlock->Index + 1;
            block->PreviousHash = lastBlock;
            block->TimeStamp = getUtcMilliseconds();
            block->Data = strdup(data);
            block->SetHash();

            ++totalBlocks;
            lastBlock = block;

            return block;
        }

        bool isValidNewBlock(Block * newBlock, Block * previousBlock)
        {
            if (previousBlock->Index + 1 != newBlock->Index)
                return false;
            else if (previousBlock->Hash != newBlock->PreviousHash->Hash)
                return false;
            else if (newBlock->CalculateHash() != newBlock->Hash)
                return false;
            return true;
        }

        void addCall(HttpClient* client)
        {
            Block * block = generateNewBlock(client->RequestBuffer.c_str());
            client->ResponseBuffer << block->Hash;
        }

        void totalBlockCall(HttpClient* client)
        {
            client->ResponseBuffer << std::to_string(totalBlocks);
        }

        static int on_message_complete(http_parser* parser) {
            HttpClient* client = (HttpClient*)parser->data;

            if (client->Url == "/add")
                client->ServerPimpl->addCall(client);
            if (client->Url == "/totalblocks")
                client->ServerPimpl->totalBlockCall(client);
            else
                client->ResponseBuffer << client->Url;

            client->Send();
            return 0;
        }

        static void on_connect(uv_stream_t* server_handle, int status) {
            HttpClient *client = new HttpClient;
            client->Handle = new uv_tcp_t;
            client->Parser = new http_parser;
            client->Async = new uv_async_t;
            client->ServerPimpl = (BlockChainServerPimpl *)server_handle->data;

            uv_tcp_init(uv_loop, (uv_tcp_t*)client->Handle);
            uv_async_init(uv_loop, ((uv_async_t*)client->Async), HttpClient::sendAsync);
            http_parser_init((http_parser*)client->Parser, HTTP_REQUEST);

            ((http_parser*)client->Parser)->data = client;
            ((uv_tcp_t*)client->Handle)->data = client;
            ((uv_async_t*)client->Async)->data = client;

            int r = uv_accept(server_handle, (uv_stream_t*)client->Handle);

            uv_read_start((uv_stream_t*)client->Handle, alloc_cb, on_read);
        }
};

/* BLOCK CHAIN SERVER */
BlockChainServer::BlockChainServer()
    :pimpl(new BlockChainServerPimpl(this))
{
    pimpl->firstBlock = new Block;
    pimpl->firstBlock->Index = 1;
    pimpl->firstBlock->PreviousHash = NULL;
    pimpl->firstBlock->TimeStamp = 1507493846081;
    pimpl->firstBlock->Data = strdup("Init block");
    pimpl->firstBlock->SetHash();

    ++pimpl->totalBlocks;
    pimpl->lastBlock = pimpl->firstBlock;
}

BlockChainServer::~BlockChainServer()
{
    delete pimpl;
}

void BlockChainServer::Start(size_t port)
{
	parser_settings->on_url = pimpl->on_url;

	parser_settings->on_message_begin = pimpl->on_message_begin;
	parser_settings->on_headers_complete = pimpl->on_headers_complete;
	parser_settings->on_message_complete = pimpl->on_message_complete;

	parser_settings->on_header_field = pimpl->on_header_field;
	parser_settings->on_header_value = pimpl->on_header_value;
	parser_settings->on_body = pimpl->on_body;

	uv_loop = uv_default_loop();
	uv_tcp_init(uv_loop, pimpl->tcpServer);
	struct sockaddr_in address;
	uv_ip4_addr("0.0.0.0", port, &address);
	uv_tcp_bind(pimpl->tcpServer, (const struct sockaddr*)&address, 0);
	uv_listen((uv_stream_t*)pimpl->tcpServer, 1000, BlockChainServerPimpl::on_connect);

	uv_run(uv_loop, UV_RUN_DEFAULT);
}

void BlockChainServer::Stop()
{ 
}