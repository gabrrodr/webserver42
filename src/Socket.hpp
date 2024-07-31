#ifndef Socket_hpp
#define Socket_hpp

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

// socketFunction, create socket itself
// bind it to the network
// listen
// the type for web server is SOCK_STREAM
namespace GS {
class Socket {
    private:
        struct sockaddr_in  adress;
        int sock;
        int connection;
    public:
        //constructor
        Socket(int domain, int serviceType, int protocol, int port
            u_long ip);
        //function to connect to network. virtual because different types of sockets need to do something different here
        virtual int connect_to_network(int sock, struct sockaddr_in adress) = 0;
        //test connections and sockets
        void    test_connection(int test);
        int     getSock();
        struct  sockaddr_in get_address();
        int     get_connection();
        void    set_connection(int con);
        
};
}
#endif