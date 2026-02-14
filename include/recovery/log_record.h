#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include "storage/tuple.h"

namespace mydb {

enum class LogRecordType {
    INVALID = 0,
    INSERT,
    COMMIT,
    ABORT,
    BEGIN
};

class LogRecord {
public:
    // Header
    int32_t size_ = 0; 
    int32_t lsn_ = 0; 
    int32_t prev_lsn_ = 0; 
    int32_t txn_id_ = 0;
    LogRecordType log_record_type_ = LogRecordType::INVALID;
    
    // For INSERT
    int32_t page_id_ = -1;
    // We store raw serialized tuple bytes here
    std::vector<char> insert_tuple_data_;

    LogRecord() = default;
    
    // Insert Record Constructor
    LogRecord(int32_t txn_id, int32_t prev_lsn, LogRecordType type, int32_t page_id, const Tuple& tuple)
        : prev_lsn_(prev_lsn), txn_id_(txn_id), log_record_type_(type), page_id_(page_id) {
        
        // Serialize tuple to temp buffer
        uint32_t t_size = tuple.GetSerializedSize();
        insert_tuple_data_.resize(t_size);
        tuple.Serialize(insert_tuple_data_.data());
        
        // Header(20) + PageID(4) + DataSize(4) + Data
        size_ = 20 + 4 + 4 + t_size;
    }
    
    // System Record
    LogRecord(int32_t txn_id, int32_t prev_lsn, LogRecordType type)
        : prev_lsn_(prev_lsn), txn_id_(txn_id), log_record_type_(type) {
        size_ = 20; 
    }

    void Serialize(char* dest) const {
        int32_t offset = 0;
        std::memcpy(dest + offset, &size_, 4); offset += 4;
        std::memcpy(dest + offset, &lsn_, 4); offset += 4;
        std::memcpy(dest + offset, &prev_lsn_, 4); offset += 4;
        std::memcpy(dest + offset, &txn_id_, 4); offset += 4;
        int32_t type = static_cast<int32_t>(log_record_type_);
        std::memcpy(dest + offset, &type, 4); offset += 4;
        
        if (log_record_type_ == LogRecordType::INSERT) {
             std::memcpy(dest + offset, &page_id_, 4); offset += 4;
             int32_t data_size = insert_tuple_data_.size();
             std::memcpy(dest + offset, &data_size, 4); offset += 4;
             std::memcpy(dest + offset, insert_tuple_data_.data(), data_size);
        }
    }
    
    static LogRecord Deserialize(const char* src) {
        LogRecord log;
        int32_t offset = 0;
        std::memcpy(&log.size_, src + offset, 4); offset += 4;
        std::memcpy(&log.lsn_, src + offset, 4); offset += 4;
        std::memcpy(&log.prev_lsn_, src + offset, 4); offset += 4;
        std::memcpy(&log.txn_id_, src + offset, 4); offset += 4;
        int32_t type;
        std::memcpy(&type, src + offset, 4); offset += 4;
        log.log_record_type_ = static_cast<LogRecordType>(type);
        
        if (log.log_record_type_ == LogRecordType::INSERT) {
             std::memcpy(&log.page_id_, src + offset, 4); offset += 4;
             int32_t data_size;
             std::memcpy(&data_size, src + offset, 4); offset += 4;
             log.insert_tuple_data_.resize(data_size);
             std::memcpy(log.insert_tuple_data_.data(), src + offset, data_size);
        }
        return log;
    }
};

} // namespace mydb
