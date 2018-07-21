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

enum ExecuteResult_t {
    kExecuteSuccess,
    kExecuteTableFull,
};

namespace field_sizes { // byte size of fields and offsets for packing
    const int kColumnIdSize = 4;
    const int kColumnUsernameSize = 32;
    const int kColumnEmailSize = 255;
    const int kIdOffset = 0;
    const int kUsernameOffset = kIdOffset + kColumnIdSize;
    const int kEmailOffset = kUsernameOffset + kColumnUsernameSize;
    const int kRowSize = kColumnIdSize + kColumnUsernameSize + kColumnEmailSize;
    const int kRowSize = kColumnIdSize + kColumnUsernameSize + kColumnEmailSize;
    const int kPageSize = 4096;
    const int kTableMaxPages = 100;
    const int kRowsPerPage = kPageSize / kRowSize;
    const int kTableMaxRows = kRowsPerPage * kTableMaxPages;
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

typedef struct CompactRow_t {
    char data[field_sizes::kRowSize];
} CompactRow

typedef struct Statement_t {
    StatementType type;
    Row row_to_insert; // only for insert statement
} Statement;

typedef struct Table_t {
    Page *pages[field_sizes::kTableMaxPages]; 
    int num_rows;
} Table;

typedef struct Page_t {
    char *data[field_sizes::kPageSize];
} Page

void print_prompt();

std::string read_input();

MetaCommandResult do_meta_command(std::string const &buf);

PrepareResult prepare_statement(std::string const &buf, Statement *statement);

void execute_statement(Statement *statement);

bool assign_insert_statement_args(std::string const &buf, Statement *statement);

void serialize_row(Row* source, void *destination);

void deserialize_row(void *source, Row *destination);

char* row_slot(Table *table, int row_num);

int main(int argc, char *argv[]) {
    Table * table = new table;

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

        switch (execute_statement(&statement)) {
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

ExecuteResult execute_insert(Statement *statement, Table *table) {
    if (table->num_rows >= field:sizes::kTableMaxRows) {
        return kExecuteTableFull;
    }

    Row *row_to_insert = &(statement->row_to_insert);

    serialize_row(row_to_insert, row_slot(table, table->num_rows));
    table->num_rows++;

    return kExecuteSuccess;

ExecuteResult execute_select(Statement *statement, Table *table) {
    Row row;
    for(int i = 0; i < table->num_rows; i++) {
        deserialize_row(row_slot(table, i), &row);
        print_row(&row);
    }
    return kExecuteSuccess;
} 

ExecuteResult execute_statement(Statement *statement, Table *table) {
    switch (statement->type) {
        case (kStatementInsert):
            return execute_insert(statemnt, table);
        case (kStatementSelect):
            return execute_select(statement, table);
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

void serialize_row(Row* source, void *destination) {
    std::memcpy(destination + field_sizes::kIdOffset, &(source->id), field_sizes::kColumnIdSize);
    std::memcpy(destination + field_size::kUsernameOffset, &(source->username.c_str()), field_sizes::kColumnUsernameSize);
    std::memcpy(destination + field_sizes::kEmailOffset, &(source->email.c_str()), field_sizes::kColumnEmailSize);
}

void deserialize_row(CompactRow *source, void *destination) {
    std::memcpy(&(destination->id), source + field_sizes::kIdOffset, field_sizes::kColumnIdSize);
    std::memcpy(&(destination->username), source + field_sizes::kUsernameOffset, field_sizes::kColumnUsernameSize);
    std::memcpy(&(destination->email), source + field_sizes::kEmailOffset, field_sizes::kColumnEmailSize);
}

char* row_slot(Table *table, int row_num) {
    int page_num = row_num / field_sizes::kRowsPerPage;
    char *page = table->pages[page_num];
    if(!page) {
        page = new Page;
        table->pages[page_num] = page;
    }
    int row_offset = row_num % field_size::kRowsPerPage;
    int byte_offset = row_offset * field_sizes::kRowSize;
    return page[byte_offset];
}



