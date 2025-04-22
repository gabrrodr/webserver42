#include "../include/Config.hpp"
#include <string>
#include "Utils.cpp"


ConfigParser::ConfigParser(const std::string& filename) : filename_(filename) {}

std::vector<ServerConfig>   ConfigParser::parse() {
    std::ifstream   file(filename_.c_str());
    if (!file.is_open()) {
        Log::error("parser error: Could not open file " + filename_);
        exit(1);
    }
    std::vector<ServerConfig>   servers;
    std::string line;
    while (std::getline(file, line)) {
        trim(line);        
        if (line.empty() || line[0] == '#')
            continue;
        if (line == "server {") {
            servers.push_back(parseServer(file, line));
        }
        if (line == "}") {
            break;
        }
        else
            throw std::runtime_error("Unexpected line in configuration (parse): " + line);
    }
    return servers;
}

void    ConfigParser::trim(std::string& str) {
    size_t  start = str.find_first_not_of(" \t");
    size_t  end = str.find_last_not_of(" \t");
    if (start == std::string::npos || end == std::string::npos) {
            str = "";
        } else {
            str = str.substr(start, end - start + 1);
        }
}

std::vector<std::string>    ConfigParser::split(const std::string& str, char delimiter) {  
    std::vector<std::string>    tokens;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        tokens.push_back(item);
    }
    return tokens;
}

size_t  ConfigParser::parseSize(const std::string& str) {
    size_t size = std::atoi(str.c_str());
    char lastChar = str[str.length() - 1];
        if (lastChar == 'M') {    
            size *= 1024 * 1024;
        } else if (lastChar == 'K') {
            size *= 1024;
        }
        return size;
}

void        ConfigParser::parseRoot(const std::string& line, Location& server) {
    std::istringstream   iss(line);
    std::string         directive;
    std::string         value;

    std::cout << "Parsing line: " << line << std::endl;

    iss >> directive >> value;
    std::cout << "Directive: " << directive << std::endl;
    std::cout << "Value: " << value << std::endl;

    if (directive == "root") {
        server.root = value;
    } else {
        throw std::runtime_error("Unknown directive: " + directive);
    }
}


bool    ConfigParser::isLocation(const std::string& line) {
    if (line.find("location") == 0 || line.find("root") == 0 || line.find("index") == 0 || line.find("methods") == 0 || line.find("upload_dir") == 0) {
        return true;
    }
    return false;
}


ServerConfig    ConfigParser::parseServer(std::ifstream& file, std::string& line) {
    ServerConfig    server;
    trim(line);
    if (line != "server {") {
        throw std::runtime_error("Unexpected end of file");
    }
    std::cout << "Parsing server..." << std::endl;
    while (std::getline(file, line)) {
            trim(line);
            if (line == "}")
                break;
            if (line.empty() || line[0] == '#')
                continue;
            if (line.find("listen ") == 0) {
                server.port = std::atoi(line.substr(7).c_str()); }
            else if (line.find("server_name ") == 0) {
                server.server_name = line.substr(12); }
            else if (line.find("error_page ") == 0) {
                server.error_page = line.substr(11); }
            else if (line.find("client_max_body_size ") == 0) {
                server.client_max_body_size = parseSize(line.substr(22)); }
            else if (line.find("location ") == 0) {
                std::string locationPath = line.substr(9);
                trim(locationPath);
                if (!locationPath.empty() && locationPath[locationPath.size() - 1] == '{') {
                    locationPath = locationPath.substr(0, locationPath.size() - 1);
                    trim(locationPath);
                }
                Location location = parseLocation(file);
                server.locations[locationPath] = location;
            } else {
                throw std::runtime_error("Unknown directive (parse server): " + line);
            }
        }
        std::cout << "Server parsed!" << std::endl;
        std::cout << "Port: " << server.port << std::endl;
        std::cout << "Server Name: " << server.server_name << std::endl;
        std::cout << "Error Page: " << server.error_page << std::endl;
        std::cout << "Max Body Size: " << server.client_max_body_size << std::endl;
        return server;
}

