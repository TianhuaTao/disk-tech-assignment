#pragma once
#include <functional>
class NetworkAgent {
public:
    enum Role { UNKNOWN,
                SERVER,
                CLIENT };

private:
    Role role = UNKNOWN;

public:
    NetworkAgent();
    ~NetworkAgent();

    bool isConnected();
    int listenAsync(const char *address, int port, std::function<void()> callback);
    int connectAsync(const char *address, int port, std::function<void()> callback);
    int connect();
};
