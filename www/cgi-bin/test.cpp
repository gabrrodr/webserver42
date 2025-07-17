#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <map>
#include <cctype>

std::string htmlEscape(const std::string& input) {
	std::string output;
	for (char c : input) {
		switch (c) {
			case '&': output += "&amp;"; break;
			case '<': output += "&lt;"; break;
			case '>': output += "&gt;"; break;
			case '"': output += "&quot;"; break;
			case '\'': output += "&#x27;"; break;
			default: output += c;
		}
	}
	return output;
}

// Simple URL decoder
std::string urlDecode(const std::string& src) {
	std::string ret;
	char ch;
	int i, ii;
	for (i = 0; i < src.length(); i++) {
		if (src[i] == '%') {
			sscanf(src.substr(i + 1, 2).c_str(), "%x", &ii);
			ch = static_cast<char>(ii);
			ret += ch;
			i += 2;
		} else if (src[i] == '+') {
			ret += ' ';
		} else {
			ret += src[i];
		}
	}
	return ret;
}

// Parses application/x-www-form-urlencoded
std::map<std::string, std::string> parseFormData(const std::string& data) {
	std::map<std::string, std::string> result;
	std::istringstream stream(data);
	std::string pair;

	while (std::getline(stream, pair, '&')) {
		size_t eqPos = pair.find('=');
		if (eqPos != std::string::npos) {
			std::string key = urlDecode(pair.substr(0, eqPos));
			std::string value = urlDecode(pair.substr(eqPos + 1));
			result[key] = value;
		}
	}
	return result;
}

int main() {
	const char* contentLengthStr = std::getenv("CONTENT_LENGTH");
	int contentLength = contentLengthStr ? std::atoi(contentLengthStr) : 0;

	std::string postData(contentLength, '\0');
	std::cin.read(&postData[0], contentLength);

	std::map<std::string, std::string> params = parseFormData(postData);
	std::string msg = htmlEscape(params["msg"]);

	std::string responseBody =
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head>\n"
		"<link rel=\"stylesheet\" href=\"/assets/css/main.css\">\n"
		"<link rel=\"stylesheet\" href=\"/assets/css/header.css\">\n"
		"<title>Home</title>\n"
		"</head>\n"
		"<body>\n"
		"<div id=\"header\">\n"
		"<a href=\"/\">\n"
		"<img id=\"webserv-logo\" src=\"/assets/img/webserv-logo.png\" alt=\"webserv-logo\">\n"
		"</a>\n"
		"</div>\n"
		"<div id=\"container\">\n"
		"<h1>POST some text</h1>\n"
		"<form action=\"/cgi-bin/testcpp.cgi\" method=\"POST\">\n"
		"<label for=\"msg\">Message:</label>\n"
		"<input type=\"text\" name=\"msg\" id=\"msg\">\n"
		"<input type=\"submit\" value=\"Send\">\n"
		"</form>\n"
		"<h3>Message:</h3>\n"
		"<p style=\"word-wrap: break-word;\">" + msg + "</p>\n"
		"</div>\n"
		"</body>\n"
		"</html>\n";

	std::cout << "Content-Type: text/html; charset=UTF-8\r\n\r\n";
	std::cout << responseBody;

	return 0;
}