#ifndef BINDING_SOCKET_HPP
#define BINDING_SOCKET_HPP

#include "Socket.hpp"

namespace GS {
class BindingSocket : public Socket {
    public:
    //Constructor
        BindingSocket(int domain, int serviceType, int protocol, int port, u_long ip);
        //virtual function from parent Socket
        int connect_to_network(int sock, struct sockaddr_in address);
};
}

#endif