#pragma once
#include <ctime>
#include <string>

bool is_file_supported(const std::string& file_path);
std::string get_file_extension(const std::string& file_path);
size_t get_file_size(const std::string& file_path);
struct FileInfo {
    size_t size;
    std::time_t last_modified;
};
FileInfo get_file_info(const std::string& file_path);