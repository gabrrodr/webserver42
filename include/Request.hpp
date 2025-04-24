
#ifndef	REQUEST_HPP
#define	REQUEST_HPP

#include <iostream>
#include <sstream>
#include <fstream>
#include "Log.hpp"

class Request
{
	public:
		Request(char *buffer, int fd);
		void	handleRequest();
		void	setErrorResponse(int error);
		void	getRequest();
		void	setContentType(std::string &path);
		void	setHeader();
		~Request();

	private:
		int			_fd;
		std::string	_response;
		std::string	_method;
		std::string	_uri;
		std::string	_protocol;
		std::string	_contentType;
		std::string	_header;
		std::string	_content;
};

#endif