#define KILL_CHILD 0

#include <iostream>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <sys/stat.h>

#include "Server.h"

void createPID()
{
    std::string path = getenv("HOME");
    path += "/pid.t";
    std::ofstream file(path);

    file << getpid();

    file.close();
}

void deletePID()
{
    std::string path = getenv("HOME");
    path += "/pid.t";
    remove(path.c_str());
}

void handle(int signum)
{
    Server::getInstance()->stop();
}

int startWork(std::string path)
{
    std::ifstream input(path);

    int port;
    std::string address;

    input >> port;
    input >> address;

    sigset_t sigset;
    int siginfo;

    signal(SIGHUP, handle);
    signal(SIGTERM, handle);

    std::cout << "Work on: " << getpid() << std::endl;

    createPID();

    Server::getInstance()->setConfig(port, address);
    Server::getInstance()->start();

    while (Server::getInstance()->getState() == Server::STATE::WORK) 
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    deletePID();

    return 0;
}

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Input conf file" << std::endl;
        return -1;
    }

    int PID;

    PID = fork();

    if (PID == -1)
    {
        std::cerr << "Error fork(): " << strerror(errno) << std::endl;

        return -1;
    }
    else if (!PID)
    {
        umask(0);
        setsid();

        return startWork(argv[1]);
    }
    else 
    {
        return 0;
    }
}