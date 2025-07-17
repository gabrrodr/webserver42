
#include "../include/Request.hpp"
#include <filesystem>

Request::Request(const std::string& fullRequest, int fd, const std::vector<ServerConfig> &servers)
{
	//std::cout << "\n======================================\nFULL REQUEST\n" << fullRequest << "======================================\n";
	_fullRequest = fullRequest;
	std::stringstream	ss(fullRequest);
	_fullRequest = fullRequest;
	ss >> _method >> _uri >> _protocol;
    //std::cout << "Request constructed.\nMethod: " << _method << ", URI: " << _uri << ", Protocol: " << _protocol << "\n\n";
	setConfig(servers);
	_location = getLocation(*this, _config.locations);
	setPath();
	cleanPath();
	size_t bodyStart = fullRequest.find("\r\n\r\n");
	if (bodyStart != std::string::npos /*&& _method != "DELETE"*/) {
		_body = fullRequest.substr(bodyStart + 4);
		//std::cout << "Body: " << _body << "\nbody size: " << _body.size() << /*std::hex <<*/std::endl;
		//std::cout << "fullRequest:\n" << fullRequest << '\n';
	} else {
		std::cout << "No body found in request.\n";
	}
	
	//std::cout << "BODY ON REQUEST = " << _body << "\nsize of: " << _body.size() << '\n';
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
					  << "\nupload_dir : " << it->second.upload_dir
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
	Log::message(RED+_config.server_name+" ["+_method+"] "+_uri+RESET);
  	handleRequest();
}


Request::~Request() {}


void Request::sendFullResponse(int fd, const std::string &response)
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
		{
			std::cout << response.substr(0, headerEnd + 4) << "\n[Body omitted]\n";
		}
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

bool	methodIsAllowed(std::string &method, std::vector<std::string> &vec)
{
	for (size_t i = 0; i < vec.size(); i++)
	{
		if (vec[i].find(method) == 0)
			return(true);
	}
	return (false);
}	

void	Request::handleRequest()
{
	//_response.clear();
	//_header.clear();
	//_content.clear();
	struct stat	pathInfo;

	std::string autoIndexBool = _location.autoindex == true ? "true" : "false";

	//std::cout << "Current Request:\n" << _method << " " << _uri << " " << _protocol << "\nPath: " << _path 
	//		  << "\nlocation: " << _location.url
	//		  << "\nautoindex: " << autoIndexBool
	//		  << "\n";
	if (!_location.redirect.second.empty())
	{
		setRedirectResponse();
		sendFullResponse(_fd, _response);
		return ;
	}
	if (!methodIsAllowed(_method, _location.methods))
	{
		Log::error("Method isn't allowed in this location.\n");
		setErrorResponse(405);
		return ;
	}
	if (stat(_path.c_str(), &pathInfo) != 0)
	{
		std::cout << "dbg: " << _path << " does not exist\n";
		setErrorResponse(404);
		sendFullResponse(_fd, _response);
		return ;
	}
	if (S_ISDIR(pathInfo.st_mode) && _location.autoindex)
		handleAutoIndexResponse();
	else if (isCGIRequest(_uri) && !S_ISDIR(pathInfo.st_mode))
	{
		std::cout << "dbg: CGI\n";
		std::cout << "dbg: CGI FULL REQUEST\n" << _fullRequest << std::endl;
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
		if (_uri == "/data")
			handleFileUpload();
		else
			postRequest();
	}	
	else if (_method == "DELETE")
	{
		std::cout << "dbg: DELETE\n"; 
		deleteRequest();
	}
}

