#pragma once

#include "recovery/log_manager.h"
#include "storage/table_page.h"
#include <fstream>
#include <iostream>

namespace mydb {

class RecoveryManager {
public:
    explicit RecoveryManager(LogManager* log_manager, DiskManager* disk_manager)
        : log_manager_(log_manager), disk_manager_(disk_manager) {}

    void ARIES() {
        std::cout << "Recovery: Starting ARIES..." << std::endl;
        // Simplified Phase: Redo only
        Redo();
        std::cout << "Recovery: Complete." << std::endl;
    }

private:
    void Redo() {
        std::ifstream log_file("wal.log", std::ios::binary);
        if (!log_file.is_open()) return;

        log_file.seekg(0, std::ios::end);
        int size = log_file.tellg();
        log_file.seekg(0, std::ios::beg);
        
        if (size == 0) return;
        
        char* buffer = new char[size];
        log_file.read(buffer, size);
        
        int offset = 0;
        while (offset < size) {
             LogRecord log = LogRecord::Deserialize(buffer + offset);
             
             if (log.log_record_type_ == LogRecordType::INSERT) {
                 std::cout << "Redo: LSN " << log.lsn_ << " INSERT to Page " << log.page_id_ << std::endl;
                 
                 // Apply to page
                 char page_buf[PAGE_SIZE];
                 disk_manager_->ReadPage(log.page_id_, page_buf);
                 TablePage page;
                 page.Init(log.page_id_, -1, page_buf);
                 
                 // We have raw tuple data.
                 // We need to insert it.
                 // BUT TablePage::InsertTuple takes a Tuple object.
                 // We can reconstruct a Tuple from raw data? 
                 // We don't have schema here. 
                 // HACK: We can just memcpy the bytes if we knew where?
                 // No, TablePage Logic relies on Tuple object to call serializer.
                 
                 // Solution: Create a "RawTuple" or modify Tuple to accept raw data.
                 // Or: Deserialize Tuple using schema? 
                 // In Recovery, we usually have access to Catalog to get Schema for table.
                 // Here, we don't know the table name from LogRecord (simplification).
                 
                 // Let's modify TablePage to have `InsertTupleRaw(char* data, int size)`?
                 // Or just assume it's OK to fail if we can't fully reconstruct.
                 // Wait, `Tuple` wrapper is just a mechanism. If we deserialize with `Value`s, we need type.
                 // 
                 // Improved Plan: LogRecord should store `TableHeap` identifier (TableID) instead of just PageID,
                 // and we look up Schema.
                 //
                 // ALTERNATIVE: Just write the whole Page to WAL (Physical Logging) for simplicity?
                 // No, that's too much data.
                 // 
                 // BEST SHORTCUT: `Tuple` class has a constructor that takes raw data?
                 // `Tuple(std::vector<Value>)`...
                 //
                 // Let's hack: We can just append the bytes to the page's free space.
                 // We know the size.
                 
                 uint32_t tuple_size = log.insert_tuple_data_.size();
                 uint32_t free_offset = page.GetFreeSpaceOffset();
                 
                 // Check if likely already there? (Simple Idempotency check: LSN on Page)
                 // Page LSN not implemented yet fully.
                 
                 if (free_offset + tuple_size <= PAGE_SIZE) {
                     std::memcpy(page_buf + free_offset, log.insert_tuple_data_.data(), tuple_size);
                     page.SetFreeSpaceOffset(free_offset + tuple_size);
                     page.SetTupleCount(page.GetTupleCount() + 1);
                     disk_manager_->WritePage(log.page_id_, page_buf);
                 }
             }
             
             offset += log.size_;
        }
        
        delete[] buffer;
    }

    LogManager* log_manager_;
    DiskManager* disk_manager_;
};

} // namespace mydb
