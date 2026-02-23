#pragma once

#include <cstring>
#include "common/config.h"
#include "storage/tuple.h"
#include "storage/disk_manager.h" 

namespace mydb {

/**
 * TablePage format (Simplified):
 * [NextPageID (4)] [TupleCount (4)] [FreeSpaceOffset (4)] ... [Tuples...]
 * 
 * We simply append tuples one after another for this simplified version.
 * Real DBs use Slotted Page Design (Footer with offsets).
 * We will stick to Append-Only for Phase 2 simplicity unless requested otherwise.
 */
class TablePage {
public:
    void Init(page_id_t page_id, page_id_t prev_page_id, char* data) {
         page_id_ = page_id;
         prev_page_id_ = prev_page_id;
         data_ = data;
         // If it's a new page, simpler init
         // Reads existing header if reading from disk, but for now we assume we manage it via higher level
    }
    
    // Header offsets
    static constexpr uint32_t OFFSET_NEXT_PAGE = 0;
    static constexpr uint32_t OFFSET_TUPLE_COUNT = 4;
    static constexpr uint32_t OFFSET_FREE_SPACE = 8;
    static constexpr uint32_t HEADER_SIZE = 12;

    page_id_t GetNextPageId() const {
        return *reinterpret_cast<page_id_t*>(data_ + OFFSET_NEXT_PAGE);
    }
    
    void SetNextPageId(page_id_t next_page_id) {
        *reinterpret_cast<page_id_t*>(data_ + OFFSET_NEXT_PAGE) = next_page_id;
    }
    
    uint32_t GetTupleCount() const {
        return *reinterpret_cast<uint32_t*>(data_ + OFFSET_TUPLE_COUNT);
    }

    void SetTupleCount(uint32_t count) {
        *reinterpret_cast<uint32_t*>(data_ + OFFSET_TUPLE_COUNT) = count;
    }
    
    uint32_t GetFreeSpaceOffset() const {
         return *reinterpret_cast<uint32_t*>(data_ + OFFSET_FREE_SPACE);
    }
    
    void SetFreeSpaceOffset(uint32_t offset) {
        *reinterpret_cast<uint32_t*>(data_ + OFFSET_FREE_SPACE) = offset;
    }

    // Initialize a blank page
    void InitNewPage(page_id_t next_page_id = -1) {
        SetNextPageId(next_page_id);
        SetTupleCount(0);
        SetFreeSpaceOffset(HEADER_SIZE);
    }

    // Try to insert a tuple. Returns true if successful, false if no space.
    bool InsertTuple(const Tuple& tuple) {
        uint32_t size = tuple.GetSerializedSize();
        uint32_t free_offset = GetFreeSpaceOffset();
        
        if (free_offset + size > PAGE_SIZE) {
            return false;
        }
        
        tuple.Serialize(data_ + free_offset);
        SetFreeSpaceOffset(free_offset + size);
        SetTupleCount(GetTupleCount() + 1);
        return true;
    }
    
    // Read all tuples (full scan helper)
    // In a real system we'd have an Iterator or GetTuple(slot_id).
    // Here we just return all for checking.
    std::vector<Tuple> GetAllTuples(const Schema& schema) const {
        std::vector<Tuple> tuples;
        uint32_t count = GetTupleCount();
        uint32_t offset = HEADER_SIZE;
        
        for (uint32_t i = 0; i < count; ++i) {
            Tuple t = Tuple::Deserialize(data_ + offset, schema);
            tuples.push_back(t);
            offset += t.GetSerializedSize();
        }
        return tuples;
    }

private:
    page_id_t page_id_;
    page_id_t prev_page_id_;
    char* data_; // Pointer to the 4KB buffer in memory
};

} // namespace mydb
