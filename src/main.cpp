#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "dbtypes.h"

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

void print_constants() {
    std::cout << "Row Size: " << sizes::kRowSize << std::endl;
    std::cout << "Common Node Header size: " << sizes::kCommonNodeHeaderSize
              << std::endl;
    std::cout << "Leaf Node Header Size: " << sizes::kLeafNodeHeaderSize
              << std::endl;
    std::cout << "Leaf Node Cell Size: " << sizes::kLeafNodeCellSize
              << std::endl;
    std::cout << "Leaf Node Space For Cells: " << sizes::kLeafNodeSpaceForCells
              << std::endl;
    std::cout << "Leaf Node Max Cell: " << sizes::kLeafNodeMaxCells
              << std::endl;
}

void print_leaf_node(LeafNode node) {
    uint32_t num_cells = *node.NumCells();
    std::cout << "  Leaf size: " << *node.NumCells() << std::endl;
    for (uint32_t i = 0; i < num_cells; i++) {
        std::cout << "    " << i << " : " << *node.Key(i) << std::endl;
    }
}

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

void db_close(Table &table);

MetaCommandResult do_meta_command(std::string const &buf, Table &table) {
    if (buf == ".exit") {
        db_close(table);
        // TODO:: exit from main
        exit(EXIT_SUCCESS);
    } else if (buf == ".constants") {
        std::cout << "Constants: " << std::endl;
        print_constants();
        return kMetaCommandSuccess;
    } else if (buf == ".btree") {
        std::cout << "Tree:" << std::endl;
        print_leaf_node(LeafNode(table.GetPage(0)));
        return kMetaCommandSuccess;
    } else {
        return KMetaCommandUnrecognized;
    }
}  // namespace simpledb

PrepareResult assign_insert_statement_args(std::string const &buf,
                                           Statement &statement) {
    std::istringstream iss(buf);
    std::string token;

    iss.ignore(6);  // ignore insert
    iss >> statement.insert_row.Id;
    if (iss.fail()) return kPrepareSyntaxError;
    if (statement.insert_row.Id < 0) return kPrepareNegativeId;

    // parse username 32 characters
    iss >> token;
    if (iss.fail()) return kPrepareSyntaxError;
    if (token.length() > sizes::kUsernameSize) return kPrepareFieldTooLong;
    std::strcpy(statement.insert_row.Username, token.c_str());

    iss >> token;
    if (iss.fail()) return kPrepareSyntaxError;
    if (token.length() > sizes::kEmailSize) return kPrepareSyntaxError;
    std::strcpy(statement.insert_row.Email, token.c_str());

    return (iss.eof()) ? kPrepareSuccess : kPrepareSyntaxError;
}

PrepareResult prepare_statement(std::string const &buf, Statement &statement) {
    if (buf.compare(0, 6, "insert") == 0) {
        statement.type = kStatementInsert;
        return assign_insert_statement_args(buf, statement);
    }
    if (buf == "select") {
        statement.type = kStatementSelect;
        return kPrepareSuccess;
    }

    return kPrepareUnrecognizedStatement;
}

ExecuteResult execute_insert(Statement const &statement, Table &table) {
    LeafNode node = LeafNode(table.GetPage(table.root_page_num()));

    if (*node.NumCells() >= sizes::kLeafNodeMaxCells) {
        return kExecuteTableFull;
    }

    uint32_t key_id = statement.insert_row.Id;
    Cursor cursor = Cursor(&table, key_id);

    if (cursor.cellnum_ < *node.NumCells()) {
        if (key_id == *node.Key(cursor.cellnum_)) {
            return kExecuteDuplicateKey;
        }
    }

    node.Insert(cursor, statement.insert_row.Id, statement.insert_row);

    return kExecuteSuccess;
}

inline void print_row(Row const &row) {
    std::cout << "[" << row.Id << ", " << row.Username << ", " << row.Email
              << "]" << std::endl;
}

// TODO:: remove
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-arith"
inline void deserialize_row(Row &dest, const void *source) {
    std::memcpy(&dest.Id, source + sizes::kIdOffset, sizes::kIdSize);
    std::memcpy(&dest.Username, source + sizes::kUsernameOffset,
                sizes::kUsernameSize);
    std::memcpy(&dest.Email, source + sizes::kEmailOffset, sizes::kEmailSize);

    // we do not copy the null terminator, ensure it's always there if the field
    // is full
    dest.Username[sizes::kUsernameSize] = '\0';
    dest.Email[sizes::kEmailSize] = '\0';
}
#pragma GCC diagnostic pop

ExecuteResult execute_select(__attribute__((unused)) Statement const &statement,
                             Table &table) {
    Cursor cursor = Cursor(&table, true);
    Row row;

    while (!cursor.end_of_table()) {
        deserialize_row(row, cursor.Value());
        print_row(row);
        cursor.Advance();
    }

    return kExecuteSuccess;
}  // namespace simpledb

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

inline Table db_open(std::string const filename) { return Table(filename); }

void db_close(Table &table) { table.~Table(); }

}  // namespace simpledb

using namespace simpledb;
int main(void) {
    std::string buf;
    // TODO:: get file name from cli
    Table table = db_open("dbfile");

    // int i = 0;
    while (true) {
        print_prompt();
        read_input(buf);
        // if (i++ < 1401) {
        //     buf = "insert 1 a a";
        // } else {
        //     buf = ".exit";
        // }

        if (buf.empty()) continue;

        if (buf[0] == '.') {
            switch (do_meta_command(buf, table)) {
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
            case (kPrepareFieldTooLong):
                std::cout << "Field is too long" << std::endl;
                continue;
            case (kPrepareNegativeId):
                std::cout << "Id cannot be negative" << std::endl;
                continue;
            case (kPrepareUnrecognizedStatement):
                std::cout << "Unrecognized command at the start of " << buf
                          << std::endl;
                continue;
        }

        switch (execute_statement(statement, table)) {
            case (kExecuteSuccess):
                std::cout << "Executed" << std::endl;
                break;
            case (kExecuteTableFull):
                std::cout << "Error: table full" << std::endl;
                break;
            case (kExecuteDuplicateKey):
                std::cout << "Error: duplicate key" << std::endl;
                break;
            case (kExecuteNotImplemented):
                std::cout << "Error: operation not implemented" << std::endl;
                break;
        }
    }

    return 0;
}