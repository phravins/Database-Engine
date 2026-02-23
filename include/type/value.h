#pragma once

#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include "type/type_id.h"

namespace mydb {

/**
 * Value is a wrapper for data. It holds the type and the actual data.
 * For simplicity in this phase:
 *  - INTEGER: 4-byte int
 *  - VARCHAR: string (length + chars)
 */
class Value {
public:
    // Invalid constructor
    Value() : type_id_(TypeID::INVALID) {}

    // Integer constructor
    explicit Value(int32_t i) : type_id_(TypeID::INTEGER) {
        value_.integer_ = i;
    }

    // Varchar constructor
    explicit Value(const std::string& s) : type_id_(TypeID::VARCHAR) {
        len_ = static_cast<uint32_t>(s.length());
        str_ = s;
    }

    TypeID GetTypeId() const { return type_id_; }

    int32_t GetAsInteger() const {
        return value_.integer_;
    }

    std::string GetAsString() const {
        return str_;
    }

    // Serialize to a buffer (for disk storage)
    // Returns number of bytes written
    uint32_t Serialize(char* dest) const {
        if (type_id_ == TypeID::INTEGER) {
            std::memcpy(dest, &value_.integer_, sizeof(int32_t));
            return sizeof(int32_t);
        } else if (type_id_ == TypeID::VARCHAR) {
            uint32_t size = str_.length();
            std::memcpy(dest, &size, sizeof(uint32_t));
            std::memcpy(dest + sizeof(uint32_t), str_.c_str(), size);
            return sizeof(uint32_t) + size;
        }
        return 0;
    }

    // Deserialize from a buffer
    // Returns number of bytes read
    static Value Deserialize(const char* src, TypeID type_id) {
        if (type_id == TypeID::INTEGER) {
            int32_t val;
            std::memcpy(&val, src, sizeof(int32_t));
            return Value(val);
        } else if (type_id == TypeID::VARCHAR) {
            uint32_t size;
            std::memcpy(&size, src, sizeof(uint32_t));
            std::string s(src + sizeof(uint32_t), size);
            return Value(s);
        }
        return Value();
    }
    
    // Get serialization size
    uint32_t GetSerializedSize() const {
         if (type_id_ == TypeID::INTEGER) {
            return sizeof(int32_t);
        } else if (type_id_ == TypeID::VARCHAR) {
            return sizeof(uint32_t) + str_.length();
        }
        return 0;
    }

    // Equality check for testing
    bool operator==(const Value& other) const {
        if (type_id_ != other.type_id_) return false;
        if (type_id_ == TypeID::INTEGER) return value_.integer_ == other.value_.integer_;
        if (type_id_ == TypeID::VARCHAR) return str_ == other.str_;
        return true;
    }

private:
    TypeID type_id_;
    union Val {
        int32_t integer_;
    } value_;
    // Simplified storage for varchar (not optimized union yet)
    uint32_t len_ = 0;
    std::string str_;
};

} // namespace mydb
