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

std::string getMatchingUrl(const std::map<std::string, Location> &locations, const std::string &uri)
{
    std::string match = "";
    std::string currLoc;
    std::map<std::string, Location> locs = locations;
    std::map<std::string, Location>::const_iterator   itLoc = locs.begin();
    std::map<std::string, Location>::const_iterator   iteLoc = locs.end();

    for (; itLoc != iteLoc; itLoc++)
    {      
        currLoc = itLoc->second.url;
        if (uri.find(currLoc) == 0 && currLoc.size() > match.size())
            match = currLoc;
    }
    return (match);
}

Location    getLocation(Request &request, const std::map<std::string, Location> &locations)
{
    std::string matchingUrl = getMatchingUrl(locations, request.getUri());
    Location    loc;

    std::map<std::string, Location>::const_iterator   it = locations.begin();
    std::map<std::string, Location>::const_iterator   ite = locations.end();
    for (; it != ite; ++it)
    {
        if (matchingUrl == it->second.url)
            loc = it->second;
    }
    return (loc);
}

std::string	getWorkingDir()
{
	char	temp[4096];
	return (getcwd(temp, sizeof(temp)) ? std::string(temp) : std::string(""));
}

std::string customTrailCheck(const std::string  &str, char c)
{
    size_t  trail = str.find_last_of(c);

    if (trail != std::string::npos)
        return (str.substr(0, trail));
    return (str);
}