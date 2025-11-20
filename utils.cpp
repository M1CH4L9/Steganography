#include "utils.h"
#include <fstream>
#include <sys/stat.h>

bool is_file_supported(const std::string &file_path) {
    size_t dot_pos = file_path.find_last_of('.');
    if (dot_pos == std::string::npos) return false;

    std::string ext = file_path.substr(dot_pos + 1);
    return (ext == "bmp" || ext == "png" || ext == "ppm");
}

std::string get_file_extension(const std::string &file_path) {
    size_t dot_position = file_path.find_last_of('.');
    std::string extension = file_path.substr(dot_position + 1);
    return extension;
}

size_t get_file_size(const std::string &file_path) {
    struct stat file_stat;
    if (stat(file_path.c_str(), &file_stat) != 0) {
        return 0;
    }
    return file_stat.st_size;
}

FileInfo get_file_info(const std::string &file_path) {
    struct stat file_stat;
    if (stat(file_path.c_str(), &file_stat) != 0) {
        throw std::runtime_error("Brak uprawnien do pliku");
    }
    return {static_cast<size_t>(file_stat.st_size),file_stat.st_mtime};
}