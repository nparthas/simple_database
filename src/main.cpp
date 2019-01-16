#include <iostream>
#include <string>

void print_prompt() { std::cout << "db > "; }

std::string read_input(std::string &buf) {
    std::getline(std::cin, buf);

    if (std::cin.bad() || std::cin.fail()) {
        std::cout << "error reading input";
        exit(EXIT_FAILURE);
    }
    return buf;
}

int main(int argc, char *argv[]) {
    std::string buf;
    while (true) {
        print_prompt();
        read_input(buf);

        if (buf.compare(".exit") == 0) {
            exit(EXIT_SUCCESS);
        } else {
            std::cout << "Unrecognized command: " << buf << std::endl;
        }
    }

    return 0;
}
