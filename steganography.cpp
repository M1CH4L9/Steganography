#include "steganography.h"
#include <fstream>
#include <vector>
#include <bitset>
#include <sstream>
#include <limits>
#include <algorithm>

#include "utils.h"

//rozmiar nagłówka BMP - 54 bajty
const int HEADER_SIZE = 54;

void encrypt_message_in_bmp(const std::string &input_path, const std::string &message) {
    std::ifstream input_file(input_path, std::ios::binary);
    if (!input_file.is_open()) {
        throw std::runtime_error("Nie mozna otworzyc pliku");
    }

    //czyta plik bajt po bajcie
    std::vector<unsigned char> file_data(
        (std::istreambuf_iterator<char>(input_file)),
        (std::istreambuf_iterator<char>()));

    if (file_data.size() < HEADER_SIZE) {
        throw std::runtime_error("Plik jest za malylub uszkodzony");
    }

    size_t max_message_length = (file_data.size() - HEADER_SIZE) / 8 - 1;
    if (message.size() > max_message_length) {
        throw std::runtime_error("Zbyt duza ilosc znakow. Limit: " + std::to_string(max_message_length));
    }

    std::string full_message = message + "@";

    size_t current_bit = 0;

    for (size_t i = HEADER_SIZE; i<file_data.size() && current_bit < full_message.size() * 8; ++i) {
        unsigned char current_char = full_message[current_bit / 8];
        bool bit = (current_char >> (7 - (current_bit % 8))) & 1;

        //01010101 & 11111110 = 01010100
        //01010100 | 00000001 = 01010101
        file_data[i] = (file_data[i] & 0xFE) | bit;

        ++current_bit;
    }

    std::ofstream output_file("output.bmp", std::ios::binary);
    //"rzutowanie" na char
    output_file.write(reinterpret_cast<const char*>(file_data.data()), file_data.size());
}

std::string decrypt_message_from_bmp(const std::string &input_path) {
    std::ifstream input_file(input_path, std::ios::binary);
    if (!input_file.is_open()) {
        throw std::runtime_error("Nie mozna otworzyc pliku");
    }

    std::vector<unsigned char> file_data(
        (std::istreambuf_iterator<char>(input_file)),
        (std::istreambuf_iterator<char>()));

    std::string message;
    unsigned char current_byte = 0;
    int bit_count = 0;

    for (size_t i = HEADER_SIZE;i< file_data.size(); ++i) {
        bool bit = file_data[i] & 1;
        current_byte = (current_byte << 1) | bit;
        ++bit_count;


        if (bit_count == 8) {
            if (current_byte == '@') {
                break;
            }
            message += static_cast<char>(current_byte);
            current_byte = 0;
            bit_count = 0;
        }
    }

    if (message.empty()) {
        throw std::runtime_error("Brak wiadomosci w pliku");
    }
    return message;
}

bool check_file_size(const std::string &input_path, const std::string &message) {
    std::string ext = get_file_extension(input_path);
    size_t available_bits = 0;

    if (ext == "bmp") {
        const size_t BMP_HEADER = 54;
        std::ifstream file(input_path, std::ios::binary | std::ios::ate);
        size_t file_size = file.tellg();
        available_bits = (file_size > BMP_HEADER) ? (file_size - BMP_HEADER) * 8 : 0;
    }
    else if (ext == "png") {
        std::ifstream file(input_path, std::ios::binary);
        std::vector<unsigned char> png_data(
            (std::istreambuf_iterator<char>(file)),
            (std::istreambuf_iterator<char>()));

        size_t idat_start = find_idat_chunk(png_data);
        if (idat_start == 0) return false;

        available_bits = (png_data.size() - idat_start) * 8;
    }
    else if (ext == "ppm") {
        std::ifstream file(input_path, std::ios::binary);
        std::string line;
        int width, height, max_val;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::stringstream ss(line);
            if (ss >> width >> height) break;
        }
        file >> max_val;

        size_t pixel_data_size = width * height * 3;
        available_bits = pixel_data_size * 8;
    }

    size_t required_bits = (message.size() + 1) * 8;
    return available_bits >= required_bits;
}

void encrypt_message_in_png(const std::string& input_path, const std::string& message) {
    std::ifstream file(input_path, std::ios::binary);

    std::vector<unsigned char> png_data(
        (std::istreambuf_iterator<char>(file)),
        (std::istreambuf_iterator<char>())
    );

    //początek chunka IDAT
    size_t idat_start = find_idat_chunk(png_data);
    if (idat_start == 0) {
        throw std::runtime_error("Brak IDAT chunk");
    }

    std::string full_message = message + "@";

    size_t bit_index = 0;

    for (size_t i = idat_start; i < png_data.size() && bit_index < full_message.size() * 8; ++i) {
        char current_bit = (full_message[bit_index / 8] >> (7 - (bit_index % 8))) & 1;
        png_data[i] = (png_data[i] & 0xFE) | current_bit;
        bit_index++;
    }

    std::ofstream output("output.png", std::ios::binary);
    output.write(reinterpret_cast<const char*>(png_data.data()), png_data.size());
    output.close();
}

