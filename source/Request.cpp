
#include "../include/Request.hpp"

//this shows the request line parsed.
Request::Request(const std::string& fullRequest, int fd, const std::vector<ServerConfig> &servers)
{
	//std::cout << "\n======================================\nFULL REQUEST\n" << fullRequest << "======================================\n";
	_fullRequest = fullRequest;
	std::stringstream	ss(fullRequest);
	ss >> _method >> _uri >> _protocol;
    //std::cout << "Request constructed.\nMethod: " << _method << ", URI: " << _uri << ", Protocol: " << _protocol << "\n\n";
	setConfig(servers);
	_location = getLocation(*this, _config.locations);
	setPath();
	size_t bodyStart = fullRequest.find("\r\n\r\n");
	if (bodyStart != std::string::npos /*&& _method != "DELETE"*/)
	{
		_body = fullRequest.substr(bodyStart + 4);
		std::cout << "Body: " << _body << std::hex << std::endl;
	}
	else
	{
		std::cout << "No body found in request.\n";
	}
	_fd = fd;
	if (DEBUG)
	{
		std::cout	<< "\n==========\nServerConfig:\n"
					<< "\nserver_name : " << _config.server_name << '\n';
        std::cout << "  Error Pages:\n";
        for (std::map<int, std::string>::iterator it = _config.error_page.begin(); it != _config.error_page.end(); ++it)
            std::cout << "    " << it->first << " " << it->second << '\n';
		std::cout	<< "\nclient_max_body_size : " << _config.client_max_body_size << '\n'
					<< "ports : ";
		for (size_t i = 0; i < _config.port.size(); i++)
			std::cout << std::dec << _config.port[i] << ' ';
		std::cout << '\n';
		std::cout << "locations:\n";
		for (std::map<std::string, Location>::iterator it = _config.locations.begin(); it != _config.locations.end(); ++it)
		{
			std::cout << "location : " << it->first
					  << "\nroot  : " << it->second.root
					  << "\nindex : " << it->second.index
					  << "\nupload_dir : " << it->second.upload_dir // upload_dir doesn't have anything even when it should it seems
					  << "\nmethods : "
					  << "\ncgi_path : ";
			for (std::vector<std::string>::iterator itCGIP = it->second.cgi_path.begin(); itCGIP != it->second.cgi_path.end(); ++itCGIP)
				std::cout << *itCGIP << ' ';
			std::cout << "\ncgi_extension : ";
			for (std::vector<std::string>::iterator itCGIE = it->second.cgi_extension.begin(); itCGIE != it->second.cgi_extension.end(); ++itCGIE)
				std::cout << *itCGIE << ' ';
			std::cout << '\n';
			for (std::vector<std::string>::iterator itM = it->second.methods.begin(); itM != it->second.methods.end(); ++itM)
				std::cout << *itM << ' ';
			std::cout << '\n';
		}
	}
  	handleRequest();
}


Request::~Request() {}


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
	else {
		size_t headerEnd = response.find("\r\n\r\n");
		if (headerEnd != std::string::npos)
			std::cout << response.substr(0, headerEnd + 4) << "\n[Body omitted]\n";
	}
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
	return (uri.find("/cgi-bin") == 0);
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
	//_response.clear();
	//_header.clear();
	//_content.clear();
	struct stat	pathInfo;

	if (stat(_path.c_str(), &pathInfo) != 0)
	{
		std::cout << "dbg: " << _path << " does not exist\n";
		setErrorResponse(404);
		sendFullResponse(_fd, _response);
		return ;
	}
	if (S_ISDIR(pathInfo.st_mode) && _location.autoindex)
	{
		if (_location.autoindex)
		{
			std::cout << "dbg: autoindex\n";
			return ; 
		}
		else
		{
			std::cout << "is a directory, no permission?\n";
			return ;
		}
	}
	if (isCGIRequest(_uri))
	{
		std::cout << "dbg: CGI\n"; 
		cgiRequest();
	}
	else if (_method == "GET")
	{
		std::cout << "dbg: GET\n"; 
		getRequest();
	}	
	else if (_method == "POST") 
	{
		std::cout << "dbg: POST\n"; 
		postRequest();
	}	
	else if (_method == "DELETE")
	{
		std::cout << "dbg: DELETE\n"; 
		deleteRequest();
	}
	else
		std::cout << "dbg: dir out of bounds?\n" << _fullRequest <<"\n"; 

}

void	Request::postRequest() {
	//second parameter is because if it exists you should be appending
	_path = _location.root + _location.upload_dir + "/upload.txt";
	std::ifstream check(_path.c_str());
	bool fileExists = check.is_open();
	check.close();
	std::ofstream file(_path.c_str(), std::ios::app);
	if (!file.is_open())
	{
		Log::error("Failed to open file for writing.");
		setErrorResponse(500);
	}
	else {
		file << _body << '\n';
		file.close();
		std::cout << "File written successfully.\n";
		_contentLength = _body.length();
		setContentType();
		if (fileExists) {
			setHeader(200); }
		else {
			setHeader(201); }
		sendFullResponse(_fd, _header);
		std::cout << "Response sent successfully.\n";
	}
}

