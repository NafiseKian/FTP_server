/*
** Author : Nafise Kian
** Date : 20.12.2023
** project outline : mplementing a modified version of the File Transfer Protocol (FTP) for transferring files 
** between two machines
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <vector>
#include <thread>

#define PORT 8080

//--------------------------------------------handle connection---------------------------------------

void handleConnection(int socket)
{
    char buff[512] = "";
    std::string command ;
    std::cout << "New connection received on socket: " << socket << std::endl;

    while (true)
    {
        memset(buff , 0 , 512);
        int ret = read(socket , buff , 512);
        if (ret < 0)
        {
            std::cerr<<"error reading the command from socket"<<std::endl;
            break;
        }
        command = buff ;

        if (command.substr(0, 4).compare("list") == 0)
        {
            std::cout << "we received a LIST command." << std::endl;
        }
        else if (command.substr(0, 3).compare("get") == 0)
        {
            std::cout << "we received a GET command." << std::endl;
        }
        else if (command.substr(0, 3).compare("put") == 0)
        {
            std::cout << "we received a PUT command." << std::endl;
        }
        else if (command.substr(0, 3).compare("del") == 0)
        {
            std::cout << "we received a DEL command." << std::endl;
        }
        else if (command.substr(0, 4).compare("quit") == 0)
        {
            std::cout << "we received a QUIT command." << std::endl;
        }
        else
        {
            std::cout << "Unknown command for the server ! " << std::endl;
        }
    }
    
    close(socket);
}
//----------------------------------------------main--------------------------------------------------

int main(int argc, char *argv[]) 
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int port = PORT ;
    std::string directory, password;
    bool dFlag = false, pFlag = false, uFlag = false;

    int opt;
    while((opt = getopt(argc, argv, "d:p:u:")) != -1) {
        switch(opt) {
            case 'd':
                directory = optarg;
                std::cout<<"your directory is : "<<directory<<std::endl ;
                dFlag = true ;
                break;
            case 'p':
                port = atoi(optarg);
                std::cout<<"your port is : "<<port<<std::endl ;
                pFlag = true ;
                break;
            case 'u':
                password = optarg;
                std::cout<<"your password is : "<<password<<std::endl ;
                uFlag = true ; 
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -d directory -p port -u password\n";
                exit(EXIT_FAILURE);
        }
    }

    if (!dFlag || !pFlag || !uFlag) {
        std::cerr << "Error: Missing required arguments.\n";
        std::cerr << "Usage: " << argv[0] << " -d directory -p port -u password\n";
        exit(EXIT_FAILURE);
    }


    // step 1 : creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // step 2 : binding the socket to the specified PORT
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // step 3 : listen for client connections
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // step 4 : accepting client connection
    std::cout << "File server listening on localhost port " << port << std::endl;
    
    std::vector<std::thread> threads;

    while(true) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        // adding new thread to handle the connection
        threads.push_back(std::thread(handleConnection, new_socket));
    }

    close(new_socket);

    for (auto& t : threads)
    {
        if (t.joinable()) {
            t.join();
        }
    }
    
    close(server_fd);

    return 0;
}
