
#include "Server.hpp"

Server::Server(int port): _port(port), _listenFd(-1) {}

Server::~Server()
{
	if (_listenFd != -1)
		close(_listenFd);
}

bool	Server::setNonBlocking(int fd)
{
	int	flags = fcntl(fd, F_GETFL, 0);// get the file access mode and the file status flags
	if (flags < 0)
		return false;
	return (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0);// sets the file status flags
}

bool	Server::start()
{
	if (!setupSocket())
	{
		std::cerr << "Failed to setup socket.\n";
		return (false);
	}
	std::cout << "\nListening on port: " << _port << '\n';
	eventLoop();
	return (true);
}

bool	Server::setupSocket()
{
	_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	std::cout << "\nsocket listening on fd: " << _listenFd << '\n';
	if (_listenFd < 0)
	{
		std::cerr << "Socket error\n";
		return (false);
	}

	int opt = 1;
	setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	std::cout << "\nopt: " << opt << '\n';

	sockaddr_in	addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);
	std::cout	<< "\naddr before bind: \n"
								<< "add.sin_family = " << AF_INET << '\n'
								<< "add.sin_addr.s_addr = " << INADDR_ANY << '\n'
								<< "addr.sin_port = " << htons(_port) << '\n';

	if (bind(_listenFd, (struct sockaddr*)&addr, sizeof(addr)) < 0)// assigns the specified address to the specified socket
	{
		std::cerr << "Binding error.\n";
		return (false);
	}
	std::cout	<< "\naddr after bind: \n"
								<< "add.sin_family = " << AF_INET << '\n'
								<< "add.sin_addr.s_addr = " << INADDR_ANY << '\n'
								<< "addr.sin_port = " << htons(_port) << '\n';

	if (listen(_listenFd, 100) < 0)// marks the specified socket as a passive socket aka a socket that will be used to receive requests
	{
		std::cerr << "Listening error.\n";
		return (false);
	}

	if (!setNonBlocking(_listenFd))
	{
		std::cerr << "Set non blocking error;\n";
		return (false);
	}

	struct pollfd	pfd;
	pfd.fd = _listenFd;
	pfd.events = POLLIN;
	_pollFds.push_back(pfd);

	return (true);
}

void	Server::eventLoop()
{
	while (true)
	{
		int	ret = poll(&_pollFds[0], _pollFds.size(), -1);
		if (ret < 0)
		{
			std::cerr << "Poll error.\n";
		}

		for (size_t i = 0; i < _pollFds.size(); i++)
		{
			int	fd = _pollFds[i].fd;
			short	revents = _pollFds[i].revents;

			if (revents == 0)
				continue;
			if (fd == _listenFd && (revents & POLLIN))
				handleNewConnection();
			else
			{
				if (revents & POLLIN)
					handleClientRead(fd);
				if (revents & POLLOUT)
					handleClientWrite(fd);
			}
		}
	}
}

void	Server::handleNewConnection()
{
	while (true)
	{
		int	clientFd = accept(_listenFd, NULL, NULL);
		if (clientFd < 0)
		{
			if (errno != EWOULDBLOCK && errno != EAGAIN)
				std::cerr << "Accept error.\n";
			break;
		}
		if (!setNonBlocking(clientFd))
		{
			close(clientFd);
			continue;
		}
		struct pollfd	pfd;
		pfd.fd = clientFd;
		pfd.events = POLLIN | POLLOUT;
		_pollFds.push_back(pfd);
		_clients[clientFd] = Client();
	}
}

void	Server::handleClientRead(int fd)
{
	char	buffer[4096];
	ssize_t	bytes = recv(fd, buffer, sizeof(buffer), 0);// recieve message from socket

	if (bytes <= 0)
	{
		closeClient(fd);
		return;
	}

	Client &client = _clients[fd];
	client.readBuffer.append(buffer, bytes);

	if (client.readBuffer.find("\r\n\r\n") != std::string::npos)
	{
		client.requestComplete = true;
		client.writeBuffer = buildHttpResponse(client.readBuffer);
	}

}

void	Server::handleClientWrite(int fd)
{
	Client	&client = _clients[fd];
	if (!client.requestComplete || client.writeBuffer.empty())
		return;

	ssize_t	sent = send(fd, client.writeBuffer.c_str(), client.writeBuffer.size(), 0);
	if (sent <= 0)
	{
		closeClient(fd);
		return;
	}

	client.writeBuffer.erase(0, sent);
	if (client.writeBuffer.empty())
		closeClient(fd);
}

void	Server::closeClient(int fd)
{
	close(fd);
	_clients.erase(fd);
	for (size_t i = 0; i < _pollFds.size(); i++)
	{
		if (_pollFds[i].fd == fd)
		{
			_pollFds.erase(_pollFds.begin() + i);
			break;
		}
	}
}

static bool	endsWith(const std::string &str, const std::string &suffix)
{
	if (str.size() < suffix.size())
		return (false);
	return (str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0);
}	

std::string	Server::buildHttpResponse(const std::string &request)
{
	std::istringstream	ss(request);
	std::string	method, uri, version;

	ss >> method >> uri >> version;

	std::string	path = "./www" + uri;
	if (uri == "/")
		path = "./www/index.html";

	std::ifstream	file(path.c_str());
	std::ostringstream	response;

	if (!file)
	{
		response << "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found\n";
	}
	else
	{
		std::ostringstream	body;
		body << file.rdbuf();
		std::string content = body.str();

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

		response << "HTTP/1.1 200 OK\r\n";
		response << "Content-Type: " << contentType << "\r\n";
		response << "Content-Length: " << content.size() << "\r\n\r\n";
		response << content;		
	}
	std::cout << response.str() << '\n';
	return (response.str());
}