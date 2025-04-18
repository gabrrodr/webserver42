#include "../include/MainHeader.hpp"

bool    isListeningSocket(int fd, const std::vector<ListeningSocket*>& sockets) {
    for (size_t i = 0; i < sockets.size(); ++i) {
        if (sockets[i]->get_sock() == fd) {
            return true;
        }
    }
    return false;
}