Location    ConfigParser::parseLocation(std::ifstream& file) {
    Location    location;
    std::string line;
    std::cout << "Parsing location..." << std::endl;
    while (std::getline(file, line)) {
        trim(line);
        if (line == "}")
            break;
        if (line.empty() || line[0] == '#')
            continue;
        else if (line.find("root ") == 0) {
            location.root = line.substr(5);
            //parseRoot(line, location);
            std::cout << "Root: " << location.root << std::endl;
        }
        else if (line.find("index ") == 0)
            location.index = line.substr(6);
        else if (line.find("methods ") == 0)
            location.methods = split(line.substr(8), ' ');
        else if (line.find("upload_dir ") == 0) {
            location.upload_dir = line.substr(11);
        } else if (!isLocation(line)) {
           break;
        }
    }
    std::cout << "Location parsed!" << std::endl;
    std::cout << "Index: " << location.index << std::endl;
    std::cout << "Upload Dir: " << location.upload_dir << std::endl;
    return location;
}

//set up listening sockets (server sockets)
//using poll() to handle multiple connections and monitor server and client sockets
//are new connections accepted and stored correctly?
//client sockets handled after connection (just accepted for now)
//memory leaks?
//can the loop handle multiple connections simultaneously?
//making sockets non-blocking using fcntl
//socket set up and connection handling in different functions!!
//keep a list of active sockets

/*
-this is doing nothing.divide handleIncomingConnections into two functions at least and use this

void    setupServerSockets(const std::vector<ServerConfig>& servers) {
    std::vector<ListeningSocket*>   serverSockets;
    for (size_t i = 0; i < servers.size(); ++i) {
        try {
            ListeningSocket* server = new ListeningSocket(AF_INET, SOCK_STREAM, 0, servers[i].port, INADDR_ANY, 10);
            serverSockets.push_back(server);

            std::cout << "Server listening on port " << servers[i].port << std::endl;
        } catch (std::exception& e) {
            Log::error("Failed to set up server on port (set up server socket): " + std::to_string(servers[i].port) + " " + e.what());
        }
    }
}*/

// utils
// checks path
// might change to another solution later since this one is kind of ugly looking
static bool	endsWith(std::string path, std::string find)
{
	if (path.size() < find.size())
		return (false);
	return(path.compare(path.size() - find.size(), find.size(), find) == 0);
}
// utils
// correctly transforms the Content Length to the expected format
template <typename T>
static std::string  toString(T value)
{
    std::stringstream   stream;
    stream << value;
    return (stream.str());
}

static void sendFullResponse(int fd, const std::string &response)
{
    size_t  totalSent = 0;
    size_t  responseLength = response.length();

    while (totalSent < responseLength)
    {
        ssize_t sent = send(fd, response.c_str() + totalSent, responseLength - totalSent, 0);
        if (sent < 0)
        {
            Log::error("Failed to send full response.");
            break;
        }
        totalSent += sent;
    }
    if (totalSent < responseLength)
        Log::error("Response not fully send. Sent: " + toString(totalSent) + " of " + toString(responseLength));
}

void    handleClientRequests(std::vector<pollfd>& poll_fds, size_t& i) {

    char buffer[4096];
    int fd = poll_fds[i].fd;
    int bytesReceived = recv(fd, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived <= 0) {
        std::cout << "Client disconnected or error on FD: " << fd << std::endl;
        close(fd);
        poll_fds.erase(poll_fds.begin() + i);
        --i;
        return;
    }

    buffer[bytesReceived] = '\0';
    std::string request(buffer);
    std::cout << "buffer:\n_" << buffer << "_\n";
    std::cout << "Received request:\n_" << request << "_\n";

    std::string	response;
    std::stringstream ss(buffer);
    std::string method, uri, version, path, content;
    std::string contentLength = ss.str();

    ss >> method >> uri >> version;
    std::cout << "Method: " << method << ", URI: " << uri << ", Version: " << version << "\n\n";
   	path = "./www" + uri;
	if (uri == "/")
		path = "./www/index.html";
    std::ifstream   file(path.c_str());
    std::ostringstream  body;
    if (!file)
	{
		// this should be our config error_file however this function isn't a ServerConfig member function
		// so i'll see that later
		std::cout << "no such file: " << path << '\n';
		path = "./www/404.html";
		std::ifstream errorFile(path.c_str());
		if (errorFile)
		{
        	body << errorFile.rdbuf();
        	content = body.str();
        	response += "HTTP/1.1 404 Not Found\r\n";
        	response += "Content-Type: text/html\r\n";
        	response += "Content-Length: ";
			response += toString(content.length());
            response += "\r\nConnection: close\r\n\r\n";
			response += "\r\n\r\n";
        	response += content;
		}
		else
		{
			std::cerr << "404.html not found or could not be opened.\n";
			response += "HTTP/1.1 500 Internal Server Error\r\n";
            response += "Connection: close\r\n\r\n";
		}
	}
    else
    {
        body << file.rdbuf();
        content = body.str();
        std::string contentType = "text/plain";
		if (endsWith(path, ".html"))
			contentType = "text/html";
		else if (endsWith(path, ".css"))
			contentType = "text/css";
		else if (endsWith(path, ".js"))
			contentType = "application/javascript";
		else if (endsWith(path, ".png"))
			contentType = "image/png";
		else if (endsWith(path, ".jpg") || endsWith(path, ".jpeg"))
			contentType = "image/jpeg";
        else
            Log::error("Unknown type: " + path + '\n'); // added to see what the tester might send through here
		response += "HTTP/1.1 200 OK\r\n";
		response += "Content-Type: ";
		response += contentType;
		response += "\r\nContent-Length: ";
		response += toString(content.length());
		response += "\r\nConnection: close\r\n\r\n";
		response += content;
    }
    sendFullResponse(fd, response);
    //send(fd, response.c_str(), response.length(), 0);// void sendFullResponse(int fd, const std::string& response)
    close(fd);
    poll_fds.erase(poll_fds.begin() + i);
    --i;
}

