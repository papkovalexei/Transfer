#ifndef H_SERVER
#define H_SERVER

#include <boost/noncopyable.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fstream>
#include <sys/signal.h>
#include <unistd.h>
#include <cstdio>
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <regex>
#include <iostream>
#include <mutex>
#include <vector>

class Server : boost::noncopyable
{
public:
    using sock = int;

    enum class STATE
    {
        INPUT_CONF,
        READY_INIT,
        BAD_BIND,
        BAD_SOCK,
        WORK,
        CLOSE,
    };

    Server();
    ~Server();

    static Server* getInstance();

    STATE start();
    void stop();

    STATE getState();

    void setConfig(int port, std::string address);
private:
    void _listenAccept();

    void _saveFile(sock socket);

    void _enableKeepalive(sock socket);

    void _deleteClient(sock socket);

    STATE _state;
    sockaddr_in _addr;
    sock _socket;
    std::map<sock, std::thread> _clients;
    std::recursive_mutex _mutex_clients;
    std::thread _handle_accept;

    static Server* _server;
};

#endif