#pragma once

#include "recovery/log_record.h"
#include "storage/disk_manager.h"
#include <mutex>
#include <vector>

namespace mydb {

class LogManager {
public:
    using lsn_t = int32_t;

    explicit LogManager(DiskManager* disk_manager) : disk_manager_(disk_manager) {
        // Create log file if needed
    }
    
    void Flush() {
        std::lock_guard<std::mutex> guard(latch_);
        if (log_buffer_.empty()) return;
        
        // Simple append to "wal.log"
        // In real system, DiskManager might handle this or we open separate stream
        // Let's use std::fstream here for simplicity distinct from DB file
        std::ofstream log_file("wal.log", std::ios::binary | std::ios::app);
        log_file.write(log_buffer_.data(), log_buffer_.size());
        log_file.flush();
        log_file.close();
        
        log_buffer_.clear();
        persistent_lsn_ = next_lsn_ - 1;
    }
    
    lsn_t AppendLogRecord(LogRecord& log_record) {
        std::lock_guard<std::mutex> guard(latch_);
        
        log_record.lsn_ = next_lsn_++;
        
        // Serialize
        // We assume max record size < 4KB for simplicity or we resize
        char buf[PAGE_SIZE]; 
        log_record.Serialize(buf);
        
        // Add to buffer
        int size = log_record.size_;
        size_t current_pos = log_buffer_.size();
        log_buffer_.resize(current_pos + size);
        std::memcpy(log_buffer_.data() + current_pos, buf, size);
        
        return log_record.lsn_;
    }

private:
    DiskManager* disk_manager_;
    std::mutex latch_;
    std::vector<char> log_buffer_;
    lsn_t next_lsn_ = 0;
    lsn_t persistent_lsn_ = -1;
};

} // namespace mydb
