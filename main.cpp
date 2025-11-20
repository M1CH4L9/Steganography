#include <ctime>
#include <iostream>
#include <string>
#include "steganography.h"
#include "utils.h"

void print_help() {
    std::cout << "Uzycie:\n"
    << " -h/--help - pomoc\n"
    << " -i/--info [sciezka] - informacje o pliku\n"
    << " -c/--check [sciezka] [wiadomosc] - sprawdz mozliwosc zapisu\n"
    << " -e/--encrypt [sciezka] [wiadomosc] - zakoduj wiadomosc\n"
    << " -d/--decrypt [sciezka] - odkoduj wiadomosc\n";
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            std::cout << "Uzyj -h aby wyswietlic pomoc\n";
            return 0;
        }

        std::string flag = argv[1];

        if ((flag == "-h" || flag == "--help") && argc == 2) {
            print_help();
        }
        else if ((flag == "-i" || flag == "-info") && argc == 3) {
            std::string file_path = argv[2];
            if (!is_file_supported(file_path)) {
                std::cout << "Blad: Nieobslugiwany format pliku\n";
                return 1;
            }
            try {
                FileInfo info = get_file_info(file_path);
                std::cout << "Format: " << get_file_extension(file_path) << "\n"
                          << "Rozmiar: " << info.size << " bajtow\n"
                          << "Ostatnia modyfikacja: " << std::asctime(std::localtime(&info.last_modified));
            } catch (...) {
                std::cout << "Blad: Brak uprawnień do pliku\n";
                return 1;
            }
        }
        else if ((flag == "-c" || flag == "--check")&& argc == 4) {
            std::string file_path = argv[2];
            std::string message = argv[3];
            if (check_file_size(file_path, message)) {
                std::cout << "Plik ma wystarczajaca pojemnosc\n";
            }
            else {
                std::cout << "Plik jest za maly\n";
            }
        }
        else if ((flag == "-e" || flag == "--encrypt") && argc == 4) {
            std::string file_path = argv[2];
            std::string message = argv[3];
            if (get_file_extension(file_path) == "bmp") {
                encrypt_message_in_bmp(file_path, message);
            }
            else if (get_file_extension(file_path) == "png") {
                encrypt_message_in_png(file_path, message);
            }
            else if (get_file_extension(file_path) == "ppm") {
                encrypt_message_in_ppm(file_path, message);
            }
            else {
                std::cout << "Nieobslugiwany format pliku\n";
            }
        }
        else if ((flag == "-d" || flag=="--decrypt") && argc == 3) {
            std::string file_path = argv[2];
            if (get_file_extension(file_path) == "bmp") {
                std::string message = decrypt_message_from_bmp(file_path);
                std::cout << "Odkodowana wiadomosc: " << message << "\n";
            }
            else if (get_file_extension(file_path) == "png") {
                std::string message = decrypt_message_from_png(file_path);
                std::cout << "Odkodowana wiadomosc: " << message << "\n";
            }
            else if (get_file_extension(file_path) == "ppm") {
                std::string message = decrypt_message_from_ppm(file_path);
                std::cout << "Odkodowana wiadomosc: " << message << "\n";
            }
            else {
                //std::cout << "Nieobslugiwany format pliku\n";
                throw std::runtime_error("Nieobslugiwany format pliku");
            }
        }
        else {
            std::cout << "Blad: nieznana flaga, Uzyj -h\n";
            return 1;
        }
    }catch (const std::exception& e) {
        std::cout << "Blad: " <<  e.what() << "\nUżyj -h aby wyswietlic pomoc\n";
        return 1;
    }
    return 0;
}