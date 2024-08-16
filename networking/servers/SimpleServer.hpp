#ifndef SIMPLE_SERVER_HPP
#define SIMPLE_SERVER_HPP

#include "../MainHeader.hpp"

class SimpleServer {
    private:
        ListeningSocket*    socket;
        virtual void    accepter() = 0;
        virtual void    handler() = 0;
        virtual void    responder() = 0;
    public:
        virtual void    launch() = 0;
        SimpleServer(int domain, int serviceType, int protocol, int port, 
                u_long ip, int backlog);
        ListeningSocket*    get_socket();
};

#endif 