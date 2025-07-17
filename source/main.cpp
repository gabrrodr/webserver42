#include <fstream>
#include "../include/MainHeader.hpp"
#include "../include/Log.hpp"
#include "../include/Config.hpp"

// Configurable launch- check
// default config generator (--generate creates a default config file)
// Parsing/Errorr handling - try catch handle configParser. 
// Handle incoming connections entry point for server loop
/*to do:
    - log::error() prints to stderr?
    - handles warnings and fatal errors?
    - flush logs immediately?
    */
void    write_line(std::ofstream &file, const std::string &line) {
    file << line << std::endl;
}

void    generate_file() {
    std::string path = "./example.conf";
    std::ofstream   file;

    file.open(path.c_str(), std::ios::out);
    if (!file.is_open()) {
        Log::error("parser error: Could not open file\n " + path);
        exit(1) ;
    }
    write_line(file, "server {");
    write_line(file, "\tlisten 8080");
    write_line(file, "\tserver_name localhost");
    write_line(file, "\tlocation / {");
    write_line(file, "\t\troot www/");
    write_line(file, "\t\tindex index.html");
    write_line(file, "\t}");
    write_line(file, "\tlocation / {");
    write_line(file, "\t\tmethods GET");
    write_line(file, "\t}");
    write_line(file, "\tlocation / {");
    write_line(file, "\t\tmethods POST");
    write_line(file, "\t}");
    write_line(file, "}");
    std::cout << "Config file generated at: " << path << std::endl;
    exit(0);
}

int    help(char **argv) {
    if (argv[1] && (!std::strcmp(argv[1], "--generate"))) {
        generate_file();
        return (1);
    }
    return (0);
}

int main(int argc, char **argv, char **env) {

    (void)env;
    if (argc != 2) {
        Log::error("❌ Invalid usage!\n"
            "Usage: ./webserv [configuration file]\n"
            "Options:\n"
            "  --generate\tGenerate a default configuration file");
        return (1);
    }
    if (!help(argv) && argc == 2) {
        try {
            ConfigParser    parser(argv[1]);
            std::vector<ServerConfig>   servers = parser.parse();

            for (size_t i = 0; i < servers.size(); ++i)
            {
                std::cout << "Server " << i + 1 << ":\n";
                std::cout << "Port: ";
                for (size_t p = 0; p < servers[i].port.size(); p++)
                    std::cout << servers[i].port[p] << " ";
                std::cout << "\n  Server Name: " << servers[i].server_name << "\n";
                std::cout << "  Error Pages:\n";
                for (std::map<int, std::string>::iterator it = servers[i].error_page.begin(); it != servers[i].error_page.end(); ++it)
                    std::cout << "    " << it->first << " " << it->second << '\n';
                std::cout << "  Max Body Size: " << servers[i].client_max_body_size << " bytes\n";
                for (std::map<std::string, Location>::iterator it = servers[i].locations.begin(); it != servers[i].locations.end(); ++it) {
                    std::cout << "  Location: " << it->first << "\n";
                    std::cout << "    url:  " << it->second.url << '\n';
                    std::cout << "    Root: " << it->second.root << "\n";
                    std::cout << "    Index: " << it->second.index << "\n";
                    std::cout << "    Methods: ";
                    for (size_t j = 0; j < it->second.methods.size(); ++j) {
                        std::cout << it->second.methods[j] << (j + 1 < it->second.methods.size() ? ", " : "");
                    }
                    std::cout << "\n";
                    if (!it->second.upload_dir.empty()) {
                        std::cout << "    Upload Dir: " << it->second.upload_dir << "\n";
                    }
                    if (it->second.autoindex)
                        std::cout << "    autoindex: true\n";
                }
            }
            parser.handleIncomingConnections(servers);
        } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
        }
    }
        
    return 0;
}