#pragma once

class WebSocketServer {
public:
    virtual void Init() = 0;

    virtual void Start(int port) = 0;
    virtual void Stop() = 0;
};
