#pragma once
#include <string>
#include <vector>

void encrypt_message_in_bmp(const std::string& input_path, const std::string& message);
std::string decrypt_message_from_bmp(const std::string& input_path);

void encrypt_message_in_png(const std::string& input_path, const std::string& message);
std::string decrypt_message_from_png(const std::string& input_path);

void encrypt_message_in_ppm(const std::string& input_path, const std::string& message);
std::string decrypt_message_from_ppm(const std::string& input_path);

size_t find_idat_chunk(const std::vector<unsigned char>& png_data);

bool check_file_size(const std::string& input_path, const std::string& message);
