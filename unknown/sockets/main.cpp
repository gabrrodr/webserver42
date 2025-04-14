#include "ListeningSocket.hpp"
#include "SimpleSocket.hpp"
#include "ConnectionSocket.hpp"
#include "BindingSocket.hpp"

int main(void) {
try {
std::cout << "Starting..." << std::endl;
//std::cout << "Socket creation..." << std::endl;
//class GS::Socket s = GS::Socket(AF_INET, SOCK_STREAM, 0, 80, INADDR_ANY);
std::cout << "Binding Socket..." << std::endl;
BindingSocket   bs(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY);
std::cout << "Listening Socket..." << std::endl;
ListeningSocket ls(AF_INET, SOCK_STREAM, 0, 8081, INADDR_ANY, 10);
std::cout << "Success!!!!!!!" << std::endl;
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
}
return (0);
}
