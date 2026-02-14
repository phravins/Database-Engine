#pragma once

#include <string>
#include "type/type_id.h"

namespace mydb {

class Column {
public:
    Column(std::string name, TypeID type_id, uint32_t offset)
        : name_(std::move(name)), type_id_(type_id), offset_(offset) {}

    std::string GetName() const { return name_; }
    TypeID GetType() const { return type_id_; }
    uint32_t GetOffset() const { return offset_; }
    
    // Fixed size for fixed-length types (INTEGER), 0 for variable (VARCHAR)
    uint32_t GetFixedLength() const {
        if (type_id_ == TypeID::INTEGER) return 4;
        return 0; 
    }

private:
    std::string name_;
    TypeID type_id_;
    uint32_t offset_; // Byte offset in tuple (if fixed schema) - simpler approach for now: we might just serialize values in order
};

} // namespace mydb
