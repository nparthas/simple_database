#include "dbtypes.h"

namespace simpledb {

Pager::Pager(std::string const &filename) {
    this->filename = filename;
    this->file.open(filename, std::ios::in | std::ios::out | std::ios::app |
                                  std::ios::binary);

    if (!this->file) {
        std::cout << "Unable to create connection";
        exit(EXIT_FAILURE);
    }

    std::streampos begin = this->file.tellg();
    file.seekg(0, std::ios::end);
    std::streampos end = this->file.tellg();

    this->file_length = end - begin;

    for (uint32_t i = 0; i < sizes::kTableMaxPages; i++) {
        this->pages[i] = nullptr;
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-incomplete"
Pager::~Pager() {
    for (uint32_t i = 0; i < sizes::kTableMaxPages; i++) {
        if (this->pages[i] != nullptr) {
            delete[] this->pages[i];
            this->pages[i] = nullptr;
        }
    }
}
#pragma GCC diagnostic pop

Pager::Pager(const Pager &pager) { Pager(pager.filename); }

Pager::Pager(Pager &&pager) noexcept {
    if (this == &pager) return;

    this->filename = pager.filename;
    this->file = std::fstream(std::move(pager.file));
    this->file_length = pager.file_length;

    for (size_t i = 0; i < sizes::kTableMaxPages; i++) {
        this->pages[i] = std::move(pager.pages[i]);
        pager.pages[i] = nullptr;
    }

    pager.filename = "";
    pager.file_length = 0;
}

void *Pager::GetPage(uint32_t pagenum) {
    if (pagenum > sizes::kTableMaxPages) {
        std::cout << "canont fetch page ( " << pagenum << ") out of bounds ( "
                  << sizes::kTableMaxPages << ")" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (this->pages[pagenum] == nullptr) {
        void *page = operator new(sizes::kPageSize);

        uint32_t num_pages = this->file_length / sizes::kPageSize;
        if (this->file_length %
            sizes::kPageSize) {  // make sure that we have enough space if
                                 // we save a partial page
            num_pages++;
        }

        if (pagenum <= num_pages) {
            this->file.seekg(pagenum * sizes::kPageSize);
            if (this->file.fail()) {
                std::cout << "unable to seek to page ( " << pagenum << ")"
                          << std::endl;
                exit(EXIT_FAILURE);
            }

            this->file.read(static_cast<char *>(page), sizes::kPageSize);
            // ensure the badbit is not set or we have the failbit set
            // witout the eof bit
            if (this->file.bad() || (this->file.fail() && !this->file.eof())) {
                std::cout << "unable to read existing page (" << pagenum << ")"
                          << std::endl;
                exit(EXIT_FAILURE);
            }

            if (file.eof()) {
                file.clear();
            }
        }

        this->pages[pagenum] = page;
    }

    return this->pages[pagenum];
}

void Pager::FlushPages(uint32_t num_rows) {
    uint32_t full_pages = num_rows / sizes::kRowsPerPage;

    for (uint32_t i = 0; i < full_pages; i++) {
        if (this->pages[i] == nullptr) continue;
        this->FlushPage(i, sizes::kPageSize);
    }

    uint32_t additional_rows = num_rows % sizes::kRowsPerPage;
    if (additional_rows) {
        if (this->pages[full_pages] != nullptr) {
            this->FlushPage(full_pages, additional_rows * sizes::kRowsize);
        }
    }
}

void Pager::FlushPage(uint32_t pagenum, uint32_t size) {
    if (this->pages[pagenum] == nullptr) {
        std::cout << "Tried to flush null page" << std::endl;
        exit(EXIT_FAILURE);
    }

    this->file.seekp(pagenum * size);
    if (this->file.fail()) {
        std::cout << "unable to seek to page ( " << pagenum << ")" << std::endl;
        exit(EXIT_FAILURE);
    }

    this->file.write(static_cast<char *>(this->pages[pagenum]), size);
}

bool Pager::Close() {
    this->file.close();
    return this->file.fail();
}

void Pager::Dump(int pagenum) {
    for (uint32_t i = 0; i < sizes::kPageSize; i++) {
        std::cout << static_cast<char *>(this->pages[pagenum])[i] << std::endl;
    }
    std::cout << std::endl;
}

}  // namespace simpledb