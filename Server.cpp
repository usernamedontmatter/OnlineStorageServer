#include "HEADERS.h"
#include "STANDARD_FUNCTIONS.cpp"
#include "FileSystemManager.cpp"

#define DEFAULT_ADDRESS "127.0.0.1"
#define DEFAULT_PORT 8000
#define DEFAULT_BUFFER_SIZE 1024

namespace Server {
    enum class StartStatus {
        SUCCESS,

        // Server Errors
        SERVER_ALREADY_STARTED,

        // Network Errors
        SOCKET_CREATION_ERROR,
        BINDING_ERROR,
        LISTENING_ERROR,

        // Input Errors
        DIRECTORY_DONT_EXISTS,
    };
    enum ResponseStatus {
        OK = 1,

        // Client Errors
        BAD_REQUEST = 2,
        INCORRECT_ARGUMENTS = 3,
        INCORRECT_COMMAND = 4,
        COMMAND_CANT_BE_EXECUTED = 5,

        // Server Errors
        SERVER_ERROR = 128,
    };

    class Server {
    private:
        std::filesystem::path base_directory;
        std::string address;
        int port;
        long long buffer_size;

        int socket_fd = 0;
        bool is_running;

        static std::string make_response(const FileSystemManager::FileSystemStatus status, const std::string& response_message = "") {
            std::string response;
            switch (status) {
                case FileSystemManager::FileSystemStatus::SUCCESS:
                    response = { static_cast<char>(OK) };
                    break;
                default:
                case FileSystemManager::FileSystemStatus::UnknownError:
                    response = { static_cast<char>(SERVER_ERROR) };
                    response += "Unknown error";
                    break;
                case FileSystemManager::FileSystemStatus::INCORRECT_NAME:
                    response = { static_cast<char>(COMMAND_CANT_BE_EXECUTED) };
                    response += "Incorrect name";
                    break;
                case FileSystemManager::FileSystemStatus::FILE_DONT_EXISTS:
                    response = { static_cast<char>(COMMAND_CANT_BE_EXECUTED) };
                    response += "File don't exists";
                    break;
                case FileSystemManager::FileSystemStatus::FILE_ALREADY_EXISTS:
                    response = { static_cast<char>(COMMAND_CANT_BE_EXECUTED) };
                    response += "File already exists";
                    break;
                case FileSystemManager::FileSystemStatus::DIRECTORY_DONT_EXISTS:
                    response = { static_cast<char>(COMMAND_CANT_BE_EXECUTED) };
                    response += "Directory don't exists";
                    break;
                case FileSystemManager::FileSystemStatus::DIRECTORY_ALREADY_EXISTS:
                    response = { static_cast<char>(COMMAND_CANT_BE_EXECUTED) };
                    response += "Directory already exists";
                    break;
            }

            response += response_message;

            return response;
        }

