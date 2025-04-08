#include "Log.hpp"

void    Log::error(const std::string &msg) {
    struct  tm   *local_time;
    time_t              rawtime;
    char                buffer[32];

    time(&rawtime);
    local_time = localtime(&rawtime);
    strftime(buffer, 32, "%T", local_time);
    std::cerr << "[" << buffer << "]" << msg << std::endl;
}