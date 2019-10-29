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

    std::streampos begin = this->file_.tellg();
    file_.seekg(0, std::ios::end);
    std::streampos end = this->file_.tellg();

    this->file_length_ = end - begin;

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

        uint32_t num_pages = this->file_length_ / sizes::kPageSize;
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
    }

    return this->pages_[pagenum];
}

void Pager::FlushPages(uint32_t num_rows) {
    uint32_t full_pages = num_rows / sizes::kRowsPerPage;

    for (uint32_t i = 0; i < full_pages; i++) {
        if (this->pages_[i] == nullptr) continue;
        this->FlushPage(i, sizes::kPageSize);
    }

    uint32_t additional_rows = num_rows % sizes::kRowsPerPage;
    if (additional_rows) {
        if (this->pages_[full_pages] != nullptr) {
            this->FlushPage(full_pages, additional_rows * sizes::kRowsize);
        }
    }
}

void Pager::FlushPage(uint32_t pagenum, uint32_t size) {
    if (this->pages_[pagenum] == nullptr) {
        std::cout << "Tried to flush null page" << std::endl;
        exit(EXIT_FAILURE);
    }

    this->file_.seekp(pagenum * size);
    if (this->file_.fail()) {
        std::cout << "unable to seek to page ( " << pagenum << ")" << std::endl;
        exit(EXIT_FAILURE);
    }

    this->file_.write(static_cast<char *>(this->pages_[pagenum]), size);
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
    this->num_rows_ = this->pager_->file_length() / sizes::kRowsize;
}

Table::~Table() {
    this->pager_->FlushPages(this->num_rows());

    if (!this->pager_->Close()) {
        std::cout << "Could not close file" << std::endl;
        exit(EXIT_FAILURE);
    }

    delete this->pager_;
}

Cursor::Cursor(Table *table, bool start) {
    this->table_ = table;
    this->row_num_ = (start) ? 0 : table->num_rows();
    this->end_of_table_ = (start) ? this->table_->num_rows() == 0 : true;
}

Cursor::~Cursor() {}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-arith"
void *Cursor::Value() {
    uint32_t page_num = this->row_num_ / sizes::kRowsPerPage;
    void *page = this->table_->GetPage(page_num);
    uint32_t row_offset = this->row_num_ % sizes::kRowsPerPage;
    uint32_t byte_offset = row_offset * sizes::kRowsize;
    return page + byte_offset;
}
#pragma GCC diagnostic pop

void Cursor::Advance() {
    this->row_num_++;
    if (this->row_num_ >= this->table_->num_rows()) {
        this->end_of_table_ = true;
    }
}

}  // namespace simpledb