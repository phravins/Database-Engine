#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include "common/config.h"

namespace mydb {

/**
 * DiskManager takes care of the allocation and deallocation of pages within a database.
 * It performs the reading and writing of pages to and from disk, providing a logical file layer within the
 * context of a database management system.
 */
class DiskManager {
public:
    explicit DiskManager(const std::string& db_file);
    ~DiskManager();

    /**
     * Write a page to the database file.
     * @param page_id id of the page
     * @param page_data raw page data
     */
    void WritePage(page_id_t page_id, const char* page_data);

    /**
     * Read a page from the database file.
     * @param page_id id of the page
     * @param[out] page_data output buffer
     */
    void ReadPage(page_id_t page_id, char* page_data);

    /**
     * Shutdown the disk manager and close all files.
     */
    void ShutDown();

    /**
     * Get the size of the file.
     */
    int GetFileSize(const std::string& file_name);

private:
    std::string file_name_;
    std::fstream db_io_;
    std::mutex db_io_mutex_; 
};

} // namespace mydb
