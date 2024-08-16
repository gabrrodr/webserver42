#ifndef TEST_SERVER_HPP
#define TEST_SERVER_HPP

#include "../MainHeader.hpp"
#include "SimpleServer.hpp"

class TestServer : public SimpleServer {
    private:
        char    buffer[30000] = {0};
        int     new_socket;
        void    accepter();
        void    handler();
        void    responder();
    public:
        TestServer();
        void    launch();
};

#endif