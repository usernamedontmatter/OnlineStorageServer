#include "HEADERS.h"

#define FORBIDDEN_FILENAME_CHARACTERS "\\/:*?\"<>|"

namespace file_system_manager {
    enum class file_system_status {
        success,
        unknown_error,

        incorrect_path,
        incorrect_name,

        file_dont_exists,
        file_already_exists,

        directory_dont_exists,
        directory_already_exists,

        directory_not_empty,
    };
    struct file_system_commands {
    private:
        static bool is_valid_name(const std::string& name) {
            return !(name.find_first_of(FORBIDDEN_FILENAME_CHARACTERS) != std::string::npos || std::filesystem::path(name).has_parent_path());
        }
    public:
        static file_system_status show_files(const std::filesystem::path& dir_path, std::list<std::filesystem::directory_entry>& files) {
            if (! is_directory(dir_path)) return file_system_status::directory_dont_exists;

            files = std::list<std::filesystem::directory_entry>();
            for (const auto& file_name : std::filesystem::directory_iterator{dir_path}) {
                files.push_back(file_name);
            }

            return file_system_status::success;
        }
        static file_system_status read_file(const std::filesystem::path& path, std::string& file_text) {
            if (! exists(path) || std::filesystem::is_directory(path)) return file_system_status::file_dont_exists;

            std::ifstream file = std::ifstream{path};
            file_text = "";
            std::string line;
            while(getline(file, line)) {
                file_text.append(line + "\n");
            }
            if (!file_text.empty()) file_text.pop_back();
            file.close();

            return file_system_status::success;
        }
        static file_system_status delete_file(const std::filesystem::path& path) {
            if (! exists(path)) return file_system_status::file_dont_exists;

            if (std::filesystem::is_directory(path) && !std::filesystem::is_empty(path)) {
                return file_system_status::directory_not_empty;
            }

            std::remove(path.c_str());
            return file_system_status::success;
        }
        static file_system_status delete_all(const std::filesystem::path& path) {
            if (! exists(path)) return file_system_status::file_dont_exists;

            if (std::filesystem::is_directory(path) && !std::filesystem::is_empty(path)) {
                for (const auto& entry : std::filesystem::directory_iterator{path}) {
                    delete_all(entry.path());
                }
            }

            std::remove(path.c_str());
            return file_system_status::success;
        }

        // Files
        static file_system_status create_file(const std::filesystem::path& file_path, const std::string& text) {
            if (exists(file_path)) return file_system_status::file_already_exists;

            std::ofstream file = std::ofstream{file_path};
            file << text << std::endl;
            file.close();

            return file_system_status::success;
        }
        static file_system_status rewrite_file(const std::filesystem::path& file_path, const std::string& text) {
            if (! exists(file_path) || std::filesystem::is_directory(file_path)) return file_system_status::file_dont_exists;

            std::ofstream file = std::ofstream{file_path};
            file << text << std::endl;
            file.close();

            return file_system_status::success;
        }
        static file_system_status create_or_rewrite_file(const std::filesystem::path& file_path, const std::string& text) {
            std::ofstream file = std::ofstream{file_path};
            file << text << std::endl;
            file.close();

            return file_system_status::success;
        }
        static file_system_status change_data(const std::filesystem::path& file_path, const std::string *new_name=nullptr, const std::string* new_directory=nullptr) {
            if (! exists(file_path)) return file_system_status::file_dont_exists;
            if (new_name != nullptr and !is_valid_name(*new_name)) return file_system_status::incorrect_name;

            std::filesystem::path parent_path = new_directory == nullptr ? file_path.lexically_normal().parent_path().string() : *new_directory;
            std::filesystem::path name = new_name == nullptr ? file_path.lexically_normal().filename().string() : *new_name;
;
            std::filesystem::path new_path = (parent_path/ name);

            if (exists(new_path)) return file_system_status::file_already_exists;

            std::rename(file_path.lexically_normal().string().c_str(), new_path.string().c_str());

            return file_system_status::success;
        }
        static file_system_status change_file_data(const std::filesystem::path& file_path, const std::string *name=nullptr, const std::string* new_directory=nullptr) {
            if (! exists(file_path) || std::filesystem::is_directory(file_path)) return file_system_status::file_dont_exists;

            return change_data(file_path, name, new_directory);
        }
        static file_system_status change_directory_data(const std::filesystem::path& file_path, const std::string *name=nullptr, const std::string* new_directory=nullptr) {
            if (! exists(file_path)) return file_system_status::directory_dont_exists;

            return change_data(file_path, name, new_directory);
        }

        // Directories
        static file_system_status create_directory(const std::filesystem::path& dir_path) {
            if (exists(dir_path)) return file_system_status::directory_already_exists;

            std::filesystem::create_directories(dir_path);

            return file_system_status::success;
        }

        // Outdated function
        static file_system_status replace_file(const std::filesystem::path& old_file_path, const std::filesystem::path& new_file_path) {
            if (! exists(old_file_path)) return file_system_status::file_dont_exists;
            if (exists(new_file_path)) return file_system_status::file_already_exists;
            if (! exists(new_file_path.parent_path())) return file_system_status::directory_dont_exists;

            std::rename(old_file_path.string().c_str(), new_file_path.string().c_str());

            return file_system_status::success;
        }
    };
}

#undef FORBIDDEN_FILENAME_CHARACTERS;