std::string decrypt_message_from_png(const std::string& input_path) {
    std::ifstream file(input_path, std::ios::binary);
    std::vector<unsigned char> png_data(
        (std::istreambuf_iterator<char>(file)),
        (std::istreambuf_iterator<char>())
    );

    size_t idat_start = find_idat_chunk(png_data);

    if (idat_start == 0) {
        throw std::runtime_error("Brak IDAT chunk");
    }

    std::string message;
    //ciąg 8 bitów
    std::bitset<8> current_byte;
    size_t bit_count = 0;

    for (size_t i = idat_start; i < png_data.size(); ++i) {
        current_byte <<= 1;
        current_byte |= (png_data[i] & 1);
        bit_count++;

        if (bit_count == 8) {
            char c = static_cast<char>(current_byte.to_ulong());
            if (c == '@') {
                break;
            }
            message += c;
            current_byte.reset();
            bit_count = 0;
        }
    }

    return message;
}

void encrypt_message_in_ppm(const std::string& input_path, const std::string& message) {
    std::ifstream file(input_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Nie mozna otworzyc pliku");
    }

    std::string line;
    int width, height, max_val;

    bool found_p6 = false;
    while (std::getline(file, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        if (line.empty()) {
            continue;
        }
        if (line.substr(0, 2) == "P6") {
            found_p6 = true;
            break;
        }
    }
    if (!found_p6) {
        throw std::runtime_error("Nieprawidlowy format PPM (brak P6)");
    }

    bool found_dimensions = false;
    while (std::getline(file, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        if (line.empty()) {
            continue;
        }
        if (line[0] == '#') {
            continue;
        }

        std::stringstream ss(line);
        if (ss >> width >> height) {
            found_dimensions = true;
            break;
        }
    }
    if (!found_dimensions) {
        throw std::runtime_error("Nieprawidlowy format PPM (brak wymiarow)");
    }

    bool found_max_val = false;
    while (std::getline(file, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        std::stringstream ss(line);
        if (ss >> max_val) {
            found_max_val = true;
            break;
        }
    }
    if (!found_max_val) {
        throw std::runtime_error("Nieprawidlowy format PPM (brak max_val)");
    }
    if (max_val != 255) {
        throw std::runtime_error("Plik PPM musi mieć max_val = 255");
    }

    std::vector<unsigned char> pixels(
        (std::istreambuf_iterator<char>(file)),
        (std::istreambuf_iterator<char>())
    );
    file.close();

    if (pixels.size() != static_cast<size_t>(width * height * 3)) {
        throw std::runtime_error("Nieprawidlowy rozmiar danych pikseli");
    }

    //kodowanie wiadomośći w LSB kanału czerwonego
    std::string full_message = message + "@";
    size_t bit_idx = 0;
    for (size_t i = 0; i < pixels.size() && bit_idx < full_message.size() * 8; i += 3) {
        pixels[i] = (pixels[i] & 0xFE) | (full_message[bit_idx / 8] >> (7 - (bit_idx % 8))) & 1;
        bit_idx++;
    }

    std::ofstream output("output.ppm", std::ios::binary);
    output << "P6\n";
    output << width << " " << height << "\n";
    output << max_val << "\n";
    output.write(reinterpret_cast<const char*>(pixels.data()), pixels.size());
    output.close();
}

std::string decrypt_message_from_ppm(const std::string& input_path) {
    std::ifstream file(input_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Nie mozna otworzyc pliku");
    }

    std::string line;
    int width, height, max_val;

    bool found_p6 = false;
    while (std::getline(file, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        if (line.empty()) continue;
        if (line.substr(0, 2) == "P6") {
            found_p6 = true;
            break;
        }
    }
    if (!found_p6) {
        throw std::runtime_error("Nieprawidlowy format PPM (brak P6)");
    }

    bool found_dimensions = false;
    while (std::getline(file, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        std::stringstream ss(line);
        if (ss >> width >> height) {
            found_dimensions = true;
            break;
        }
    }
    if (!found_dimensions) {
        throw std::runtime_error("Nieprawidlowy format PPM (brak wymiarów)");
    }

    bool found_max_val = false;
    while (std::getline(file, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        std::stringstream ss(line);
        if (ss >> max_val) {
            found_max_val = true;
            break;
        }
    }
    if (!found_max_val) {
        throw std::runtime_error("Nieprawidlowy format PPM (brak max_val)");
    }

    std::vector<unsigned char> pixels(
        (std::istreambuf_iterator<char>(file)),
        (std::istreambuf_iterator<char>())
    );

    std::string message;
    std::bitset<8> bits;
    size_t bit_count = 0;

    for (size_t i = 0; i < pixels.size(); i += 3) {
        bits <<= 1;
        bits |= (pixels[i] & 1);
        bit_count++;

        if (bit_count == 8) {
            char c = static_cast<char>(bits.to_ulong());
            if (c == '@') break;
            message += c;
            bits.reset();
            bit_count = 0;
        }
    }

    return message;
}


size_t find_idat_chunk(const std::vector<unsigned char>& png_data) {
    for (size_t i = 0; i < png_data.size() - 8; ++i) {
        //hex dla IDAT chunk to 0x49 0x44 0x41 0x54
        if (png_data[i] == 0x49 &&
            png_data[i + 1] == 0x44 &&
            png_data[i + 2] == 0x41 &&
            png_data[i + 3] == 0x54)
        {
            //początek chunka IDAT po długości i type
            return i + 8;
        }
    }
    return 0;
}
