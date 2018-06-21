#include <iostream>
#include <string>
#include <limits>

enum MetaCommandResult {
    kMetaCommandSuccess,
    kMetaCommandUnrecognizedCommand
};

enum PrepareResult {
    kPrepareSuccess,
    kPrepareUnrecognizedStatement
};

enum StatementType {
    kStatementInsert,
    kStatementSelect,
};

typedef struct Statement_t {
    StatementType type;
} Statement;


void print_prompt();

std::string read_input();

MetaCommandResult do_meta_command(std::string const &buf);

PrepareResult prepare_statement(std::string const &buf, Statement *statement);

void execute_statement(Statement *statement);

int main(int argc, char *argv[]) {

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
            case (kPrepareUnrecognizedStatement):
                std::cout << "unrecognized keyword at the start of: " << input_buf << std::endl;
                continue;
        }

        execute_statement(&statement);
        std::cout << "executed " << std::endl;

        if (input_buf.empty()) break; // remove infinite loop warning
    }

    return 0;
}

void print_prompt() {
    std::cout << "db > ";
}

std::string read_input() {
    //std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string buf;
    std::getline(std::cin, buf);

    if (buf.length() <= 0) {
        std::cout << "error reading input";
        exit(EXIT_FAILURE);
    }
    return buf;
}

MetaCommandResult do_meta_command(std::string const &buf) {
    if (buf == ".exit") {
        exit(EXIT_SUCCESS);
    } else {
        return kMetaCommandUnrecognizedCommand;
    }
}

PrepareResult prepare_statement(std::string const &buf, Statement *statement) {
    if (buf.compare(0, 6, "insert") == 0) {
        statement->type = kStatementInsert;
        return kPrepareSuccess;
    }
    if (buf == "select") {
        statement->type = kStatementSelect;
        return kPrepareSuccess;
    }

    return kPrepareUnrecognizedStatement;
}

void execute_statement(Statement *statement) {
    switch (statement->type) {
        case (kStatementInsert):
            std::cout << "insert stub" << std::endl;
            break;
        case (kStatementSelect):
            std::cout << "select stub" << std::endl;
            break;
    }
}