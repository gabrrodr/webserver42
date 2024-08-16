#include "SimpleServer.hpp"

SimpleServer::SimpleServer(int domain, int serviceType, int protocol, 
            int port, u_long ip, int backlog)
{
    socket = new ListeningSocket(domain, serviceType, protocol, 
                port, ip, backlog);
}

ListeningSocket*    SimpleServer::get_socket() {
    return socket;
}