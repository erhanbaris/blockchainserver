#pragma once

#include <WebSocketServer.h>


#include <iostream>
#include <set>

using namespace std;



struct WebSocketPPServerPimpl;
class WebSocketPPServer : public WebSocketServer {
public:
    void Init();

    void Start(int port);
    void Stop();

private:
    WebSocketPPServerPimpl *pimpl;
};
