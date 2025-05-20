
#include "../include/Request.hpp"
#include "../include/Config.hpp"
#include <filesystem>

//this shows the request line parsed.
Request::Request(const std::string& fullRequest, int fd)
{
	std::stringstream	ss(fullRequest);
	_fullRequest = fullRequest;
	ss >> _method >> _uri >> _protocol;
    std::cout << "Request constructed.\nMethod: " << _method << ", URI: " << _uri << ", Protocol: " << _protocol << "\n\n";
	setPath();
	size_t bodyStart = fullRequest.find("\r\n\r\n");
	if (bodyStart != std::string::npos /*&& _method != "DELETE"*/) {
		_body = fullRequest.substr(bodyStart + 4);
		std::cout << "Body: " << _body << /*std::hex <<*/std::endl;
	} else {
		std::cout << "No body found in request.\n";
	}
	_fd = fd;
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
	//_response.clear();
	//_header.clear();
	//_content.clear();
	if (isCGIRequest(_uri))
		cgiRequest();
	else if (_method == "GET")
		getRequest();
	else if (_method == "POST") {
		if (_uri == "/data")
			handleFileUpload();
		else
			postRequest();
	}
	else if (_method == "DELETE") {
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
		file << _body;
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
		_contentLength = _content.length();
		setContentType();
		setHeader(200);
		_response += _header + _content;
	}
	sendFullResponse(_fd, _response);
}


void	Request::setPath()
{
	if (_method == "GET") {
		_path = "./www" + _uri;
		if (_uri == "/")
			_path = "./www/index.html";
	}
	else if (_method == "POST")
	{
		if (_uri == "/upload")
			_path = "./upload" + _uri + ".txt";
		if (_uri == "/data")
			_path = "./upload/data";
	}
	else if (_method == "DELETE")
	{
		_path = "." + _uri;
		/*if (_uri == "/")
			_path = "./upload/data.txt";*/
	}
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


void	Request::setHeader(int code)
{
	if (code == 200) {
		_header += "HTTP/1.1 200 OK\r\n";
	}
	else if (code == 201) {
		_header += "HHTP/1.1 201 Created\r\n";
	}
	_header += "Content-Type: " + _contentType + "\r\n";
	_header += "Content-Length: " + toString(_contentLength) + "\r\n";
	_header += "Connection: close\r\n\r\n";
}
