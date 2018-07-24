#ifndef SIMPLE_DATABASE_STORE_H
#define SIMPLE_DATABASE_STORE_H

#include <cstring>
#include <iostream>

#include "statements.h"


void serialize_row(Row* source, void *destination);

void deserialize_row(void *source, Row *destination);

void* row_slot(Table *table, int row_num);

void print_row(Row *row);

ExecuteResult execute_insert(Statement *statement, Table *table);

ExecuteResult execute_select(Statement *statement, Table *table);

ExecuteResult execute_statement(Statement *statement, Table *table);




#endif //SIMPLE_DATABASE_STORE_H
