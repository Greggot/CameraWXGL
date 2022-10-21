#include <SkyBlue/Types.hpp>
#include <TCP/client.hpp>
#include <UDP/server.hpp>

#include <algorithm>
#include <string>
#include <iostream>
#include <thread>
#include <cstdio>

SkyBlue::buffer tx;
uint8_t buffer[1440];

void sendfile(const char* filename, TCP::client& client)
{
    FILE* f = fopen(filename, "r");
    if(f == nullptr)
    {
        printf("File \'%s\' not found!\n", filename);
        return;
    }

    fread(&tx, sizeof(uint8_t), sizeof(tx), f);
    client.Send(&tx, tx.length + SkyBlue::BUFFER_HEADER_SiZE());
}

int main()
{
    TCP::client client;
    UDP::server server({{ 192, 168, 1, 134}, 5555});
    client.Connect({{192, 168, 1, 176}, 4444});

    FILE* log = fopen("log.log", "a");
    std::thread([&client, log](){
        std::string command;
        while(true)
        {
            std::cin >> command;
            if(command == "exit")
            {
                fclose(log);
                exit(0);
            }
            else
                sendfile(command.c_str(), client);
        }
    }).detach();

    std::thread([&server, log](){
        std::string command;
        while(true)
        {
            int length = server.Receive(buffer, sizeof(buffer));
            if(length < 0)
                exit(1);

            fwrite(buffer, sizeof(uint8_t), length, log);
            printf("UDP received %u: \n", length);
        }
    }).detach();

    printf("  Connected to server!\n");
    while(true)
    {
        int length = client.Receive(buffer, sizeof(buffer));
        if(length < 0)
            exit(1);
        
        fwrite(buffer, sizeof(uint8_t), length, log);
        printf("TCP received %u: \n", length);
    }
}