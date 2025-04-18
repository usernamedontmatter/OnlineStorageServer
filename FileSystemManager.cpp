#include "HEADERS.h"

namespace FileSystemManager {
    enum class FileSystemStatus {
        SUCCESS,
        UnknownError,

        INCORRECT_NAME,

        FILE_DONT_EXISTS,
        FILE_ALREADY_EXISTS,

        DIRECTORY_DONT_EXISTS,
        DIRECTORY_ALREADY_EXISTS,
    };
    struct FileSystemCommands {
        static FileSystemStatus ShowFiles(const std::filesystem::path* dir_path, std::list<std::filesystem::directory_entry>& files) {
            if (! is_directory(*dir_path)) return FileSystemStatus::DIRECTORY_DONT_EXISTS;

            files = std::list<std::filesystem::directory_entry>();
            for (const auto& file_name : std::filesystem::directory_iterator{*dir_path}) {
                files.push_back(file_name);
            }

            return FileSystemStatus::SUCCESS;
        }
        static FileSystemStatus Delete(const std::filesystem::path* path) {
            if (! exists(*path)) return FileSystemStatus::FILE_DONT_EXISTS;

            std::remove(path->string().c_str());

            return FileSystemStatus::SUCCESS;
        }

        // Files
        static FileSystemStatus CreateFile(const std::filesystem::path* file_path, const std::string* text) {
            if (exists(*file_path)) return FileSystemStatus::FILE_ALREADY_EXISTS;

            std::ofstream file = std::ofstream{*file_path};
            file << *text << std::endl;
            file.close();

            return FileSystemStatus::SUCCESS;
        }
        static FileSystemStatus RewriteFile(const std::filesystem::path* file_path, const std::string* text) {
            if (! exists(*file_path)) return FileSystemStatus::FILE_DONT_EXISTS;

            std::ofstream file = std::ofstream{*file_path};
            file << *text << std::endl;
            file.close();

            return FileSystemStatus::SUCCESS;
        }
        static FileSystemStatus CreateOrRewriteFile(const std::filesystem::path* file_path, const std::string* text) {
            std::ofstream file = std::ofstream{*file_path};
            file << *text << std::endl;
            file.close();

            return FileSystemStatus::SUCCESS;
        }
        static FileSystemStatus ChangeFileData(const std::filesystem::path* file_path, const std::string *name=nullptr) {
            if (! exists(*file_path)) return FileSystemStatus::FILE_DONT_EXISTS;

            if (name != nullptr) {
                std::rename(file_path->lexically_normal().string().c_str(), (file_path->lexically_normal().parent_path()/ *name).string().c_str());
            }

            return FileSystemStatus::SUCCESS;
        }
        static FileSystemStatus ChangeDirectoryData(const std::filesystem::path* file_path, const std::string *name=nullptr) {
            if (! exists(*file_path)) return FileSystemStatus::DIRECTORY_DONT_EXISTS;

            if (name != nullptr) {
                std::rename(file_path->lexically_normal().string().c_str(), (file_path->lexically_normal().parent_path()/ *name).string().c_str());
            }

            return FileSystemStatus::SUCCESS;
        }
        static FileSystemStatus ReplaceFile(const std::filesystem::path* old_file_path, const std::filesystem::path* new_file_path) {
            if (! exists(*old_file_path)) return FileSystemStatus::FILE_DONT_EXISTS;
            if (exists(*new_file_path)) return FileSystemStatus::FILE_ALREADY_EXISTS;
            if (! exists(new_file_path->parent_path())) return FileSystemStatus::DIRECTORY_DONT_EXISTS;

            std::rename(old_file_path->string().c_str(), new_file_path->string().c_str());

            return FileSystemStatus::SUCCESS;
        }

        // Directories
        static FileSystemStatus CreateDirectory(const std::filesystem::path* dir_path) {
            if (exists(*dir_path)) return FileSystemStatus::DIRECTORY_ALREADY_EXISTS;

            std::filesystem::create_directories(*dir_path);

            return FileSystemStatus::SUCCESS;
        }
    };
}