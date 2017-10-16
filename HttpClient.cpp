#include <HttpClient.h>
#include <http_parser.h>
#include <TcpClientUv.h>

HttpClient::HttpClient()
{
	Handle = nullptr;
	Parser = nullptr;
	Async = nullptr;
	Data1 = nullptr;
	Data2 = nullptr;
}

void HttpClient::sendAsync(uv_async_t *handle) {
	HttpClient * client = (HttpClient*)handle->data;

	std::string bufferStr = client->ResponseBuffer.str();
	std::ostringstream rep;
	rep << "HTTP/1.1 200 OK\r\n"
		<< "Content-Type: application/json charset=utf-8\r\n"
		<< "Connection: close\r\n"
		<< "Content-Length: " << bufferStr.size() << "\r\n"
		<< "Access-Control-Allow-Origin: *" << "\r\n"
		<< "\r\n";
	rep << bufferStr;
	std::string res = rep.str();


	((TcpClientUv*)client->Data1)->SendAndClose(std::move(res));
}

void HttpClient::Send()
{
	std::string bufferStr = this->ResponseBuffer.str();
	std::ostringstream rep;
	rep << "HTTP/1.1 200 OK\r\n"
		<< "Content-Type: application/json charset=utf-8\r\n"
		<< "Connection: close\r\n"
		<< "Content-Length: " << bufferStr.size() << "\r\n"
		<< "Access-Control-Allow-Origin: *" << "\r\n"
		<< "\r\n";
	rep << bufferStr;
	((TcpClientUv*)this->Data1)->SendAndClose(rep.str());
}

void HttpClient::afterWrite(uv_write_t* req, int status) {
	if (!uv_is_closing((uv_handle_t*)req->handle))
		uv_close((uv_handle_t*)req->handle, onClose);
}

void HttpClient::onClose(uv_handle_t* handle) {
	HttpClient* client = (HttpClient*)handle->data;

	if (client->Parser != nullptr)
		delete (http_parser*)client->Parser;

	if (client->Handle != nullptr)
		delete (uv_tcp_t*)client->Handle;

	if (client->Async != nullptr)
		delete (uv_async_t*)client->Async;

	delete client;
}
