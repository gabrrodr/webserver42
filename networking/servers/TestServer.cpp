#include "TestServer.hpp"
#include <string.h>

TestServer::TestServer() : SimpleServer(AF_INET, SOCK_STREAM, 0, 8081, INADDR_ANY, 10) {
    launch();
}

//accept(int server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)
void    TestServer::accepter() {
    struct sockaddr address = get_address();
    int addrlen = sizeof(address);
    new_socket = accept(get_sock(), 
            (struct sockaddr *)&address, (socklen_t *)&sizeof(address));
    read(new_socket, buffer, 30000);
}

void    TestServer::handler() {
    stad::cout << buffer << std::endl;
}

void    TestServer::responder() {
    std::string hello = "Hello from server";
    write(new_socket, hello, length(hello));
    close(new_socket);
}

void    TestServer::launch() {
    while (true) {
        std::cout << "***** WAITING *****" << std::endl;
        accepter();
        handler();
        responder();
        std::cout << "***** DONE *****" << std::endl;
    }
}