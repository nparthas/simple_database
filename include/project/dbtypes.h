#pragma once

#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>

namespace simpledb {
namespace sizes {
constexpr size_t kColUsername = 33;
constexpr size_t kColEmail = 256;

constexpr size_t kIdSize = sizeof(uint32_t);  // id is backed but a uint32_t
constexpr size_t kUsernameSize = kColUsername * sizeof(char);
constexpr size_t kEmailSize = kColEmail * sizeof(char);
constexpr size_t kRowSize = kIdSize + kUsernameSize + kEmailSize;

constexpr size_t kIdOffset = 0;
constexpr size_t kUsernameOffset = kIdOffset + kIdSize;
constexpr size_t kEmailOffset = kUsernameOffset + kUsernameSize;

constexpr size_t kPageSize = 4096;
constexpr size_t kTableMaxPages = 100;

// Common node header layout
constexpr size_t kNodeTypeSize = sizeof(uint8_t);
constexpr size_t KNodeTypeOffset = 0;
constexpr size_t kIsRootSize = sizeof(uint8_t);
constexpr size_t kIsRootOffset = kNodeTypeSize;
constexpr size_t kParentPointerSize = sizeof(uint32_t);  // check this
constexpr size_t kParentPointerOffset = kIsRootSize + kIsRootOffset;
constexpr size_t
    kCommonNodeHeaderSize =  // check that this doesn't need to be a uint8_t
    kNodeTypeSize + kIsRootSize + kParentPointerSize;

// Leaf node header layout
constexpr size_t kLeafNodeNumCellsSize = sizeof(uint32_t);
constexpr size_t kLeafNodeNumCellsOffset = kCommonNodeHeaderSize;
constexpr size_t kLeafNodeHeaderSize =
    kCommonNodeHeaderSize + kLeafNodeNumCellsSize;

// Leaf node body layout
constexpr size_t kLeafNodeKeySize = sizeof(uint32_t);
constexpr size_t kLeafNodeKeyOffset = 0;
constexpr size_t kLeafNodeValueSize = kRowSize;
constexpr size_t kLeafNodeValueOffset = kLeafNodeKeyOffset + kLeafNodeKeySize;
constexpr size_t kLeafNodeCellSize = kLeafNodeKeySize + kLeafNodeValueSize;
constexpr size_t kLeafNodeSpaceForCells = kPageSize - kLeafNodeHeaderSize;
constexpr size_t kLeafNodeMaxCells = kLeafNodeSpaceForCells / kLeafNodeCellSize;

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

enum NodeType {
    kNodeInternal,
    kNodeLeaf,
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

    void FlushPages();

    void FlushPage(uint32_t pagenum);

    bool Close();

    inline uint32_t file_length() const { return this->file_length_; }

    inline uint32_t num_pages() const { return this->num_pages_; }

   private:
    std::string filename_;
    std::fstream file_;
    uint32_t file_length_;
    uint32_t num_pages_;
    void *pages_[sizes::kTableMaxPages];

    void Dump(int pagenum);
};

class Table {
   public:
    Table(std::string const &filename);

    ~Table();

    inline uint32_t root_page_num() const { return this->root_page_num_; }

    void *GetPage(uint32_t pagenum) { return this->pager_->GetPage(pagenum); }

   private:
    Pager *pager_;
    uint32_t root_page_num_;  // should be private
};

class Cursor {
   public:
    Table *table_;
    uint32_t pagenum_;
    uint32_t cellnum_;
    bool end_of_table_;  // at position one past last element

    Cursor(Table *table, bool start);

    void *Value();

    void Advance();

    ~Cursor();

    inline bool end_of_table() const { return this->end_of_table_; }
};

class LeafNode {
   public:
    LeafNode(void *data) { this->data_ = data; }

    ~LeafNode() {}  // does not deallocate the data

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-arith"
    uint32_t *NumCells() {
        return (uint32_t *)this->data_ + sizes::kLeafNodeNumCellsOffset;
    }
#pragma GCC diagnostic pop

    uint32_t *Key(uint32_t cell_num) {
        return (uint32_t *)this->Cell(cell_num);
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-arith"
    void *Value(uint32_t cell_num) {
        // return (uint32_t *)this->Cell(cell_num) + sizes::kLeafNodeKeySize +
        // 1;
        return (uint32_t *)this->Cell(cell_num) + sizes::kLeafNodeKeySize + 1;
    }
#pragma GCC diagnostic pop

    void Insert(Cursor const &cursor, uint32_t key, Row value);

    void Initialize() { *this->NumCells() = 0; }

   private:
    void *data_;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-arith"
    void *Cell(uint32_t cell_num) {
        return this->data_ + sizes::kLeafNodeHeaderSize +
               cell_num * sizes::kLeafNodeCellSize;
    }
#pragma GCC diagnostic pop

    inline void SerializeRow(void *dest, const Row &source);

    inline void DeserializeRow(Row &dest, const void *source);
};

}  // namespace simpledb
