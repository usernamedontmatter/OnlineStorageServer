#include "HEADERS.h"
#include "STANDARD_FUNCTIONS.cpp"
#include "FileSystemManager.cpp"

#define DEFAULT_ADDRESS "127.0.0.1"
#define DEFAULT_PORT 8000
#define DEFAULT_BUFFER_SIZE 1024

namespace server {
    enum class start_status {
        success,

        // Server Errors
        server_already_started,

        // Network Errors
        socket_creation_error,
        binding_error,
        listening_error,

        // Input Errors
        directory_dont_exists,
    };
    enum response_status {
        ok = 1,

        // Client Errors
        bad_request = 2,
        incorrect_arguments = 3,
        incorrect_command = 4,
        command_cant_be_executed = 5,

        // Server Errors
        server_error = 128,
    };

    class server {
    private:
        std::filesystem::path base_directory;
        std::string address;
        int port;
        long long buffer_size;

        int socket_fd = 0;
        bool is_running;

        static std::string make_response(const file_system_manager::file_system_status status, const std::string& response_message = "") {
            std::string response;
            switch (status) {
                case file_system_manager::file_system_status::success:
                    response = { static_cast<char>(ok) };
                    break;
                default:
                case file_system_manager::file_system_status::unknown_error:
                    response = { static_cast<char>(server_error) };
                    response += "Unknown error";
                    break;
                case file_system_manager::file_system_status::incorrect_name:
                    response = { static_cast<char>(command_cant_be_executed) };
                    response += "Incorrect name";
                    break;
                case file_system_manager::file_system_status::file_dont_exists:
                    response = { static_cast<char>(command_cant_be_executed) };
                    response += "File don't exists";
                    break;
                case file_system_manager::file_system_status::file_already_exists:
                    response = { static_cast<char>(command_cant_be_executed) };
                    response += "File already exists";
                    break;
                case file_system_manager::file_system_status::directory_dont_exists:
                    response = { static_cast<char>(command_cant_be_executed) };
                    response += "Directory don't exists";
                    break;
                case file_system_manager::file_system_status::directory_already_exists:
                    response = { static_cast<char>(command_cant_be_executed) };
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
                buffer[0] = static_cast<char>(incorrect_command);
                strcpy(buffer + 1, "Request must contain command");
                write(socket, &buffer[0], buffer_size);
            }
            else if (arr->at(0) == "show_files" || arr->at(0) == "read" || arr->at(0) == "delete") {
                if (arr->size() < 2) {
                    bzero(buffer, buffer_size);
                    buffer[0] = static_cast<char>(incorrect_arguments);
                    strcpy(buffer + 1, (" \"" + arr->at(0) + "\" command must have path to file as first argument").data());+
                    write(socket, &buffer[0], buffer_size);
                }
                else {
                    std::filesystem::path path = base_directory;
                    path += "/" + arr->at(1);
                    path = path.lexically_normal();

                    file_system_manager::file_system_status status;
                    std::string response_message;
                    if (arr->at(0) == "show_files") {
                        std::list<std::filesystem::directory_entry> files;
                        status = file_system_manager::file_system_commands::show_files(&path, files);

                        if (status == file_system_manager::file_system_status::success) {
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
                    else if (arr->at(0) == "read") {
                        response_message = "";
                        status = file_system_manager::file_system_commands::read_file(&path, response_message);
                    }
                    else if (arr->at(0) == "delete") {
                        response_message = "";
                        status = file_system_manager::file_system_commands::delete_file(&path);
                    }
                    else {
                        status = file_system_manager::file_system_status::unknown_error;
                    }

                    std::string response = make_response(status, response_message);

                    for (long long i = 0; i < response.length(); i += buffer_size) {
                        bzero(buffer, buffer_size);
                        strcpy(buffer, response.substr(i, buffer_size).data());

                        write(socket, &buffer[0], buffer_size);
                    }
                }
            }
            else if (arr->at(0) == "create_file" || arr->at(0) == "rewrite_file" || arr->at(0) == "create_or_rewrite_file") {
                if (arr->size() < 3) {
                    bzero(buffer, buffer_size);
                    buffer[0] = static_cast<char>(incorrect_arguments);
                    strcpy(buffer + 1, (" \"" + arr->at(0) + "\" command must have path to file as first argument and file length as second argument").data());
                    write(socket, &buffer[0], buffer_size);
                }
                else if(! is_number(arr->at(2))) {
                    bzero(buffer, buffer_size);
                    buffer[0] = static_cast<char>(incorrect_arguments);
                    strcpy(buffer + 1, (" \"" + arr->at(0) + "\" command must have number as second argument").data());
                    write(socket, &buffer[0], buffer_size);
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

                    file_system_manager::file_system_status status;
                    if (arr->at(0) == "create_file") {
                        status = file_system_manager::file_system_commands::create_file(&path, &file_text);
                    }
                    else if (arr->at(0) == "rewrite_file") {
                        status = file_system_manager::file_system_commands::rewrite_file(&path, &file_text);
                    }
                    else if (arr->at(0) == "create_or_rewrite_file") {
                        status = file_system_manager::file_system_commands::create_or_rewrite_file(&path, &file_text);
                    }
                    else {
                        status = file_system_manager::file_system_status::unknown_error;
                    }

                    bzero(buffer, buffer_size);
                    strcpy(buffer, make_response(status).data());

                    write(socket, &buffer[0], buffer_size);
                }
            }
            else if (arr->at(0) == "change_data" || arr->at(0) == "change_file_data" || arr->at(0) == "change_directory_data") {
                if (arr->size() < 2) {
                    bzero(buffer, buffer_size);
                    buffer[0] = static_cast<char>(incorrect_arguments);
                    strcpy(buffer + 1, (" \"" + arr->at(0) + "\" command must have path to file as first argument").data());
                    write(socket, &buffer[0], buffer_size);
                }
                else {
                    bool is_arguments_ok = true;
                    std::string name;
                    std::string directory;
                    for (auto it = arr->begin(); it != arr->end(); ++it) {
                        if (*it == "--name") {
                            ++it;
                            if (it != arr->end()) {
                                name = *it;
                            }
                            else {
                                is_arguments_ok = false;
                                bzero(buffer, buffer_size);
                                buffer[0] = static_cast<char>(incorrect_arguments);
                                strcpy(buffer + 1, (" \"" + arr->at(0) + "\" command must have path to file after --name parameter").data());
                                write(socket, &buffer[0], buffer_size);
                            }
                        }
                        else if (*it == "--dir") {
                            ++it;
                            if (it != arr->end()) {
                                directory = *it;
                            }
                            else {
                                is_arguments_ok = false;
                                bzero(buffer, buffer_size);
                                buffer[0] = static_cast<char>(incorrect_arguments);
                                strcpy(buffer + 1, (" \"" + arr->at(0) + "\" command must have path to new directory after --dir parameter").data());
                                write(socket, &buffer[0], buffer_size);
                            }
                        }
                    }

                    if (!directory.empty()) {
                        std::filesystem::path relative_path = base_directory;
                        relative_path += "/" + directory;
                        relative_path = relative_path.lexically_normal();
                        directory = relative_path.generic_string();
                    }

                    if (is_arguments_ok) {
                        std::filesystem::path path = base_directory;
                        path += "/" + arr->at(1);
                        path = path.lexically_normal();

                        file_system_manager::file_system_status status;
                        if (arr->at(0) == "change_data") {
                            status = file_system_manager::file_system_commands::change_data(&path, name.empty() ? nullptr : &name, directory.empty() ? nullptr : &directory);
                        }
                        else if (arr->at(0) == "change_file_data") {
                            status = file_system_manager::file_system_commands::change_file_data(&path, name.empty() ? nullptr : &name, directory.empty() ? nullptr : &directory);
                        }
                        else if (arr->at(0) == "change_directory_data") {
                            status = file_system_manager::file_system_commands::change_directory_data(&path, name.empty() ? nullptr : &name, directory.empty() ? nullptr : &directory);
                        }
                        else {
                            status = file_system_manager::file_system_status::unknown_error;
                        }

                        bzero(buffer, buffer_size);
                        strcpy(buffer, make_response(status).data());

                        write(socket, &buffer[0], buffer_size);
                    }
                }
            }
            else if(arr->at(0) == "replace_file") {
                if (arr->size() < 3) {
                    bzero(buffer, buffer_size);
                    buffer[0] = static_cast<char>(incorrect_arguments);
                    strcpy(buffer + 1, (" \"" + arr->at(0) + "\" command must have old path to file as first argument and new path to file as second").data());
                    write(socket, &buffer[0], buffer_size);
                }
                else {
                    std::filesystem::path old_path = base_directory;
                    old_path += "/" + arr->at(1);
                    old_path = old_path.lexically_normal();

                    std::filesystem::path new_path = base_directory;
                    new_path += "/" + arr->at(2);
                    new_path = new_path.lexically_normal();

                    bzero(buffer, buffer_size);
                    strcpy(buffer, make_response(file_system_manager::file_system_commands::replace_file(&old_path, &new_path)).data());

                    write(socket, &buffer[0], buffer_size);
                }
            }
            else if (arr->at(0) == "create_directory") {
                if (arr->size() < 2) {
                    bzero(buffer, buffer_size);
                    buffer[0] = static_cast<char>(incorrect_arguments);
                    strcpy(buffer + 1, (" \"" + arr->at(0) + "\" command must have path to folder as first argument").data());
                    write(socket, &buffer[0], buffer_size);
                }
                else {
                    std::filesystem::path path = base_directory;
                    path += "/" + arr->at(1);
                    path = path.lexically_normal();

                    bzero(buffer, buffer_size);
                    strcpy(buffer, make_response(file_system_manager::file_system_commands::create_directory(&path)).data());

                    write(socket, &buffer[0], buffer_size);
                }
            }
            else {
                bzero(buffer, buffer_size);
                buffer[0] = static_cast<char>(incorrect_command);
                strcpy(buffer + 1, ("It's no \"" + arr->at(0) + "\" command").data());
                write(socket, &buffer[0], buffer_size);
            }

            close(socket);
            delete arr;
            delete[] buffer;
        }

        void run(sockaddr_in socket_address, int socket_address_len) {
            while (is_running) {
                int new_socket = accept(socket_fd, reinterpret_cast<sockaddr*>(&socket_address), reinterpret_cast<socklen_t*>(&socket_address_len));
                std::thread(&server::process_command, this, new_socket).detach();
            }
        }

    public:
        explicit server(const std::string& base_directory, const std::string &address = DEFAULT_ADDRESS, const int port = DEFAULT_PORT, const long long buffer_size = DEFAULT_BUFFER_SIZE) {
            this->base_directory = base_directory + "/";
            this->address = address;
            this->port = port;
            this->buffer_size = buffer_size;
            this->is_running = false;
        }
        ~server() {
            this->stop();
        }

        start_status start() {
            if (!std::filesystem::exists(base_directory)) return start_status::directory_dont_exists;
            if (is_running) return start_status::server_already_started;

            socket_fd = socket(AF_INET , SOCK_STREAM , 0);
            if(socket_fd == 0) return start_status::socket_creation_error;

            sockaddr_in socket_address;
            socket_address.sin_family = AF_INET;
            socket_address.sin_addr.s_addr = inet_addr(address.data());
            socket_address.sin_port = htons(port);

            size_t socket_address_len = sizeof(socket_address);
            if (bind(socket_fd, reinterpret_cast<sockaddr*>(&socket_address), socket_address_len) < 0) return start_status::binding_error;
            if (listen(socket_fd, 3) < 0) return start_status::listening_error;

            is_running = true;
            std::thread(&server::run, this, socket_address, socket_address_len).detach();

            return start_status::success;
        }
        void stop() {
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