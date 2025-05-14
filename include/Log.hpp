#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>

class Log {
    public:
        static void    error(const std::string &msg);
        static void    message(const std::string &msg);
};
#endif