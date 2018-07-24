#ifndef SIMPLE_DATABASE_STATEMENTS_H
#define SIMPLE_DATABASE_STATEMENTS_H

#include <cstdint>
#include <string>
#include <sstream>


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

enum ExecuteResult {
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

//typedef struct CompactRow_t {
//    char data[field_sizes::kRowSize];
//} CompactRow;

typedef struct Statement_t {
    StatementType type;
    Row row_to_insert; // only for insert statement
} Statement;

//typedef struct Page_t {
//    char *data[field_sizes::kPageSize];
//} Page;

typedef struct Table_t {
    void *pages[field_sizes::kTableMaxPages];
    int num_rows;
} Table;


MetaCommandResult do_meta_command(std::string const &buf);

PrepareResult prepare_statement(std::string const &buf, Statement *statement);

bool assign_insert_statement_args(std::string const &buf, Statement *statement);




#endif //SIMPLE_DATABASE_STATEMENTS_H
