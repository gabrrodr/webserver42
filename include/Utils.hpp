
#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include "MainHeader.hpp"
#include "Request.hpp"

class Request;

bool isListeningSocket(int fd, const std::vector<ListeningSocket*>& sockets);
std::string getExtension(const std::string &uri);
Location    getLocation(const Request &request, const std::map<std::string, Location> &locations);

template <typename T>
std::string toString(T value) {
    std::stringstream stream;
    stream << value;
    return (stream.str());
}

#endif