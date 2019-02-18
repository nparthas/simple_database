#include <algorithm>
#include <iostream>
#include <string>
#include "../include/dbtypes.h"

void print_prompt() { std::cout << "db > "; }

std::string read_input(std::string &buf) {
    std::getline(std::cin, buf);

    if (std::cin.bad() || std::cin.fail()) {
        std::cout << "error reading input";
        exit(EXIT_FAILURE);
    }
    return buf;
}

MetaCommandResult do_meta_command(std::string const &buf) {
    if (buf == ".exit") {
        exit(EXIT_SUCCESS);
    } else {
        return KMetaCommandUnrecognized;
    }
}

PrepareResult prepare_statement(std::string const &buf, Statement &statement) {
    if (buf.compare(0, 6, "insert") == 0) {
        statement.type = kStatementInsert;
        return kPrepareSuccess;
    }
    if (buf == "select") {
        statement.type = kStatementSelect;
        return kPrepareSuccess;
    }

    return kPrepareUnrecognizedStatement;
}

void execute_statement(Statement const &statement) {
    switch (statement.type) {
        case kStatementSelect:
            std::cout << "select stub" << std::endl;
            break;
        case kStatementInsert:
            std::cout << "insert stub" << std::endl;
            break;
    }
}

int main(int argc, char *argv[]) {
    std::string buf;
    while (true) {
        print_prompt();
        read_input(buf);

        if (std::all_of(buf.begin(), buf.end(), iswspace)) {
            continue;
        }

        if (buf[0] == '.') {
            switch (do_meta_command(buf)) {
                case (kMetaCommandSuccess):
                    continue;
                case (KMetaCommandUnrecognized):
                    std::cout << "Unrecognized meta command: " << buf
                              << std::endl;
                    continue;
            }
        }

        Statement statement;
        switch (prepare_statement(buf, statement)) {
            case (kPrepareSuccess):
                break;
            case (kPrepareUnrecognizedStatement):
                std::cout << "Unrecognized command at the start of " << buf
                          << std::endl;
                continue;
        }

        execute_statement(statement);
        std::cout << "Executed" << std::endl;
    }
    return 0;
}