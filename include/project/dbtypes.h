#pragma once

#include <fstream>
#include <iostream>

namespace simpledb {

namespace sizes {
const size_t kColUsername = 32;
const size_t kColEmail = 255;

const size_t kIdSize = sizeof(uint32_t);  // id is backed but a uint32_t
const size_t kUsernameSize = kColUsername * sizeof(char);
const size_t kEmailSize = kColEmail * sizeof(char);
const size_t kRowsize = kIdSize + kUsernameSize + kEmailSize;

const size_t kIdOffset = 0;
const size_t kUsernameOffset = kIdOffset + kIdSize;
const size_t kEmailOffset = kUsernameOffset + kUsernameSize;

const size_t kPageSize = 4096;
const size_t kTableMaxPages = 100;
const size_t kRowsPerPage = kPageSize / kRowsize;
const size_t kTableMaxRows = kRowsPerPage * kTableMaxPages;
}  // namespace sizes

enum MetaCommandResult {
    kMetaCommandSuccess,
    KMetaCommandUnrecognized,
};

enum PrepareResult {
    kPrepareSuccess,
    kPrepareSyntaxError,
    kPrepareFieldTooLong,
    kPrepareNegativeId,
    kPrepareUnrecognizedStatement,
};

enum StatementType {
    kStatementSelect,
    kStatementInsert,
};

enum ExecuteResult {
    kExecuteSuccess,
    kExecuteTableFull,
    kExecuteNotImplemented,
};

struct Row {
    int32_t Id;
    char Username[sizes::kColUsername + 1];
    char Email[sizes::kColEmail + 1];
};

struct Statement {
    StatementType type;
    Row insert_row;
};

class Pager {
   public:
    explicit Pager(std::string const &filename);

    ~Pager();

    Pager(Pager &&pager) noexcept;

    Pager &operator=(Pager &&bigint) noexcept;

    void *GetPage(uint32_t pagenum);

    void FlushPages(uint32_t num_rows);

    void FlushPage(uint32_t pagenum, uint32_t size);

    bool Close();

    uint32_t file_length() { return this->file_length_; }

   private:
    std::string filename_;
    std::fstream file_;
    uint32_t file_length_;
    void *pages_[sizes::kTableMaxPages];

    void Dump(int pagenum);
};

class Table {
   public:
    uint32_t num_rows_;  // should be private

    Table(std::string const &filename);

    ~Table();

    void *GetPage(uint32_t pagenum) { return this->pager_->GetPage(pagenum); }

    uint32_t num_rows() { return this->num_rows_; }

   private:
    Pager *pager_;
};
}  // namespace simpledb
