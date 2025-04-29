#include "HEADERS.h"

namespace file_system_manager {
    enum class file_system_status {
        SUCCESS,
        UnknownError,

        INCORRECT_NAME,

        FILE_DONT_EXISTS,
        FILE_ALREADY_EXISTS,

        DIRECTORY_DONT_EXISTS,
        DIRECTORY_ALREADY_EXISTS,
    };
    struct file_system_commands {
        static file_system_status show_files(const std::filesystem::path* dir_path, std::list<std::filesystem::directory_entry>& files) {
            if (! is_directory(*dir_path)) return file_system_status::DIRECTORY_DONT_EXISTS;

            files = std::list<std::filesystem::directory_entry>();
            for (const auto& file_name : std::filesystem::directory_iterator{*dir_path}) {
                files.push_back(file_name);
            }

            return file_system_status::SUCCESS;
        }
        static file_system_status delete_file(const std::filesystem::path* path) {
            if (! exists(*path)) return file_system_status::FILE_DONT_EXISTS;

            std::remove(path->string().c_str());

            return file_system_status::SUCCESS;
        }

        // Files
        static file_system_status create_file(const std::filesystem::path* file_path, const std::string* text) {
            if (exists(*file_path)) return file_system_status::FILE_ALREADY_EXISTS;

            std::ofstream file = std::ofstream{*file_path};
            file << *text << std::endl;
            file.close();

            return file_system_status::SUCCESS;
        }
        static file_system_status rewrite_file(const std::filesystem::path* file_path, const std::string* text) {
            if (! exists(*file_path)) return file_system_status::FILE_DONT_EXISTS;

            std::ofstream file = std::ofstream{*file_path};
            file << *text << std::endl;
            file.close();

            return file_system_status::SUCCESS;
        }
        static file_system_status create_or_rewrite_file(const std::filesystem::path* file_path, const std::string* text) {
            std::ofstream file = std::ofstream{*file_path};
            file << *text << std::endl;
            file.close();

            return file_system_status::SUCCESS;
        }
        static file_system_status change_file_data(const std::filesystem::path* file_path, const std::string *name=nullptr) {
            if (! exists(*file_path)) return file_system_status::FILE_DONT_EXISTS;

            if (name != nullptr) {
                std::rename(file_path->lexically_normal().string().c_str(), (file_path->lexically_normal().parent_path()/ *name).string().c_str());
            }

            return file_system_status::SUCCESS;
        }
        static file_system_status change_directory_data(const std::filesystem::path* file_path, const std::string *name=nullptr) {
            if (! exists(*file_path)) return file_system_status::DIRECTORY_DONT_EXISTS;

            if (name != nullptr) {
                std::rename(file_path->lexically_normal().string().c_str(), (file_path->lexically_normal().parent_path()/ *name).string().c_str());
            }

            return file_system_status::SUCCESS;
        }
        static file_system_status replace_file(const std::filesystem::path* old_file_path, const std::filesystem::path* new_file_path) {
            if (! exists(*old_file_path)) return file_system_status::FILE_DONT_EXISTS;
            if (exists(*new_file_path)) return file_system_status::FILE_ALREADY_EXISTS;
            if (! exists(new_file_path->parent_path())) return file_system_status::DIRECTORY_DONT_EXISTS;

            std::rename(old_file_path->string().c_str(), new_file_path->string().c_str());

            return file_system_status::SUCCESS;
        }

        // Directories
        static file_system_status create_directory(const std::filesystem::path* dir_path) {
            if (exists(*dir_path)) return file_system_status::DIRECTORY_ALREADY_EXISTS;

            std::filesystem::create_directories(*dir_path);

            return file_system_status::SUCCESS;
        }
    };
}