#include "Socket.hpp"

GS::Socket::Socket(int domain, int serviceType, int protocol, int port, u_long ip) {
    //define address struct
    address.sin_family = domain;
    address.sin_port = htons(port);
    address.sin_address.s_addr = htonl(ip);
    //establish socket
    this->sock = socket(domain, serviceType, protocol);
    test_connection(sock);
    //bind function called on the server side code. 
    //connect in client side
    //establish connection
    //this->connection = connect_to_network(sock, address);
    //test_connection(connection);
}

//virtual function
//checks if the socket or the connection has been properly established
void    GS::Socket::test_connection(int test) {
    if (test < 0) {
        perror("Failed to connect");
        exit(EXIT_FAILURE);
    }
}

//getter functions
int GS::Socket::get_sock() {
    return this->sock;
}

struct  sockaddr_in GS::Socket::get_address() {
    return this->address;
}

int GS::Socket::get_connection() {
    return this->connection;
}

//setter functions

void    GS::Socket::set_connection(int con) {
    this->connection = con;
}