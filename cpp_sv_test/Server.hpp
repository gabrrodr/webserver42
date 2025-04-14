
#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <sys/socket.h> // socket()
#include <netinet/in.h> // sockaddr_in
#include <fcntl.h>
#include <poll.h>

struct	Client
{
	std::string readBuffer;
	std::string	writeBuffer;
	bool	requestComplete;
};

class Server
{
	public:
		Server(int port);
		~Server();

		bool	start();

	private:
		int	_port;
		int	_listenFd;
		std::vector<struct pollfd>	_pollFds;
		std::map<int, Client>	_clients;

		bool	setupSocket();
		bool	setNonBlocking(int fd);
		void	eventLoop();
		void	handleNewConnection();
		void	handleClientRead(int fd);
		void	handleClientWrite(int fd);
		void	closeClient(int fd);
		std::string	buildHttpResponse(const std::string &request);

};

#endif