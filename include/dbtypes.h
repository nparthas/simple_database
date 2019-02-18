typedef enum MetaCommandResult_t {
    kMetaCommandSuccess,
    KMetaCommandUnrecognized
} MetaCommandResult;

typedef enum PrepareResult_t {
    kPrepareSuccess,
    kPrepareUnrecognizedStatement
} PrepareResult;

typedef enum StatementType_t {
    kStatementSelect,
    kStatementInsert
} StatementType;

typedef struct Statement_t {
    StatementType type;
} Statement;