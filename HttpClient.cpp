#include <HttpClient.h>

HttpClient::HttpClient()
{
	Handle = nullptr;
	Parser = nullptr;
	Async = nullptr;
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
	uv_buf_t resbuf;
	resbuf.base = (char *)res.c_str();
	resbuf.len = res.size();

	uv_write_t *write_req = new uv_write_t;

	int r = uv_write(write_req, (uv_stream_t *)client->Handle, &resbuf, 1, afterWrite);
	uv_close((uv_handle_t*)client->Async, NULL);
}



void HttpClient::Send()
{
	uv_async_send((uv_async_t*)this->Async);
}

void HttpClient::afterWrite(uv_write_t* req, int status) {
	if (!uv_is_closing((uv_handle_t*)req->handle))
		uv_close((uv_handle_t*)req->handle, onClose);
}

void HttpClient::onClose(uv_handle_t* handle) {
	HttpClient* client = (HttpClient*)handle->data;

	if (client->Parser != nullptr)
		delete client->Parser;

	if (client->Handle != nullptr)
		delete client->Handle;

	if (client->Async != nullptr)
		delete client->Async;

	delete client;
}
