
#include "../include/Request.hpp"
#include "../include/Config.hpp"

//this shows the request line parsed.
Request::Request(const std::string& fullRequest, int fd)
{
	std::stringstream	ss(fullRequest);
	ss >> _method >> _uri >> _protocol;
    std::cout << "Request constructed.\nMethod: " << _method << ", URI: " << _uri << ", Protocol: " << _protocol << "\n\n";
	parseHeaders(ss);
	size_t bodyStart = fullRequest.find("\r\n\r\n");
	if (bodyStart != std::string::npos)
	{
		_body = fullRequest.substr(bodyStart + 4);
		std::cout << "Body: " << _body << '\n';
	}
	else
	{
		std::cerr << "No body found in request.\n";
	}
	_fd = fd;
	//std::cout << "buffer: " << buffer << '\n';
  handleRequest();
	setPath();
	
}

//trim content length so i only save the number
int	trimContentLength(std::string &str)
{
	size_t pos = str.find(":");
	if (pos != std::string::npos)
	{
		str.erase(0, pos + 1);
	}
	ConfigParser::trim(str);
	return (atoi(str.c_str()));
}

//trim content-type so i only save the type

std::string	trimContentType(std::string &str)
{
	size_t pos = str.find(":");
	if (pos != std::string::npos)
	{
		str.erase(0, pos + 1);
	}
	ConfigParser::trim(str);
	return (str);
}

//headers- content length and content type
void	Request::parseHeaders(std::stringstream &ss) {
	std::string line;

	while (std::getline(ss, line) && line != "\r")
	{
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.length() - 1);

		if (line.find("Content-Length:") == 0)
		{
			std::string contentLength = line.substr(15);
			_contentLength = trimContentLength(contentLength);
			std::cout << "Content-Length: " << _contentLength << '\n';
		}
		else if (line.find("Content-Type:") == 0)
		{
			std::string contentType = line.substr(13);
			_contentType = trimContentType(contentType);
			std::cout << "Content-Type: " << _contentType << '\n';
		}
		else
			std::cout << "Header: " << line << '\n';
	}
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

static bool	isCGIRequest(std::string &uri)
{
	return (endsWith(uri, ".cgi") || endsWith(uri, "/cgi-bin/"));
}

//	part of the string that comes after the ?
//	ex: GET /script.cgi?name=Gabi&age=25 HTTP/1.1
//	
std::string	Request::getQueryString(const std::string &uri)
{
	std::string	queryString;
	size_t	pos = uri.find('?');
	if (pos != std::string::npos)
	{
		queryString = uri.substr(pos + 1);
		_uri = uri.substr(0, pos);
	}
	return (queryString);
}

void	Request::handleRequest()
{
	if (isCGIRequest(_uri))
		cgiRequest();
	else if (_method == "GET")
		getRequest();
	else if (_method == "POST") 
		postRequest();
}

void	Request::postRequest() {
	std::string path = "./upload" + _uri;
	if (_uri == "/")
		path = "./upload/data.txt";
	//second parameter is because if it exists you should be appending
	std::ofstream file(path.c_str(), std::ios::app);
	if (!file.is_open())
	{
		Log::error("Failed to open file for writing.");
		setErrorResponse(500);
	}
	else {
		file << _body;
		file.close();
		std::cout << "File written successfully.\n";
		_header += "HTTP/1.1 200 OK\r\n";
		_header += "Content-Type: text/plain\r\n";
		_header += "Content-Length: 0\r\n";
		_header += "Connection: close\r\n\r\n";
		sendFullResponse(_fd, _header);
		std::cout << "Response sent successfully.\n";
	}

	
}

void	Request::cgiRequest()
{
	std::string	queryString = getQueryString(_uri);
	setContentType();

	char	*env[] = {
		const_cast<char *>(("REQUEST_METHOD=" + _method).c_str()),
		const_cast<char *>(("QUERY_STRING=" + getQueryString(_uri)).c_str()),
		const_cast<char *>(("CONTENT_TYPE=" + _contentType).c_str()),
		const_cast<char *>(("CONTENT_LENGTH=" + toString(_content.length())).c_str()),
		NULL
	};
	
	int	pipefd[2];
	if (pipe(pipefd) == -1)
	{
		Log::error("Failed to create pipe for CGI.");
		setErrorResponse(500);
		return ;
	}

	pid_t	pid = fork();
	if (pid == 0)// child process aka CGI script
	{
		close(pipefd[0]); // close read end
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);

		char	*args[] = {
			const_cast<char *>(_path.c_str()),
			NULL
		};
		execve(_path.c_str(), args, env);
		exit(1);
	}
	else if (pid > 0)// parent process
	{
		close(pipefd[1]);// close write end
		char	buffer[1024];
		std::ostringstream	responseStream;

		ssize_t	bytesRead;
		while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
		{
			responseStream.write(buffer, bytesRead);
		}
		close(pipefd[0]);// close read end
		_response = responseStream.str();
	}
	else
	{
		Log::error("Failed to fork CGI.");
		setErrorResponse(500);
	}
	sendFullResponse(_fd, _response);
	// init the requiered CGI ENV variables based on the HTTP request
	//
	// execute the script with fork()
	//
	// redirect the stdin and stdout to handle input and output between server
	// and the script
	//
	// read the script's output and send it as the HTTP response to the client
}

void	Request::getRequest()
{
	std::ifstream	file(_path.c_str());
	std::ostringstream	body;
	
	if (!file)
		setErrorResponse(404);
	else
	{
		body << file.rdbuf();
		_content = body.str();
		setContentType();
		setHeader();
		_response += _header + _content;
	}
	sendFullResponse(_fd, _response);
}


void	Request::setPath()
{
	_path = "./www" + _uri;
	if (_uri == "/")
		_path = "./www/index.html";
}

void	Request::setContentType()
{
	if (endsWith(_path, ".html"))
		_contentType = "text/html";
	else if (endsWith(_path, ".css"))
		_contentType = "text/css";
	else if (endsWith(_path, ".js"))
		_contentType = "application/javascript";
	else if (endsWith(_path, ".png"))
		_contentType = "image/png";
	else if (endsWith(_path, ".jpg") || endsWith(_path, ".jpeg"))
		_contentType = "image/jpeg";
    else
        Log::error("Unknown type: " + _path + '\n'); // added to see what the tester might send through here
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
	if (error == 500)
	{
		_response += "HTTP/1.1 500 Internal Server Error\r\n";
        _response += "Connection: close\r\n\r\n";
	}
}


void	Request::setHeader()
{
	_header += "HTTP/1.1 200 OK\r\n";
	_header += "Content-Type: " + _contentType + "\r\n";
	_header += "Content-Length: " + toString(_content.length()) + "\r\n";
	_header += "Connection: close\r\n\r\n";
}