        void process_command(const int socket) const {
            auto buffer = new char[buffer_size];

            read(socket, &buffer[0], buffer_size);

            std::string request = buffer;
            const std::vector<std::string>* arr = split_with_delimiter_removing(&request);
            if (arr->empty()) {
                bzero(buffer, buffer_size);
                buffer[0] = static_cast<char>(INCORRECT_COMMAND);
                strcpy(buffer + 1, "Request must contain command");
                send(socket, &buffer[0], buffer_size, 0);
            }
            else if (arr->at(0) == "show_files" || arr->at(0) == "delete") {
                if (arr->size() < 2) {
                    bzero(buffer, buffer_size);
                    buffer[0] = static_cast<char>(INCORRECT_ARGUMENTS);
                    strcpy(buffer + 1, (" \"" + arr->at(0) + "\" command must have path to file as first argument").data());
                    send(socket, &buffer[0], buffer_size, 0);
                }
                else {
                    std::filesystem::path path = base_directory;
                    path += "/" + arr->at(1);
                    path = path.lexically_normal();

                    FileSystemManager::FileSystemStatus status;
                    std::string response_message;
                    if (arr->at(0) == "show_files") {
                        std::list<std::filesystem::directory_entry> files;
                        status = FileSystemManager::FileSystemCommands::ShowFiles(&path, files);

                        if (status == FileSystemManager::FileSystemStatus::SUCCESS) {
                            response_message = "";
                            for (auto it = files.begin(); it != files.end(); ++it) {
                               if (it->is_directory()) {
                                   response_message += "directory ";
                               }
                               else {
                                   response_message += "file ";
                               }

                               response_message += it->path().filename().string();
                               response_message += " ";
                           }
                        }
                    }
                    else if (arr->at(0) == "delete") {
                        response_message = "";
                        status = FileSystemManager::FileSystemCommands::Delete(&path);
                    }
                    else {
                        status = FileSystemManager::FileSystemStatus::UnknownError;
                    }

                    bzero(buffer, buffer_size);
                    strcpy(buffer, make_response(status, response_message).data());

                    send(socket, &buffer[0], buffer_size, 0);
                }
            }
            else if (arr->at(0) == "create_file" || arr->at(0) == "rewrite_file" || arr->at(0) == "create_or_rewrite_file") {
                if (arr->size() < 3) {
                    bzero(buffer, buffer_size);
                    buffer[0] = static_cast<char>(INCORRECT_ARGUMENTS);
                    strcpy(buffer + 1, (" \"" + arr->at(0) + "\" command must have path to file as first argument and file length as second argument").data());
                    send(socket, &buffer[0], buffer_size, 0);
                }
                else if(! is_number(arr->at(2))) {
                    bzero(buffer, buffer_size);
                    buffer[0] = static_cast<char>(INCORRECT_ARGUMENTS);
                    strcpy(buffer + 1, (" \"" + arr->at(0) + "\" command must have number as second argument").data());
                    send(socket, &buffer[0], buffer_size, 0);
                }
                else {
                    const long long file_size = stoll(arr->at(2));
                    std::filesystem::path path = base_directory;
                    path += "/" + arr->at(1);
                    path = path.lexically_normal();

                    std::string file_text;
                    for (int i = 0; i < div_with_round_up(file_size, buffer_size); i++) {
                        read(socket, buffer, buffer_size);
                        file_text += buffer;
                    }

                    FileSystemManager::FileSystemStatus status;
                    if (arr->at(0) == "create_file") {
                        status = FileSystemManager::FileSystemCommands::CreateFile(&path, &file_text);
                    }
                    else if (arr->at(0) == "rewrite_file") {
                        status = FileSystemManager::FileSystemCommands::RewriteFile(&path, &file_text);
                    }
                    else if (arr->at(0) == "create_or_rewrite_file") {
                        status = FileSystemManager::FileSystemCommands::CreateOrRewriteFile(&path, &file_text);
                    }
                    else {
                        status = FileSystemManager::FileSystemStatus::UnknownError;
                    }

                    bzero(buffer, buffer_size);
                    strcpy(buffer, make_response(status).data());

                    send(socket, &buffer[0], buffer_size, 0);
                }
            }
            else if (arr->at(0) == "change_file_data" or arr->at(0) == "change_directory_data") {
                if (arr->size() < 2) {
                    bzero(buffer, buffer_size);
                    buffer[0] = static_cast<char>(INCORRECT_ARGUMENTS);
                    strcpy(buffer + 1, (" \"" + arr->at(0) + "\" command must have path to file as first argument").data());
                    send(socket, &buffer[0], buffer_size, 0);
                }
                else {
                    bool is_arguments_ok = true;
                    std::string name;
                    for (auto it = arr->begin(); it != arr->end(); ++it) {
                        if (*it == "--name") {
                            ++it;
                            if (it != arr->end()) {
                                name = *it;
                            }
                            else {
                                is_arguments_ok = false;
                                bzero(buffer, buffer_size);
                                buffer[0] = static_cast<char>(INCORRECT_ARGUMENTS);
                                strcpy(buffer + 1, (" \"" + arr->at(0) + "\" command must have path to file after --name parameter").data());
                                send(socket, &buffer[0], buffer_size, 0);
                            }

                            break;
                        }
                    }

                    if (is_arguments_ok) {
                        std::filesystem::path path = base_directory;
                        path += "/" + arr->at(1);
                        path = path.lexically_normal();

                        FileSystemManager::FileSystemStatus status;
                        if (arr->at(0) == "change_file_data") {
                            status = FileSystemManager::FileSystemCommands::ChangeFileData(&path, name.empty() ? nullptr : &name);
                        }
                        else if (arr->at(0) == "change_directory_data") {
                            status = FileSystemManager::FileSystemCommands::ChangeDirectoryData(&path, name.empty() ? nullptr : &name);
                        }
                        else {
                            status = FileSystemManager::FileSystemStatus::UnknownError;
                        }

                        std::string response = make_response(status);

                        send(socket, &response[0], buffer_size, 0);
                    }
                }
            }
            else if(arr->at(0) == "replace_file") {
                if (arr->size() < 3) {
                    bzero(buffer, buffer_size);
                    buffer[0] = static_cast<char>(INCORRECT_ARGUMENTS);
                    strcpy(buffer + 1, (" \"" + arr->at(0) + "\" command must have old path to file as first argument and new path to file as second").data());
                    send(socket, &buffer[0], buffer_size, 0);
                }
                else {
                    std::filesystem::path old_path = base_directory;
                    old_path += "/" + arr->at(1);
                    old_path = old_path.lexically_normal();

                    std::filesystem::path new_path = base_directory;
                    new_path += "/" + arr->at(2);
                    new_path = new_path.lexically_normal();

                    std::string response = make_response(FileSystemManager::FileSystemCommands::ReplaceFile(&old_path, &new_path));

                    send(socket, &response[0], buffer_size, 0);
                }
            }
            else if (arr->at(0) == "create_directory") {
                if (arr->size() < 2) {
                    bzero(buffer, buffer_size);
                    buffer[0] = static_cast<char>(INCORRECT_ARGUMENTS);
                    strcpy(buffer + 1, (" \"" + arr->at(0) + "\" command must have path to folder as first argument").data());
                    send(socket, &buffer[0], buffer_size, 0);
                }
                else {
                    std::filesystem::path path = base_directory;
                    path += "/" + arr->at(1);
                    path = path.lexically_normal();

                    std::string response = make_response(FileSystemManager::FileSystemCommands::CreateDirectory(&path));

                    send(socket, &response[0], buffer_size, 0);
                }
            }
            else {
                bzero(buffer, buffer_size);
                buffer[0] = static_cast<char>(INCORRECT_COMMAND);
                strcpy(buffer + 1, ("It's no \"" + arr->at(0) + "\" command").data());
                send(socket, &buffer[0], buffer_size, 0);
            }

            close(socket);
            delete arr;
            delete[] buffer;
        }

