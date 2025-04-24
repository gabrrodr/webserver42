
#include "../include/Request.hpp"

Request::Request(char *buffer, int fd)
{
	std::stringstream	ss(buffer);

	ss >> _method >> _uri >> _protocol;
	_fd = fd;
	std::cout << "buffer: " << buffer << '\n';
    std::cout << "Request constructed.\nMethod: " << _method << ", URI: " << _uri << ", Protocol: " << _protocol << "\n\n";

	handleRequest();
}

Request::~Request() {}

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

// utils
static bool	endsWith(std::string path, std::string find)
{
	if (path.size() < find.size())
		return (false);
	return(path.compare(path.size() - find.size(), find.size(), find) == 0);
}

void	Request::handleRequest()
{
	if (_method == "GET")
		getRequest();
}

void	Request::getRequest()
{
	std::string	path;

   	path = "./www" + _uri;
	if (_uri == "/")
		path = "./www/index.html";
	
	std::ifstream	file(path.c_str());
	std::ostringstream	body;
	if (!file)
		setErrorResponse(404);
	else
	{
		body << file.rdbuf();
		_content = body.str();
		setContentType(path);
		setHeader();
		_response += _header + _content;
	}
	sendFullResponse(_fd, _response);
}

void	Request::setContentType(std::string &path)
{
	if (endsWith(path, ".html"))
		_contentType = "text/html";
	else if (endsWith(path, ".css"))
		_contentType = "text/css";
	else if (endsWith(path, ".js"))
		_contentType = "application/javascript";
	else if (endsWith(path, ".png"))
		_contentType = "image/png";
	else if (endsWith(path, ".jpg") || endsWith(path, ".jpeg"))
		_contentType = "image/jpeg";
    else
        Log::error("Unknown type: " + path + '\n'); // added to see what the tester might send through here
}


void	Request::setErrorResponse(int error)
{
	if (error == 404)
	{
		// this should be our config error_file however this function isn't a ServerConfig member function
		// so i'll see that later
		std::string	path = "./www/404.html";// path to desired error in the future
		std::ostringstream	body;
		std::ifstream	errorFile(path.c_str());
		if (errorFile)
		{
			body << errorFile.rdbuf();
			_content = body.str();
			_header += "HTTP/1.1 404 Not Found\r\n";
        	_header += "Content-Type: text/html\r\n";
        	_header += "Content-Length: " + toString(_content.length()) + "\r\n";
			_header += "Connection: close\r\n\r\n";
			_response += _header + _content;
		}
		else
		{
			std::cerr << "404.html not found or could not be opened.\n";
			_response += "HTTP/1.1 500 Internal Server Error\r\n";
            _response += "Connection: close\r\n\r\n";
		}
	}
}


void	Request::setHeader()
{
	_header += "HTTP/1.1 200 OK\r\n";
	_header += "Content-Type: " + _contentType + "\r\n";
	_header += "Content-Length: " + toString(_content.length()) + "\r\n";
	_header += "Connection: close\r\n\r\n";
}
