#include "../include/project/parser.h"


void print_prompt() {
    std::cout << "db > ";
}

std::string read_input() {
    std::string buf;
    std::getline(std::cin, buf);

    if (buf.length() <= 0) {
        std::cout << "error reading input";
        exit(EXIT_FAILURE);
    }
    return buf;
}

