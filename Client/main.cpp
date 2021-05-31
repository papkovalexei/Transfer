#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fstream>
#include <sys/signal.h>
#include <unistd.h>
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <fstream>
#include <regex>
#include <iostream>
#include <mutex>
#include <vector>

bool sendAll(int sock, const char* buffer, size_t length)
{
    char *ptr = (char*) buffer;

    while (length > 0)
    {
        int i = send(sock, ptr, length, 0);

        if (i < 1) 
            return false;

        ptr += i;
        length -= i;
    }

    return true;
}

int main(int argc, char const *argv[]) // port address path
{
    if (argc != 4)
    {
        std::cerr << "Input port, address, file" << std::endl;
        return -1;
    }    

    std::string fileName = argv[3];
    std::string path = argv[3];
    size_t pos = 0;
    while ((pos = fileName.find('/')) != std::string::npos) fileName.erase(0, pos + 1);

    sockaddr_in address;
    int sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_port = htons(std::atoi(argv[1]));
    inet_pton(AF_INET, argv[2], &(address.sin_addr));

    connect(sock, (sockaddr*)&address, sizeof(address));

    char buffer[12];

    std::string sizeName = std::to_string(fileName.size());
    send(sock, sizeName.c_str(), sizeof(char)*sizeName.size(), 0);
    recv(sock, buffer, 12, 0); // received

    sendAll(sock, fileName.c_str(), sizeof(char)*fileName.size());
    recv(sock, buffer, 12, 0); // create file

    FILE* file = fopen(path.c_str(), "rb");

    fseek(file, 0L, SEEK_END);
    size_t len = ftell(file);
    rewind(file);

    std::string sizeFile = std::to_string(len);
    send(sock, sizeFile.c_str(), sizeof(char)*sizeFile.size(), 0);
    recv(sock, buffer, 12, 0); // accept

    const int chunk = 512;
    char data[chunk];

    while (fread(data, sizeof(char), chunk, file))
        sendAll(sock, data, chunk);

    recv(sock, buffer, 12, 0); // received
    fclose(file);

    shutdown(sock, 2);
    close(sock);

    return 0;
}
