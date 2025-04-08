#include "ListeningSocket.hpp"

ListeningSocket::ListeningSocket(int domain, int serviceType, int protocol, int port, u_long ip, int backlog) : BindingSocket(domain, serviceType, protocol, port, ip), backlog(backlog)
{
    start_listening();
    test_connection(get_connection());
}

void    ListeningSocket::start_listening() {
    listening = listen(get_sock(), backlog);
    test_connection(listening);
}