        void run(sockaddr_in socket_address, int socket_address_len) {
            while (is_running) {
                int new_socket = accept(socket_fd, reinterpret_cast<sockaddr*>(&socket_address), reinterpret_cast<socklen_t*>(&socket_address_len));
                std::thread(&Server::process_command, this, new_socket).detach();
            }
        }

    public:
        explicit Server(const std::string& base_directory, const std::string &address = DEFAULT_ADDRESS, const int port = DEFAULT_PORT, const long long buffer_size = DEFAULT_BUFFER_SIZE) {
            this->base_directory = base_directory + "/";
            this->address = address;
            this->port = port;
            this->buffer_size = buffer_size;
            this->is_running = false;
        }
        ~Server() {
            this->Stop();
        }

        StartStatus Start() {
            if (!std::filesystem::exists(base_directory)) return StartStatus::DIRECTORY_DONT_EXISTS;
            if (is_running) return StartStatus::SERVER_ALREADY_STARTED;

            socket_fd = socket(AF_INET , SOCK_STREAM , 0);
            if(socket_fd == 0) return StartStatus::SOCKET_CREATION_ERROR;

            sockaddr_in socket_address;
            socket_address.sin_family = AF_INET;
            socket_address.sin_addr.s_addr = inet_addr(address.data());
            socket_address.sin_port = htons(port);

            size_t socket_address_len = sizeof(socket_address);
            if (bind(socket_fd, reinterpret_cast<sockaddr*>(&socket_address), socket_address_len) < 0) return StartStatus::BINDING_ERROR;
            if (listen(socket_fd, 3) < 0) return StartStatus::LISTENING_ERROR;

            is_running = true;
            std::thread(&Server::run, this, socket_address, socket_address_len).detach();

            return StartStatus::SUCCESS;
        }
        void Stop() {
            if (is_running) {
                is_running = false;
                close(socket_fd);
            }
        }
    };
}

#undef DEFAULT_ADDRESS
#undef DEFAULT_PORT
#undef DEFAULT_BUFFER_SIZE