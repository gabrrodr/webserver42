#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "MainHeader.hpp"
#include "Request.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <fcntl.h>

struct  Location {
    //can i initialize here, all empty = "", except methods
    std::string root;
    std::string index;
    std::string upload_dir;
    std::vector<std::string>    methods;
};

struct  ServerConfig {
    //initialize? 80; 1 << 20 (1MB); "localhost"; "error.html";
    std::vector<int> port;
    size_t  client_max_body_size;
    std::string server_name;
    std::string error_page;
    std::map<std::string, Location> locations;
};

class   ConfigParser {
    private:
        std::string filename_;
        ServerConfig    parseServer(std::ifstream& file, std::string& line);
        Location    parseLocation(std::ifstream& file);

    public:
        ConfigParser(const std::string& filename);
        std::vector<ServerConfig>   parse();

        bool    isLocation(const std::string& line);
        void    trim(std::string& str);
        std::vector<std::string>    split(const std::string& str, char delimiter);
        size_t  parseSize(const std::string& str);
        //maybe instead of server use location, cleaner
        void    parseRoot(const std::string& line, Location& server);
		std::vector<int>	parsePorts(const std::string	&line);
		bool	isPort(std::string &token);
};

//void  setupServerSockets(const std::vector<ServerConfig>& servers);
void    handleIncomingConnections(const std::vector<ServerConfig>& servers);

#endif