#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "MainHeader.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <fcntl.h>

struct  Location {
    std::string root;
    std::string index;
    std::string upload_dir;
    std::vector<std::string>    methods;
};

struct  ServerConfig {
    int port;
    size_t  client_max_body_size;
    std::string server_name;
    std::string error_page;
    std::map<std::string, Location> locations;
};

class   ConfigParser {
    private:
        std::string filename_;
        ServerConfig    parseServer(std::ifstream& filename, std::string& line);
        Location    parseLocation(std::ifstream& filename);

    public:
        bool    isLocation(const std::string& line);
        ConfigParser(const std::string& filename);
        std::vector<ServerConfig>   parse();
        void    trim(std::string& str);
        std::vector<std::string>    split(const std::string& str, char delimiter);
        size_t  parseSize(const std::string& str);
        void    parseRoot(const std::string& line, Location& server);
};

void    setupServerSockets(const std::vector<ServerConfig>& servers);
void    handleIncomingConnections(const std::vector<ServerConfig>& servers);

#endif