void    handleIncomingConnections(const std::vector<ServerConfig>& servers) {
    std::vector<ListeningSocket*>   sockets;
    std::vector<pollfd>             poll_fds;

    Log::error("\033[35mSETTIN UP SERVERS...\033[0m\n");

    for (size_t i = 0; i < servers.size(); ++i) {
        try {

            std::cout << "Setting up server on port " << servers[i].port << std::endl;
            ListeningSocket *server = new ListeningSocket(AF_INET, SOCK_STREAM, 0, servers[i].port, INADDR_ANY, 10);
            int sock_fd = server->get_sock();

            fcntl(sock_fd, F_SETFL, O_NONBLOCK);

            pollfd  pfd;
            pfd.fd = sock_fd;
            pfd.events = POLLIN;
            poll_fds.push_back(pfd);

            sockets.push_back(server);
            std::cout << "Server listening on port " << servers[i].port << std::endl;

            //vector to store client sockets but now using for now
            //std::vector<ConnectionSocket*> clientSockets;

            std::cout << "Entering main event loop..." << std::endl;

            while (true) {
                int activity = poll(&poll_fds[0], poll_fds.size(), -1);
                if (activity < 0) {
                    Log::error("Poll failed");
                    continue;
                }

                for (size_t i = 0; i < poll_fds.size(); ++i) {

                    if (poll_fds[i].revents & POLLIN) {

                        //check if the event is for the listening socket
                        if (isListeningSocket(poll_fds[i].fd, sockets)) {
                           
                            struct sockaddr_in  clientAddress;
                            socklen_t           clientAddressLength = sizeof(clientAddress);

                            std::cout << "Incoming connection detected on socket..." << poll_fds[i].fd << std::endl;
                            
                            int clientSocket = accept(poll_fds[i].fd, (struct sockaddr*)&clientAddress, &clientAddressLength);

                            if (clientSocket < 0) {
                                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                    //std::cout << "No incoming connections" << std::endl;
                                    continue;
                                }
                                Log::error("Failed to accept incoming connection ");
                                continue;
                            }
                        
                            std::cout << "New client connected! FD:" << clientSocket << std::endl;
                            
                            fcntl(clientSocket, F_SETFL, O_NONBLOCK);
                            pollfd  client_fd;
                            client_fd.fd = clientSocket;
                            client_fd.events = POLLIN;
                            poll_fds.push_back(client_fd);
                            
                        } else {

                            handleClientRequests(poll_fds, i);

                        }

                       /* 
                        ConnectionSocket* client = new ConnectionSocket(AF_INET, SOCK_STREAM, 0, 0, 0);
                        client->set_connection(clientSocket);
                        clientSockets.push_back(client);
                        */

                        //(sets to non-blocking mode
                        //add client socket to poll_fds so i can recv() from it

                        /*handleClientRequest(poll_fds[i].fd, sockets)*/
                    }
                }
            }

        } catch (std::exception& e) {
            Log::error("Failed to set up server on port : " + servers[i].port);
        }
    }

    for (size_t i = 0; i < sockets.size(); ++i) {
        delete sockets[i];
    }
}
