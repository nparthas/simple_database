#include "../include/project/statements.h"




bool assign_insert_statement_args(std::string const &buf, Statement *statement) {
    std::istringstream iss(buf);

    iss.ignore(6); // ignore insert
    iss >> statement->row_to_insert.id >> statement->row_to_insert.username >> statement->row_to_insert.email;

//    std::cout << iss.str() << std::endl;
//    std::cout << statement->row_to_insert.id << std::endl;
//    std::cout << statement->row_to_insert.username << std::endl;
//    std::cout << statement->row_to_insert.email << std::endl;
    return !iss.bad();
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
    if (buf.compare(0, 6, "select") == 0) {
        statement->type = kStatementSelect;
        return kPrepareSuccess;
    }

    return kPrepareUnrecognizedStatement;
}


