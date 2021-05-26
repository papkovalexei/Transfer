#include "Server.h"

Server::Server()
{

}

Server::~Server() 
{
    stop();
}

Server* Server::_server = nullptr;

Server* Server::getInstance()
{
    if (_server == nullptr)
        _server = new Server();
    return _server;
}

Server::STATE Server::getState()
{
    return _state;
}

Server::STATE Server::start()
{
    if (_state != STATE::READY_INIT)
    {
        _state = STATE::INPUT_CONF;
        return _state;
    }

    _state = STATE::CLOSE;
    _socket = socket(AF_INET, SOCK_STREAM, 0);

    int err;

    if (_socket < 0)
    {
        std::cerr << "Bad (" << _socket << ")" << std::endl;

        _state = STATE::BAD_SOCK;
        return _state;
    }

    err = bind(_socket, (sockaddr*)&_addr, sizeof(_addr));

    if (err < 0)
    {
        std::cerr << "Bad bind: " << err << std::endl;

        _state = STATE::BAD_BIND;
        return _state;
    }

    listen(_socket, 1);


    _handle_accept = std::thread([this]{_listenAccept();});

    _state = STATE::WORK;
    return _state;
}

void Server::stop()
{
    shutdown(_socket, 2);
    close(_socket);

    for (auto& client : _clients)
    {
        char buffer[] = {'N'};

        send(client.first, buffer, sizeof(buffer), 0);

        shutdown(client.first, 2);
        close(client.first);

        client.second.join();
    }

    _state = STATE::CLOSE;
    _handle_accept.join();
}

void Server::_deleteClient(sock socket)
{
    shutdown(socket, 2);
    close(socket);
    
    auto it = _clients.find(socket);

    _mutex_clients.lock();
    it->second.join();
    _clients.erase(it);
    _mutex_clients.unlock();
}

void Server::setConfig(int port, std::string address)
{
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(port);
    inet_pton(AF_INET, address.c_str(), &(_addr.sin_addr));

    _state = STATE::READY_INIT;
}

void Server::_saveFile(sock socket)
{  
    int res = 1;
    FILE* file;
    std::string name = "";

    do
    {
        std::vector<char> buffer(1024);

        res = recv(socket, buffer.data(), buffer.size(), 0);

        if (res <= 0)
            break;

        if (name.empty())
        {
            name = buffer.data();

            file = fopen(name.c_str(), "wb");

            char buf[] = "create";
            send(socket, buf, sizeof(buf), 0);
        }
        else
        {
            fwrite(buffer.data(), 1, strlen(buffer.data()), file);
        }
    } while (res > 0);
    
    fclose(file);

    std::cout << "Start del\n";
    auto del = std::thread([this, socket]{_deleteClient(socket);});
    del.detach();
    std::cout << "Detach\n";
}

void Server::_enableKeepalive(sock socket)
{
    int yes = 1;
    setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int));
    int idle = 1;
    setsockopt(socket, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int));

    int interval = 1;
    setsockopt(socket, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int));

    int maxpkt = 10;
    setsockopt(socket, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int));
}

void Server::_listenAccept()
{
    while (_state == STATE::WORK)
    {
        sockaddr_in cs_addr;
        socklen_t cs_addrsize = sizeof(cs_addr);

        sock socket = accept(_socket, (sockaddr*)&cs_addr, &cs_addrsize);

        if (socket <= 0)
            continue;

        _mutex_clients.lock();
        _enableKeepalive(socket);
        _clients[socket] = std::thread([this, socket]{_saveFile(socket);});
        _mutex_clients.unlock();
    }
}