void	Request::handleFileUpload() {
	size_t filenamePos = _body.find("filename=\"");
	if (filenamePos == std::string::npos) {
		Log::error("Filename not found in request body.");
		return;
	}
	//Getting the filename so we can store it with the correct name
	size_t filenameStart = filenamePos + 10;
	size_t filenameEnd = _body.find("\"", filenameStart);
	_filename = _body.substr(filenameStart, filenameEnd - filenameStart);	
	
	//now the content type
	size_t contentTypePos = _body.find("Content-Type: ");
	if (contentTypePos == std::string::npos) {
		Log::error("Content-Type not found in request body.");
		return;
	}
	size_t contentTypeStart = contentTypePos + 15;
	size_t contentTypeEnd = _body.find("\r\n", contentTypeStart);
	_contentType = _body.substr(contentTypeStart, contentTypeEnd - contentTypeStart);

	size_t contentStart = _body.find("\r\n\r\n", filenameEnd) + 4;
	size_t contentEnd = _body.find("\r\n");
	_content = _body.substr(contentStart, contentEnd - contentStart);
	//now we save the file
	if (mkdir("upload/data", 0777) == -1 && errno != EEXIST) {
		Log::error("Failed to create directory.");
		return;
	}
	std::ofstream file((_path + _filename).c_str(), std::ios::binary);
	if (!file) {
		Log::error("Failed to open file.");
		return;
	}
	file.write(_content.c_str(), _content.length());
	file.close();

	std::cout << "File uploaded successfully.\n";

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
// ToDo
// throw error when it is a directory and cant access?
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

void	Request::cleanPath()
{
	size_t	qMark = _path.find('?');

	if (qMark != std::string::npos)
	{
		_queryString = _path.substr(qMark + 1);
		_path = _path.substr(0, qMark);
	}
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
	if (endsWith(_path, ".html") || endsWith(_path, ".htm"))
		_contentType = "text/html";
	else if (endsWith(_path, ".txt"))
		_contentType = "text/plain";
	else if (endsWith(_path, ".css"))
		_contentType = "text/css";
	else if (endsWith(_path, ".js"))
		_contentType = "application/javascript";
	else if (endsWith(_path, ".json"))
		_contentType = "application/json";
	else if (endsWith(_path, ".gif"))
		_contentType = "image/gif";
	else if (endsWith(_path, ".png"))
		_contentType = "image/png";
	else if (endsWith(_path, ".jpg") || endsWith(_path, ".jpeg"))
		_contentType = "image/jpeg";
	else if (endsWith(_path, ".ico"))
	    _contentType= "image/x-icon";
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
		case 405:
			errorStr = "Method Not Allowed";
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
	_header += "\r\n";
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

void	Request::setRedirectResponse()
{
	_response = "HTTP/1.1 " + toString(_location.redirect.first) + " Redirect\r\n";
	_response += "Location: " + _location.redirect.second + "\r\n\r\n";
}

/************************************************************************/
/*                                                                      */
/*                               autoindex                              */
/*                                                                      */
/************************************************************************/

void	Request::handleAutoIndexResponse()
{
	std::string	pathTest = _location.root + _uri;
	DIR	*dir = opendir(pathTest.c_str()); // returns a pointer to the directory stream
	if (!dir)
	{
		Log::error("Failed to open dir: " + _path + " for autoindexing.\n");
		return;
	}
	std::stringstream	body;
	body << "<!DOCTYPE html><html><head><title>Home</title><style>";
	body << "body{font-family: 'Helvetica neue', Arial, sans-serif;background: linear-gradient(0deg, rgba(176, 72, 169, 1) 35%, rgba(71, 139, 222, 1) 100%);background-attachment: fixed;background-size: cover;color: #333;}";
	body << "#container{background-color: #fff;margin: 5% auto;min-height: 50%;width: 80%;padding: 2%;border-radius: 15px;box-shadow: 0 4px 10px rgba(0, 0, 0, 0.1);}";
	body << "#header{margin: 5% auto 0 auto;text-align: center;}#webserv-logo{max-width: 400px;}";
	body << "button{padding: 10px 20px;margin-top: 15px;font-size: 16px;background-color: rgba(71, 139, 222, 1);color: white;border: none;border-radius: 5px;cursor: pointer;transition: background-color 0.2s ease;}button:hover{background-color: rgba(176, 72, 169, 1);}";
	body <<	"</style></head><body><div id=\"header\"><a href=\"/\"><img id=\"webserv-logo\" src=\"/assets/img/webserv-logo.png\" alt=\"webserv-logo\"></a></div>";
	body <<	"<div id=\"container\"><ul>";
	
			
	struct dirent	*entry;
	while ((entry = readdir(dir)))// returns a pointer to a dirent struct representing the next directory stream
	{
		std::string	currName = entry->d_name;
		std::string	hrefLink = _uri + "/" + currName;
		if (currName == "." || currName == "..")
			continue;
		struct stat	currStat;
		if (stat(_path.c_str(), &currStat) == 0 && S_ISDIR(currStat.st_mode))
			currName = "/" + currName;
		body << "<li><a href=\""<< hrefLink << "\">" << currName << "</a></li>";
	}
	closedir(dir);
	
	body << "</ul></div></body></html>";

	setContentType();
	_content = body.str();
	_contentLength = _content.length();
	setHeader(200);
	_response = _header + _content;
	sendFullResponse(_fd, _response);
}

/************************************************************************/
/*                                                                      */
/*                               autoindex                              */
/*                                  end                                 */
/************************************************************************/

const std::string	&Request::getMethod() const { return(_method); }

const std::string	&Request::getUri() const { return(_uri); }

const std::string	&Request::getProtocol() const { return(_protocol); }

const std::string	&Request::getHeader() const { return(_header); }

const std::string	&Request::getBody() const { return(_body); }

ServerConfig	Request::getConfig() const { return(_config); }