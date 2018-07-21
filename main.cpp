#include <iostream>
#include <string>
#include <limits>
#include <sstream>
#include <vector>
#include <iterator>

enum MetaCommandResult {
    kMetaCommandSuccess,
    kMetaCommandUnrecognizedCommand
};

enum PrepareResult {
    kPrepareSuccess,
    kPrepareSyntaxError,
    kPrepareUnrecognizedStatement
};

enum StatementType {
    kStatementInsert,
    kStatementSelect,
};

namespace field_sizes { // byte size of fields and offsets for packing
    const int kColumnIdSize = 4;
    const int kColumnUsernameSize = 32;
    const int kColumnEmailSize = 255;
    const int kIdOffset = 0;
    const int kUsernameOffset = kIdOffset + kColumnIdSize;
    const int kEmailOffset = kUsernameOffset + kColumnUsernameSize;
    const int kRowSize = kColumnIdSize + kColumnUsernameSize + kColumnEmailSize;
}

typedef struct Row_t {
    int32_t id;
    std::string username;  // need to validate these sizes in input to make sure that they are at max 255 and 32 bits 
    std::string email;

    Row_t() : id(), username(), email() {}
//        username.reserve(field_sizes::kColumnUsernameSize); // reserving max string sizes
//        email.reserve(field_sizes::kColumnEmailSize);
//    }
} Row;

typedef struct Statement_t {
    StatementType type;
    Row row_to_insert; // only for insert statement
} Statement;


typedef struct CompactRow_t {
//    unsigned char *row;
    unsigned char row[field_sizes::kRowSize];
} CompactRow;


void print_prompt();

std::string read_input();

MetaCommandResult do_meta_command(std::string const &buf);

PrepareResult prepare_statement(std::string const &buf, Statement *statement);

void execute_statement(Statement *statement);

bool assign_insert_statement_args(std::string const &buf, Statement *statement);

void serialize_row(Row* source, CompactRow *destination);

void deserialize_row(CompactRow *source, CompactRow *destination);

int main(int argc, char *argv[]) {

    auto c = new char;
    auto row = new CompactRow();

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
        bool success = assign_insert_statement_args(buf, statement);

//        std::cout << statement->row_to_insert.id << statement->row_to_insert.username << statement->row_to_insert.email << std::endl;
//        std::cout << success << std::endl;

        return (success) ? kPrepareSuccess : kPrepareSyntaxError;
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

bool assign_insert_statement_args(std::string const &buf, Statement *statement) {
    std::istringstream iss(buf);

    iss.ignore(6); // ignore insert
    iss >> statement->row_to_insert.id >> statement->row_to_insert.username >> statement->row_to_insert.email;

//    std::cout << iss.str() << std::endl;
//    std::cout << statement->row_to_insert.id << std::endl;
//    std::cout << statement->row_to_insert.username << std::endl;
//    std::cout << statement->row_to_insert.email << std::endl;
    return iss.good();
}
