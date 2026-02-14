#pragma once

#include <cstdint>
#include <sstream>

namespace mydb {

class RID {
public:
    RID() = default;
    RID(int32_t page_id, uint32_t slot_num) : page_id_(page_id), slot_num_(slot_num) {}

    int32_t GetPageId() const { return page_id_; }
    uint32_t GetSlotNum() const { return slot_num_; }
    
    void Set(int32_t page_id, uint32_t slot_num) {
        page_id_ = page_id;
        slot_num_ = slot_num;
    }

    bool operator==(const RID& other) const {
        return page_id_ == other.page_id_ && slot_num_ == other.slot_num_;
    }

    friend std::ostream& operator<<(std::ostream& os, const RID& rid) {
        os << "RID(" << rid.page_id_ << ", " << rid.slot_num_ << ")";
        return os;
    }

private:
    int32_t page_id_ = -1;
    uint32_t slot_num_ = 0;
};

} // namespace mydb
