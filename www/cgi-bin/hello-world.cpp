#include <iostream>
#include <cstdlib>

int main() {
	std::cout << "Content-Type: text/plain\r\n\r\n";
	std::cout << "Hello, CGI World!\n";
	std::cout <<
	"		<div id=\"content\"> \
				<img src=\"/assets/img/cat-1.jpeg\" alt=\"cat-1\"> \
				<img src=\"/assets/img/cat-2.jpeg\" alt=\"cat-2\"> \
				<img src=\"/assets/img/cat-3.jpeg\" alt=\"cat-3\"> \
			</div>";
	return 0;
}