
#ifndef	REQUEST_HPP
#define	REQUEST_HPP

#include <iostream>
#include <sstream>
#include <fstream>
#include "Log.hpp"
#include "Config.hpp"

class Request
{
	public:
		Request(const std::string& fullRequest, int fd);
		void	parseHeaders(std::stringstream &ss);
		void	handleRequest();
		void	setErrorResponse(int error);
		void	getRequest();
<<<<<<< post
		void	postRequest();
		void	setContentType(std::string &path);
=======
		void	cgiRequest();
		void	setPath();
		void	setContentType();
		std::string	getQueryString(const std::string &uri);
>>>>>>> main
		void	setHeader();
		~Request();

	private:
		int			_fd;
		int			_contentLength;
		std::string	_response;
		std::string	_method;
		std::string	_uri;
		std::map<std::string, std::string>	_headers;
		std::string _body;
		std::string	_protocol;
		std::string	_contentType;
		std::string	_header;
		std::string	_content;
		std::string	_path;
};

#endif