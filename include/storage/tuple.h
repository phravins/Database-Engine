#pragma once

#include <vector>
#include "type/value.h"
#include "catalog/schema.h"

namespace mydb {

class Tuple {
public:
    Tuple() = default;
    
    explicit Tuple(std::vector<Value> values) : values_(std::move(values)) {}

    const Value& GetValue(uint32_t idx) const {
        return values_[idx];
    }
    
    // Serialize tuple to buffer
    // Format: [Count] [Value1] [Value2] ... 
    // This is a simplified row format.
    uint32_t Serialize(char* dest) const {
        uint32_t count = values_.size();
        uint32_t offset = 0;
        
        // Write value count
        std::memcpy(dest + offset, &count, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        
        for (const auto& val : values_) {
            offset += val.Serialize(dest + offset);
        }
        return offset;
    }
    
    // Deserialize tuple from buffer
    static Tuple Deserialize(const char* src, const Schema& schema) {
         // Note: In a real system, we'd use the schema to know types.
         // Here, our serialized format includes simple length for Varchar, but we need types.
         // Let's assume the schema matches the stored data order perfectly.
         
         uint32_t count;
         uint32_t offset = 0;
         std::memcpy(&count, src + offset, sizeof(uint32_t));
         offset += sizeof(uint32_t);
         
         std::vector<Value> values;
         for (uint32_t i = 0; i < count; ++i) {
             const Column& col = schema.GetColumn(i);
             // We peek/deserialize based on type
             if (col.GetType() == TypeID::INTEGER) {
                 Value v = Value::Deserialize(src + offset, TypeID::INTEGER);
                 values.push_back(v);
                 offset += v.GetSerializedSize();
             } else if (col.GetType() == TypeID::VARCHAR) {
                 Value v = Value::Deserialize(src + offset, TypeID::VARCHAR);
                 values.push_back(v);
                 offset += v.GetSerializedSize();
             }
         }
         return Tuple(values);
    }
    
    uint32_t GetSerializedSize() const {
        uint32_t size = sizeof(uint32_t); // count
        for (const auto& val : values_) {
            size += val.GetSerializedSize();
        }
        return size;
    }

private:
    std::vector<Value> values_;
};

} // namespace mydb
