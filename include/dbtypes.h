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

const size_t kPagesize = 4096;
const size_t kTableMaxPages = 100;
const size_t kRowsPerPage = kPagesize / kRowsize;
const size_t kTableMaxRows = kRowsPerPage * kTableMaxPages;
}  // namespace sizes

enum MetaCommandResult {
    kMetaCommandSuccess,
    KMetaCommandUnrecognized,
};

enum PrepareResult {
    kPrepareSuccess,
    kPrepareSyntaxError,
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
    uint32_t Id;
    char Username[sizes::kColUsername];
    char Email[sizes::kColEmail];
};

struct Statement {
    StatementType type;
    Row insert_row;
};

struct Table {
    void *pages[sizes::kTableMaxPages] = {
        NULL,
    };
    uint32_t num_rows;
};
}  // namespace simpledb