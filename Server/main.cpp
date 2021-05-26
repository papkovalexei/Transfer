#define KILL_CHILD 0
#define PATH_TO_PID "/home/alexei/pid.t"

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
    std::ofstream file(PATH_TO_PID);

    file << getpid();

    file.close();
}

void deletePID()
{
    remove(PATH_TO_PID);
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

    while (Server::getInstance()->getState() == Server::STATE::WORK) {}

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
        chdir("~/");

        return startWork(argv[1]);
    }
    else 
    {
        return 0;
    }
}
