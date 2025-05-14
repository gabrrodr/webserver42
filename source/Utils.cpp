#include "../include/Utils.hpp"

bool isListeningSocket(int fd, const std::vector<ListeningSocket*>& sockets) {
    for (size_t i = 0; i < sockets.size(); ++i) {
        if (sockets[i]->get_sock() == fd) {
            return true;
        }
    }
    return false;
}

std::string getExtension(const std::string &uri) {
    size_t queryPos = uri.find("?");
    std::string extension = (queryPos != std::string::npos) ? uri.substr(0, queryPos) : uri;
    size_t dotPos = extension.find_first_of(".");

    if (dotPos == std::string::npos)
        return ("");
    if (dotPos == extension.size() - 1)
        return ("");
    extension = extension.substr(dotPos);
    extension = extension.substr(0, extension.find_first_of("/#"));
    return (extension);
}

std::string getMatchingUrl(const std::map<std::string, Location> &locations, const std::string &uri, const std::string &method)
{
    std::string match = "";
    std::string currMethod;
    std::string currLoc;
    std::map<std::string, Location>::const_iterator   itLoc = locations.begin();
    std::map<std::string, Location>::const_iterator   iteLoc = locations.end();

    for (; itLoc != iteLoc; ++itLoc)
    {
        std::vector<std::string>::const_iterator  itM = itLoc->second.methods.begin();
        std::vector<std::string>::const_iterator  iteM = itLoc->second.methods.end();
        currLoc = itLoc->second.url;
        for (; itM != iteM; ++itM)
        {
            currMethod = *itM;
            if (currMethod.find(method) == 0)
                break;
        }
        if (itM == itLoc->second.methods.end())
            continue;
        if (uri.find(currLoc) == 0 && currLoc.size() > match.size())
            match = currLoc;
    }
    return (match);
}

Location    getLocation(const Request &request, const std::map<std::string, Location> &locations)
{
    std::string uri = request.getUri();
    std::string matchingUrl = getMatchingUrl(locations, request.getUri(), request.getMethod());
    Location    loc;

    std::map<std::string, Location>::const_iterator   it = locations.begin();
    std::map<std::string, Location>::const_iterator   ite = locations.end();
    for (; it != ite; ++it)
    {
        if (matchingUrl == it->second.url)
            return (it->second);
    }
    return (loc);
}