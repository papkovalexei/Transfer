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

    send(sock, fileName.c_str(), sizeof(fileName.c_str()), 0);

    char buffer[12];

    recv(sock, buffer, 12, 0); // create file

    FILE* file = fopen(path.c_str(), "rb");

    fseek(file, 0L, SEEK_END);
    size_t len = ftell(file);
    rewind(file);

    std::vector<char> input(len + 1);

    fread(input.data(), 1, len, file);

    input[len] = '\0';
    send(sock, input.data(), len + 1, 0);

    fclose(file);

    shutdown(sock, 2);
    close(sock);

    return 0;
}
