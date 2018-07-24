#include <iostream>
#include <string>
#include <limits>
#include <sstream>
#include <vector>
#include <iterator>
#include <cstring>

#include "../include/project/statements.h"
#include "../include/project/parser.h"
#include "../include/project/store.h"


int main(int argc, char *argv[]) {
    Table *table = new Table; // may be the cause

    while (true) {
        print_prompt();
        std::string input_buf = read_input();

        if (input_buf[0] == '.') {
            switch (do_meta_command(input_buf)) {
                case (kMetaCommandSuccess):
                    continue;
                case (kMetaCommandUnrecognizedCommand):
                    std::cout << "unrecognized meta command: " << input_buf << std::endl;
                    continue;
            }
        }

        Statement statement;
        switch (prepare_statement(input_buf, &statement)) {
            case (kPrepareSuccess):
                break;
            case (kPrepareSyntaxError):
                std::cout << "syntax error in: " << input_buf << std::endl;
                continue;
            case (kPrepareUnrecognizedStatement):
                std::cout << "unrecognized keyword at the start of: " << input_buf << std::endl;
                continue;
        }

        switch (execute_statement(&statement, table)) {
            case (kExecuteSuccess):
                std::cout << "Executed" << std::endl;
                break;
            case (kExecuteTableFull):
                std::cout << "Error: table full" << std::endl;
                break;
        }

        if (input_buf.empty()) break; // remove infinite loop warning
    }

    return 0;
}


