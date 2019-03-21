#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include "../include/dbtypes.h"

namespace {

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    [](int ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](int ch) { return !std::isspace(ch); })
                .base(),
            s.end());
}

// trim from both ends (in place)
static inline std::string trim(std::string &s) {
    ltrim(s);
    rtrim(s);
    return s;
}
}  // namespace

namespace simpledb {
void print_prompt() { std::cout << "db > "; }

std::string read_input(std::string &buf) {
    std::getline(std::cin, buf);

    // ensure the badbit is not set or we have the failbit set witout the eof
    // bit
    if (std::cin.bad() || (std::cin.fail() && !std::cin.eof())) {
        std::cout << "error reading input";
        exit(EXIT_FAILURE);
    }

    // we failbit was set by empty newline, reset the flag, we are good
    if (std::cin.fail()) {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.clear();
    }

    trim(buf);
    return buf;
}

MetaCommandResult do_meta_command(std::string const &buf) {
    if (buf == ".exit") {
        exit(EXIT_SUCCESS);
    } else {
        return KMetaCommandUnrecognized;
    }
}

bool assign_insert_statement_args(std::string const &buf,
                                  Statement &statement) {
    std::istringstream iss(buf);

    iss.ignore(6);  // ignore insert
    iss >> statement.insert_row.Id >> statement.insert_row.Username >>
        statement.insert_row.Email;

    return (iss.eof() & !(iss.bad() || iss.fail()));
}

PrepareResult prepare_statement(std::string const &buf, Statement &statement) {
    if (buf.compare(0, 6, "insert") == 0) {
        statement.type = kStatementInsert;
        bool assigned = assign_insert_statement_args(buf, statement);
        return (assigned) ? kPrepareSuccess : kPrepareSyntaxError;
    }
    if (buf == "select") {
        statement.type = kStatementSelect;
        return kPrepareSuccess;
    }

    return kPrepareUnrecognizedStatement;
}

void serialize_row(void *dest, const Row &source) {
    std::memcpy(dest + sizes::kIdOffset, &source.Id, sizes::kIdSize);
    std::memcpy(dest + sizes::kUsernameOffset, &source.Username,
                sizes::kUsernameSize);
    std::memcpy(dest + sizes::kEmailOffset, &source.Email, sizes::kEmailOffset);
}

void deserialize_row(Row &dest, const void *source) {
    std::memcpy(&dest.Id, source + sizes::kIdOffset, sizes::kIdSize);
    std::memcpy(&dest.Username, source + sizes::kUsernameOffset,
                sizes::kUsernameSize);
    std::memcpy(&dest.Email, source + sizes::kEmailOffset, sizes::kEmailSize);
}

void *row_slot(Table &table, uint32_t row_num) {
    uint32_t page_num = row_num / sizes::kRowsPerPage;
    void *page = table.pages[page_num];
    if (!page) {
        table.pages[page_num] = malloc(sizes::kPagesize);
        page = table.pages[page_num];
    }

    uint32_t row_offset = row_num % sizes::kRowsPerPage;
    uint32_t byte_offset = row_offset * sizes::kRowsize;
    return page + byte_offset;
}

ExecuteResult execute_insert(Statement const &statement, Table &table) {
    if (table.num_rows >= sizes::kTableMaxRows) {
        return kExecuteTableFull;
    }

    serialize_row(row_slot(table, table.num_rows), statement.insert_row);
    table.num_rows++;

    return kExecuteSuccess;
}

void print_row(Row const &row) {
    std::cout << "[" << row.Id << ", " << row.Username << ", " << row.Email
              << "]" << std::endl;
}

ExecuteResult execute_select(Statement const &statement, Table &table) {
    Row row;
    for (uint32_t i = 0; i < table.num_rows; i++) {
        deserialize_row(row, row_slot(table, i));
        print_row(row);
    }
    return kExecuteSuccess;
}

ExecuteResult execute_statement(Statement const &statement, Table &table) {
    switch (statement.type) {
        case kStatementSelect:
            return execute_select(statement, table);
        case kStatementInsert:
            return execute_insert(statement, table);
        default:
            return kExecuteNotImplemented;
    }
}
}  // namespace simpledb

using namespace simpledb;
int main(void) {
    std::string buf;
    Table *table = (Table *)calloc(1, sizeof(Table));
    table->num_rows = 0;

    while (true) {
        print_prompt();
        read_input(buf);

        if (buf.empty()) continue;

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
            case (kPrepareSyntaxError):
                std::cout << "Syntax error. Could not parse statement"
                          << std::endl;
                continue;
            case (kPrepareUnrecognizedStatement):
                std::cout << "Unrecognized command at the start of " << buf
                          << std::endl;
                continue;
        }

        switch (execute_statement(statement, *table)) {
            case (kExecuteSuccess):
                std::cout << "Executed" << std::endl;
                break;
            case (kExecuteTableFull):
                std::cout << "Error: table full" << std::endl;
                break;
            case (kExecuteNotImplemented):
                std::cout << "Error: operation not implemented" << std::endl;
                break;
        }
    }

    return 0;
}