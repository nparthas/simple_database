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

typedef enum MetaCommandResult_t {
    kMetaCommandSuccess,
    KMetaCommandUnrecognized,
} MetaCommandResult;

typedef enum PrepareResult_t {
    kPrepareSuccess,
    kPrepareSyntaxError,
    kPrepareUnrecognizedStatement,
} PrepareResult;

typedef enum StatementType_t {
    kStatementSelect,
    kStatementInsert,
} StatementType;

typedef enum ExecuteResult_t {
    kExecuteSuccess,
    kExecuteTableFull,
    kExecuteNotImplemented,
} ExecuteResult;

typedef struct Row_t {
    uint32_t Id;
    char Username[sizes::kColUsername];
    char Email[sizes::kColEmail];
} Row;

typedef struct Statement_t {
    StatementType type;
    Row insert_row;
} Statement;

typedef struct Table_t {
    void *pages[sizes::kTableMaxPages];
    uint32_t num_rows;
} Table;
}  // namespace simpledb