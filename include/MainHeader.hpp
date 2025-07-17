#ifndef MAIN_HEADER_HPP
#define MAIN_HEADER_HPP

#include "SimpleSocket.hpp"
#include "ListeningSocket.hpp"
#include "ConnectionSocket.hpp"
#include "BindingSocket.hpp"
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <sys/socket.h>
#include <ctime>

#define RED "\033[38;5;196m"
#define YELLOW "\033[38;5;226m"
#define GREEN "\033[38;5;47m"
#define RESET "\033[0m"
#define CLEAR "\033[2J\033[H"

# define DEF_CONFIG "./conf/valid/example.conf"
#endif