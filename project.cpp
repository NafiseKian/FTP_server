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
#include <map>
#include <fstream>
#include <sstream>
#include <dirent.h> 
#include <sys/stat.h>
#include <mutex>

#define PORT 8080
//--------------------------------------------------------------------------------------------------

std::map<std::string, std::string> users ;
std::string directory = "files/";
std::mutex usersMutex;
std::mutex userStructMutex; 


class userStruct
{
    std::string user_name ;
    std::string password ;
    bool auth_flag  ;

    public:

    userStruct(std::string user ,std::string  pass , bool flag){
        user_name = user;
        password = pass; 
        auth_flag = flag ;
    }

    void setUserName(std::string name )
    {
        this->user_name = name ;
    }

    void setPass(std::string pass )
    {
        this->password = pass ;
    }

    void setFlag(bool flag )
    {
        this->auth_flag = flag ;
    }

    bool getFlag()
    {
        return this->auth_flag ;
    }
};

//-------------------------------------------read user names and passwords----------------------------


std::map<std::string, std::string> readUsersData(const std::string& filename) {

    std::map<std::string, std::string> credentials;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line))
    {
        std::size_t delimiterPos = line.find(':');
        std::string username = line.substr(0, delimiterPos);
        std::string password = line.substr(delimiterPos + 1);

        credentials[username] = password;
    }

    return credentials;
}
//--------------------------------------------handle connection---------------------------------------

void handleConnection(int socket)
{
    char buff[512] = "";
    int rcnt ; 
    std::string command ;
    std::cout << "New connection received on socket: " << socket << std::endl;
    userStruct new_user = userStruct("" , "", 0);

    std::string deniedMsg ="You are not allowed to do this action try to authorize first";
    std::string welcomeMsg = "Welcome ! autheticate your self by entering -> user <Username> <PASS>\n";
    
    send(socket, welcomeMsg.c_str(), welcomeMsg.length(), 0);

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
        if (command.substr(0, 4).compare("user") == 0)
        {
            std::cout << "we received a user command." << std::endl;
            std::istringstream iss(command);
            std::string cmd, user, pass;
            iss >> cmd >> user >> pass; // Extract command, username, password
            
            std::lock_guard<std::mutex> lock(userStructMutex);
            auto it = users.find(user);
            if (it != users.end() && it->second == pass)
            {
            // Correct credentials
            std::string msg = "200 User " + user + " granted access.\n";
            new_user.setUserName(user);
            new_user.setPass(pass);
            new_user.setFlag(1);
            send(socket, msg.c_str(), msg.length(), 0);
            }
            else
            {
            // Incorrect credentials
            std::string msg = "401 Unauthorized.\n";
            new_user.setFlag(0);
            send(socket, msg.c_str(), msg.length(), 0);
            }
        }
        else if (command.substr(0, 4).compare("list") == 0)
        {
            std::cout << "we received a LIST command." << std::endl;
            if (new_user.getFlag() == 1)
            {
                DIR *dir;
                struct dirent *ent;
                struct stat fileStat;
                std::lock_guard<std::mutex> lock(usersMutex);
                if ((dir = opendir(directory.c_str())) != NULL)
                {
                    while ((ent = readdir(dir)) != NULL)
                    {
                        std::string filePath = directory + ent->d_name;
                        if (stat(filePath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode))
                        {
                            std::string fileInfo = std::string(ent->d_name) + " " + std::to_string(fileStat.st_size) + "\n";
                            send(socket, fileInfo.c_str(), fileInfo.length(), 0);
                        }
                    }
                    closedir(dir);
                    send(socket, ".\n", 2, 0); // send '.' to indicate the end of the list
                } else {
                    // Error opening directory
                    std::string errorMsg = "Error opening directory.\n";
                    send(socket, errorMsg.c_str(), errorMsg.length(), 0);
                }
            }else{
               send(socket, deniedMsg.c_str(), deniedMsg.length(), 0);
            }
        }
        else if (command.substr(0, 3).compare("get") == 0)
        {
            std::cout << "we received a GET command." << std::endl;
            if (new_user.getFlag() == 1){
                
                std::istringstream iss(command);
                std::string cmd, filename;
                iss >> cmd >> filename; 
                std::lock_guard<std::mutex> lock(usersMutex);
                std::ifstream file(directory + filename, std::ifstream::binary);
                if (file)
                {
                    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                    content += "\r\n.\r\n"; 
                    send(socket, content.c_str(), content.length(), 0);
                }
                else
                {
                    std::string errorMsg = "404 File Not Found.\n";
                    send(socket, errorMsg.c_str(), errorMsg.length(), 0);
                }
            }
            else
            {
               send(socket, deniedMsg.c_str(), deniedMsg.length(), 0);
            }
        }
        else if (command.substr(0, 3).compare("put") == 0)
        {
            std::cout << "we received a PUT command." << std::endl;
            if (new_user.getFlag() == 1)
            {

                std::istringstream iss(command);
                std::string cmd, filename;
                iss >> cmd >> filename; 
                
                std::lock_guard<std::mutex> lock(usersMutex);
                std::ofstream file(directory + filename, std::ofstream::binary);
                if (file)
                {
                    std::string successMsg = "200 " + filename + " file retrieved by server and was saved.\n";
                    send(socket, successMsg.c_str(), successMsg.length(), 0);
                }
                else
                {
                    std::string errorMsg = "400 File cannot be saved on server side.\n";
                    send(socket, errorMsg.c_str(), errorMsg.length(), 0);
                }

            }
            else
            {
               send(socket, deniedMsg.c_str(), deniedMsg.length(), 0);
            }
        }
        else if (command.substr(0, 3).compare("del") == 0)
        {
            std::cout << "we received a DEL command." << std::endl;
            if (new_user.getFlag() == 1)
            {
                std::istringstream iss(command);
                std::string cmd, filename;
                iss >> cmd >> filename; // Extract command and filename
                
                std::lock_guard<std::mutex> lock(usersMutex);
                std::string filePath = directory + filename;
                if (remove(filePath.c_str()) == 0)
                {
                    std::string successMsg = "200 File " + filename + " deleted.\n";
                    send(socket, successMsg.c_str(), successMsg.length(), 0);
                }
                else
                {
                    std::string errorMsg = "404 File " + filename + " not found on the server.\n";
                    send(socket, errorMsg.c_str(), errorMsg.length(), 0);
                }

            }
            else
            {
               send(socket, deniedMsg.c_str(), deniedMsg.length(), 0);
            }
        }
        else if (command.substr(0, 4).compare("quit") == 0)
        {
            std::string byeMsg = "Goodbye user ! " ;
            std::cout << "we received a QUIT command." << std::endl;
            send(socket, byeMsg.c_str(), byeMsg.length(), 0);
            close(socket);
            
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
    std::string dir, password;
    bool dFlag = false, pFlag = false, uFlag = false;

    users = readUsersData("./credentials.txt");

    int opt;
    while((opt = getopt(argc, argv, "d:p:u:")) != -1) {
        switch(opt) {
            case 'd':
                dir = optarg;
                std::cout<<"your directory is : "<<dir<<std::endl ;
                directory = dir ;
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
