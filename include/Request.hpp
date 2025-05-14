
#ifndef	REQUEST_HPP
#define	REQUEST_HPP

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include "Config.hpp"
#include "CGIRequest.hpp"

#define DEBUG 0

class Request
{
	public:
		Request(const std::string& fullRequest, int fd, const std::vector<ServerConfig> &servers);
		void	parseHeaders(std::stringstream &ss);
		void	handleRequest();
		void	setErrorResponse(int error);
		void	getRequest();
		void	postRequest();
		void	setContentType(std::string &path);
		void	cgiRequest();
		void	setPath();
		void	setContentType();
		std::string	getQueryString(const std::string &uri);
		void	setHeader(int code);
		void	setConfig(const std::vector<ServerConfig> &servers);
		const std::string	&getMethod() const;
		const std::string	&getUri() const;
		const std::string	&getProtocol() const;
		const std::string	&getHeader() const;
		const std::string	&getBody() const;
		ServerConfig	getConfig() const;
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
		std::string	_fullRequest;
		ServerConfig	_config;
};

#endif