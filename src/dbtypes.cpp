#include "dbtypes.h"

namespace simpledb {

Pager::Pager(std::string const &filename) {
    this->filename_ = filename;
    this->file_.open(filename, std::ios::in | std::ios::out | std::ios::app |
                                   std::ios::binary);

    if (!this->file_) {
        std::cout << "Unable to create connection";
        exit(EXIT_FAILURE);
    }

    file_.seekg(0, std::ios::beg);
    std::streampos begin = this->file_.tellg();
    file_.seekg(0, std::ios::end);
    std::streampos end = this->file_.tellg();

    this->file_length_ = end - begin;
    this->num_pages_ = this->file_length_ / sizes::kPageSize;

    if (this->file_length_ % sizes::kPageSize != 0) {
        std::cout << "DB file corrupt, must have whole pages only" << std::endl;
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < sizes::kTableMaxPages; i++) {
        this->pages_[i] = nullptr;
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-incomplete"
Pager::~Pager() {
    for (uint32_t i = 0; i < sizes::kTableMaxPages; i++) {
        if (this->pages_[i] != nullptr) {
            delete[] this->pages_[i];
            this->pages_[i] = nullptr;
        }
    }
}
#pragma GCC diagnostic pop

Pager::Pager(Pager &&pager) noexcept {
    if (this == &pager) return;

    this->filename_ = pager.filename_;
    this->file_ = std::fstream(std::move(pager.file_));
    this->file_length_ = pager.file_length_;

    for (size_t i = 0; i < sizes::kTableMaxPages; i++) {
        this->pages_[i] = std::move(pager.pages_[i]);
        pager.pages_[i] = nullptr;
    }

    pager.filename_ = "";
    pager.file_length_ = 0;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-incomplete"
Pager &Pager::operator=(Pager &&pager) noexcept {
    if (this == &pager) return *this;

    this->filename_ = pager.filename_;
    this->file_ = std::fstream(std::move(pager.file_));
    this->file_length_ = pager.file_length_;

    for (size_t i = 0; i < sizes::kTableMaxPages; i++) {
        if (this->pages_[i] != nullptr) {
            delete[] this->pages_[i];
            this->pages_[i] = nullptr;
        }

        this->pages_[i] = std::move(pager.pages_[i]);
        pager.pages_[i] = nullptr;
    }

    pager.filename_ = "";
    pager.file_length_ = 0;

    return *this;
}
#pragma GCC diagnostic pop

void *Pager::GetPage(uint32_t pagenum) {
    if (pagenum > sizes::kTableMaxPages) {
        std::cout << "canont fetch page ( " << pagenum << ") out of bounds ( "
                  << sizes::kTableMaxPages << ")" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (this->pages_[pagenum] == nullptr) {
        void *page = operator new(sizes::kPageSize);

        uint64_t num_pages = this->file_length_ / sizes::kPageSize;
        if (this->file_length_ %
            sizes::kPageSize) {  // make sure that we have enough space if
                                 // we save a partial page
            num_pages++;
        }

        if (pagenum <= num_pages) {
            this->file_.seekg(pagenum * sizes::kPageSize);
            if (this->file_.fail()) {
                std::cout << "unable to seek to page ( " << pagenum << ")"
                          << std::endl;
                exit(EXIT_FAILURE);
            }

            this->file_.read(static_cast<char *>(page), sizes::kPageSize);
            // ensure the badbit is not set or we have the failbit set
            // witout the eof bit
            if (this->file_.bad() ||
                (this->file_.fail() && !this->file_.eof())) {
                std::cout << "unable to read existing page (" << pagenum << ")"
                          << std::endl;
                exit(EXIT_FAILURE);
            }

            if (file_.eof()) {
                file_.clear();
            }
        }

        this->pages_[pagenum] = page;

        if (pagenum >= this->num_pages_) {
            this->num_pages_ = pagenum + 1;
        }
    }

    return this->pages_[pagenum];
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-incomplete"
void Pager::FlushPages() {
    for (uint32_t i = 0; i < this->num_pages_; i++) {
        if (this->pages_[i] == nullptr) continue;
        this->FlushPage(i);
        delete[] this->pages_[i];
        this->pages_[i] = nullptr;
    }
}
#pragma GCC diagnostic pop

void Pager::FlushPage(uint32_t pagenum) {
    if (this->pages_[pagenum] == nullptr) {
        std::cout << "Tried to flush null page" << std::endl;
        exit(EXIT_FAILURE);
    }

    this->file_.seekp(pagenum * sizes::kPageSize);
    if (this->file_.fail()) {
        std::cout << "unable to seek to page ( " << pagenum << ")" << std::endl;
        exit(EXIT_FAILURE);
    }

    this->file_.write(static_cast<char *>(this->pages_[pagenum]),
                      sizes::kPageSize);
}

bool Pager::Close() {
    this->file_.close();
    return !this->file_.fail();
}

void Pager::Dump(int pagenum) {
    for (uint32_t i = 0; i < sizes::kPageSize; i++) {
        std::cout << static_cast<char *>(this->pages_[pagenum])[i] << std::endl;
    }
    std::cout << std::endl;
}

Table::Table(std::string const &filename) {
    this->pager_ = new Pager(filename);

    this->root_page_num_ = 0;

    if (this->pager_->num_pages() == 0) {
        // new db, make page 0 the leaf
        LeafNode(this->pager_->GetPage(0)).Initialize();
    }
}

Table::~Table() {
    this->pager_->FlushPages();

    if (!this->pager_->Close()) {
        std::cout << "Could not close file" << std::endl;
        exit(EXIT_FAILURE);
    }

    delete this->pager_;
}

Cursor::Cursor(Table *table, bool start) {
    this->table_ = table;

    this->pagenum_ = table->root_page_num();

    LeafNode rootnode = LeafNode(table->GetPage(this->pagenum_));
    uint32_t num_cells = *rootnode.NumCells();

    this->cellnum_ = (start) ? 0 : num_cells;
    this->end_of_table_ = (start) ? num_cells == 0 : true;
}

Cursor::~Cursor() {}

void *Cursor::Value() {
    return LeafNode(this->table_->GetPage(this->pagenum_))
        .Value(this->cellnum_);
}

void Cursor::Advance() {
    this->cellnum_++;
    if (this->cellnum_ >=
        *LeafNode(this->table_->GetPage(this->pagenum_)).NumCells()) {
        this->end_of_table_ = true;
    }
}

void LeafNode::Insert(Cursor const &cursor,
                      __attribute__((unused)) uint32_t key,
                      __attribute__((unused)) Row value) {
    uint32_t num_cells = *this->NumCells();

    if (num_cells >= sizes::kLeafNodeMaxCells) {
        std::cout << "splitting node not implemented" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (cursor.cellnum_ < num_cells) {
        // make room for cell
        for (uint32_t i = num_cells; i > cursor.cellnum_; i--) {
            std::memcpy(this->Cell(i), this->Cell(i - 1),
                        sizes::kLeafNodeCellSize);
        }
    }

    uint32_t s = (*this->NumCells()) + 1;

    *(this->NumCells()) = (*this->NumCells()) + 1;
    *this->Key(cursor.cellnum_) = key;
    this->SerializeRow(this->Value(cursor.cellnum_), value);

    *(this->NumCells()) = s;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-arith"
inline void LeafNode::SerializeRow(void *dest, const Row &source) {
    std::memcpy(dest + sizes::kIdOffset, &source.Id, sizes::kIdSize);
    std::memcpy(dest + sizes::kUsernameOffset, &source.Username,
                sizes::kUsernameSize);
    std::memcpy(dest + sizes::kEmailOffset, &source.Email, sizes::kEmailSize);
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-arith"
inline void LeafNode::DeserializeRow(Row &dest, const void *source) {
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

}  // namespace simpledb