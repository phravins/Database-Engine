#include "storage/disk_manager.h"
#include <iostream>
#include <cstring>
#include <filesystem>
#include <stdexcept>

namespace mydb {

DiskManager::DiskManager(const std::string& db_file) : file_name_(db_file) {
    db_io_.open(file_name_, std::ios::binary | std::ios::in | std::ios::out);
    if (!db_io_.is_open()) {
        // file does not exist, create it
        db_io_.clear();
        db_io_.open(file_name_, std::ios::binary | std::ios::trunc | std::ios::out);
        db_io_.close();
        // re-open with original mode
        db_io_.open(file_name_, std::ios::binary | std::ios::in | std::ios::out);
        if (!db_io_.is_open()) {
            throw std::runtime_error("can't open db file");
        }
    }
}

DiskManager::~DiskManager() {
    ShutDown();
}

void DiskManager::ShutDown() {
    if (db_io_.is_open()) {
        db_io_.close();
    }
}

void DiskManager::WritePage(page_id_t page_id, const char* page_data) {
    std::lock_guard<std::mutex> guard(db_io_mutex_);
    int offset = page_id * PAGE_SIZE;
    db_io_.seekp(offset);
    db_io_.write(page_data, PAGE_SIZE);
    if (db_io_.bad()) {
        std::cerr << "I/O error while writing" << std::endl;
        return;
    }
    db_io_.flush();
}

void DiskManager::ReadPage(page_id_t page_id, char* page_data) {
    std::lock_guard<std::mutex> guard(db_io_mutex_);
    int offset = page_id * PAGE_SIZE;
    if (offset > GetFileSize(file_name_)) {
        // read past end of file
         std::cerr << "I/O error while reading past end of file" << std::endl;
         std::memset(page_data, 0, PAGE_SIZE);
         return;
    }
    db_io_.seekg(offset);
    db_io_.read(page_data, PAGE_SIZE);
    if (db_io_.bad()) {
        std::cerr << "I/O error while reading" << std::endl;
        return;
    }
    int read_count = db_io_.gcount();
    if (read_count < PAGE_SIZE) {
        std::cerr << "Read less than a page" << std::endl;
        db_io_.clear();
        std::memset(page_data + read_count, 0, PAGE_SIZE - read_count);
    }
}

int DiskManager::GetFileSize(const std::string& file_name) {
    try {
        return static_cast<int>(std::filesystem::file_size(file_name));
    } catch (...) {
        return -1;
    }
}

} // namespace mydb
