#include "../include/project/store.h"


ExecuteResult execute_insert(Statement *statement, Table *table) {
    if (table->num_rows >= field_sizes::kTableMaxRows) {
        return kExecuteTableFull;
    }

    Row *row_to_insert = &(statement->row_to_insert);

    serialize_row(row_to_insert, row_slot(table, table->num_rows));
    table->num_rows++;

    return kExecuteSuccess;
}

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
            return execute_insert(statement, table);
        case (kStatementSelect):
            return execute_select(statement, table);
    }
}


void serialize_row(Row* source, void *destination) {
    const char *username = source->username.c_str();
    const char *email = source->email.c_str();
    std::memcpy(destination + field_sizes::kIdOffset, &(source->id), field_sizes::kColumnIdSize);
    std::memcpy(destination + field_sizes::kUsernameOffset, &(username), field_sizes::kColumnUsernameSize);
    std::memcpy(destination + field_sizes::kEmailOffset, &(email), field_sizes::kColumnEmailSize);
}

void deserialize_row(void *source, Row *destination) {
    char username[field_sizes::kColumnUsernameSize] = {};
    char email[field_sizes::kColumnEmailSize] = {};

    std::memcpy(&(destination->id), source + field_sizes::kIdOffset, field_sizes::kColumnIdSize);
    std::memcpy(&username, source + field_sizes::kUsernameOffset, field_sizes::kColumnUsernameSize);
    std::memcpy(&email, source + field_sizes::kEmailOffset, field_sizes::kColumnEmailSize);

    destination->username = std::string(username);
    destination->email = std::string(email);
}

void* row_slot(Table *table, int row_num) {
    int page_num = row_num / field_sizes::kRowsPerPage;
    void *page = table->pages[page_num];
    if(!page) {
        page = malloc(field_sizes::kPageSize);
        table->pages[page_num] = page;
    }
    int row_offset = row_num % field_sizes::kRowsPerPage;
    int byte_offset = row_offset * field_sizes::kRowSize;
    return page + byte_offset;
}


void print_row(Row *row) {
    std::cout << row->id << " " << row->username << " " << row->email << std::endl;
}


