#pragma once

#include "storage/page/b_plus_tree_page.h"
#include "common/rid.h"

namespace mydb {

#define B_PLUS_TREE_LEAF_PAGE_TYPE BPlusTreeLeafPage
#define LEAF_PAGE_HEADER_SIZE 28
#define LEAF_PAGE_SIZE ((PAGE_SIZE - LEAF_PAGE_HEADER_SIZE) / (sizeof(std::pair<int, RID>)))

/**
 * Store n keys and n values (RID).
 * Linked list pointers (next/prev) leaf pages.
 */
class BPlusTreeLeafPage : public BPlusTreePage {
public:
    void Init(page_id_t page_id, page_id_t parent_id = -1, int max_size = LEAF_PAGE_SIZE);

    // Helpers
    page_id_t GetNextPageId() const;
    void SetNextPageId(page_id_t next_page_id);

    int KeyAt(int index) const;
    RID ValueAt(int index) const;
    const std::pair<int, RID> &GetItem(int index);

    // Insert
    int Insert(const int &key, const RID &value, const int &comparator);
    
    // Split
    void MoveHalfTo(BPlusTreeLeafPage *recipient);

    // Look up
    int KeyIndex(const int &key, const int &comparator) const;
    RID Lookup(const int &key, const int &comparator) const;

private:
    page_id_t next_page_id_;
    std::pair<int, RID> array_[1];
};

} // namespace mydb
