/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrrodr <gabrrodr@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/16 18:01:18 by gabrrodr          #+#    #+#             */
/*   Updated: 2024/08/16 18:01:18 by gabrrodr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MainHeader.hpp"

void    write_line(std::ofstream &file, const std::string &line) {
    file << line << std::endl;
}

void    generate_file() {
    std::string path = "./example";
    std::ofstream   file;

    file.open(path.c_str(), std::ios::out);
    if (!file.is_open()) {
        Log::error("parser error: Could not open file " + std::string(file));
        exit(1) ;
    }
    write_line(file, "server {");
    write_line(file, "\tlisten 8080");
    write_line(file, "\tserver_name localhost");
    write_line(file, "\tdocroot /var/www/");
    write_line(file, "\tindex index.html");
    write_line(file, "\tlocation /onlyget {");
    write_line(file, "\t\tmethods GET");
    write_line(file, "\t}");
    write_line(file, "\tlocation /onlypost {");
    write_line(file, "\t\tmethods POST");
    write_line(file, "\t}");
    write_line(file, "}");
    std::cout << "Config file generated at: " << path << std::endl;
    exit(0);
}

void    help(char **argv) {
    if (argv[1] && (!std::strcmp(argv[1], "--generate"))) {
            generate_file();
    }
}

int main(int argc, char **argv, char **env) {
    //copyEnv(env);
    if (argc != 2) {
        std::cout << "Invalid! Usage: ./webserv [configuration file]" << std::endl;
        std::cout << "options: ./webserv --generate (generate default configuration file)" << std::endl;
        return (1);
    }
    help(argv);
    
}