#include "HEADERS.h"

#include "Server.cpp"

#define DEFAULT_ADDRESS "127.0.0.1"
#define DEFAULT_PORT 8000
#define DEFAULT_BUFFER_SIZE 1024

int main(int argc, char *argv[]){
    std::string root_path, address;
    int port = -1;
    bool is_argument_valid = true;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--root_path") == 0 || std::strcmp(argv[i], "--address") == 0 || std::strcmp(argv[i], "--port") == 0) {
            if (i == argc - 1) {
                std::cout << R"(It must be at least one argument after "--root_path", "--address" and "--port")" << std::endl;
                is_argument_valid = false;
                break;
            }

            if (std::strcmp(argv[i], "--root_path") == 0) {
                root_path = argv[i + 1];
            }
            else if (std::strcmp(argv[i], "--address") == 0) {
                address = argv[i + 1];
            }
            else if (std::strcmp(argv[i], "--port") == 0) {
                if (! is_number(argv[i + 1])) {
                    std::cout << R"(It must be number after "--port")" << std::endl;
                    is_argument_valid = false;
                    break;
                }
                else {
                    port = std::stoi(argv[i + 1]);
                }
            }

            ++i;
        }
    }

    if (root_path.empty()) {
        root_path = std::filesystem::path(argv[0]).parent_path();
    }
    if (address.empty()) {
        address = DEFAULT_ADDRESS;
    }
    if (port == -1) {
        port = DEFAULT_PORT;
    }

    if (is_argument_valid) {
        Server::Server server = Server::Server(root_path, address, port);

        switch (server.Start()) {
            case Server::StartStatus::SUCCESS:
                getchar();

            server.Stop();
            break;
            case Server::StartStatus::SERVER_ALREADY_STARTED:
                std::cout << "Server already started" << std::endl;
            break;
            case Server::StartStatus::SOCKET_CREATION_ERROR:
                std::cout << "Failed to create socket" << std::endl;
            break;
            case Server::StartStatus::BINDING_ERROR:
                std::cout << "Failed to bind socket" << std::endl;
            break;
            case Server::StartStatus::LISTENING_ERROR:
                std::cout << "Failed of listening" << std::endl;
            break;
            case Server::StartStatus::DIRECTORY_DONT_EXISTS:
                std::cout << "Directory don't exists" << std::endl;
            break;
        }
    }

    return 0;
}
