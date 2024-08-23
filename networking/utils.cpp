#include "MainHeader.hpp"
#include "utils.hpp"


char    **copyEnv(char **env) {
    static char**   copy = NULL;

    if (env) {
        copy = env;
    }
    return (copy);
}