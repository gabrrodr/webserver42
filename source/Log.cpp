#include "../include/Log.hpp"
/*

add log levels and colorrrrr

RED "\033[31m"
RESET "\033[0m"

enum Level { ERROR, INFO, DEBUG }

void log(Level level, const std::string& msg) {
*/
void    Log::error(const std::string &msg) {
    struct  tm   *local_time;
    time_t              rawtime;
    char                buffer[32];

    time(&rawtime);
    local_time = localtime(&rawtime);
    strftime(buffer, 32, "%T", local_time);
    std::cerr << "[" << buffer << "]" << msg << std::endl;
}