void 	Request::deleteRequest() {
	std::ifstream check(_path.c_str());
	bool fileExists = check.is_open();
	check.close();
	if (!fileExists) {
		Log::error("Failed to find file for deletion.");
		setErrorResponse(404);
		return;
	}
	std::ofstream file(_path.c_str());
	if (!file.is_open()) {
		Log::error("Failed to open file for deletion.");
		setErrorResponse(500);
		return;
	} else {
		_contentLength = 0;
		file.close();
		if (std::remove(_path.c_str())) {
			Log::error("Failed to delete file.");
			//setErrorResponse(204); --do the 204 No content one.
			return;
		}
		setHeader(200);
		sendFullResponse(_fd, _header);
		std::cout << "Response sent successfully" << std::endl;
	}
	//check if the path exists 
	//check if you can delete it
}

void	Request::cgiRequest()
{
	CGIRequest	cgiRequest(*this, _fullRequest, _fd);

	cgiRequest.handleRequest();
}

void	Request::getRequest()
{
	std::ifstream	file(_path.c_str());
	std::ostringstream	body;
	
	if (!file)
	{
		std::cout << "HERE\n";
		setErrorResponse(404);
	}
	else
	{
		setContentType();
		//if (_contentType.empty())
		//{
		//	setErrorResponse();
		//}
		body << file.rdbuf();
		_content = body.str();
		_contentLength = _content.length();
		setHeader(200);
		_response += _header + _content;
	}
	sendFullResponse(_fd, _response);
}


void	Request::setPath()
{
	if (_uri == _location.url || _uri == _location.url + "/")
		_path = _location.root + "/" + _location.index;
	else
		_path = _location.root + _uri;
}

void	Request::setContentType()
{
	if (endsWith(_path, ".html"))
		_contentType = "text/html";
	else if (endsWith(_path, ".txt"))
		_contentType = "text/plain";
	else if (endsWith(_path, ".css"))
		_contentType = "text/css";
	else if (endsWith(_path, ".js"))
		_contentType = "application/javascript";
	else if (endsWith(_path, ".png"))
		_contentType = "image/png";
	else if (endsWith(_path, ".jpg") || endsWith(_path, ".jpeg"))
		_contentType = "image/jpeg";
    else
	{
		_contentType = "";
        Log::error("Unknown type: " + _path + '\n'); // added to see what the tester might send through here
	}
}


void	Request::setErrorResponse(int error)
{
		std::string	path = _location.root + _config.error_page[error];// path to desired error in the future
		std::string	errorStr;
		std::ostringstream	body;
		std::ifstream	errorFile(path.c_str());

		switch (error)
		{
		case 400:
			errorStr = "Bad Request";
			break;
		case 401:
			errorStr = "Unauthorized";
			break;
		case 403:
			errorStr = "Forbidden";
			break;
		case 404:
			errorStr = "Not Found";
			break;
		case 409:
			errorStr = "Conflict";
			break;
		case 504:
			errorStr = "Gateway Timeout";
			break;
		default:
			errorStr = "Internal Server Error";
			error = 500;
			break;
		}

		if (errorFile)
		{
			body << errorFile.rdbuf();
			_content = body.str();
			_header += "HTTP/1.1 " + toString(error) + " " + errorStr + "\r\n";
        	_header += "Content-Type: text/html\r\n";
        	_header += "Content-Length: " + toString(_content.length()) + "\r\n";
			_header += "Connection: close\r\n\r\n";
			_response += _header + _content;
		}
		else
		{
			std::cerr << path << " could not found or could not open.\n";
			_response += "HTTP/1.1 500 Internal Server Error\r\n";
            _response += "Connection: close\r\n\r\n";
		}
}


void	Request::setHeader(int code)
{
	if (code == 200) {
		_header += "HTTP/1.1 200 OK\r\n";
	}
	else if (code == 201) {
		_header += "HTTP/1.1 201 Created\r\n";
	}
	_header += "Content-Type: " + _contentType + "\r\n";
	_header += "Content-Length: " + toString(_contentLength) + "\r\n";
	_header += "Connection: close\r\n\r\n";
}

void	Request::setConfig(const std::vector<ServerConfig> &servers)
{
	std::vector<ServerConfig>	svConf = servers;
	size_t	hostPos = _fullRequest.find("Host:");
	std::string	hostStr, host;
	
	if (hostPos != std::string::npos)
	{
		size_t lineEnd = _fullRequest.find("\r\n", hostPos);
		hostStr = _fullRequest.substr(hostPos, lineEnd - hostPos);
		host = hostStr.substr(hostStr.size() - 4, hostStr.size());
	}
	
	for (size_t i = 0; i < svConf.size(); i++)
	{
		std::vector<int>	currPorts = svConf[i].port;
		for (size_t j = 0; j < currPorts.size(); j++)
		{
			if (toString(currPorts[j]) == host)
			{
				_config = svConf[i];
			}
		}
	}
}

const std::string	&Request::getMethod() const { return(_method); }

const std::string	&Request::getUri() const { return(_uri); }

const std::string	&Request::getProtocol() const { return(_protocol); }

const std::string	&Request::getHeader() const { return(_header); }

const std::string	&Request::getBody() const { return(_body); }

ServerConfig	Request::getConfig() const { return(_config); }