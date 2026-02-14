#pragma once

#include "storage/page/b_plus_tree_page.h"
#include "type/type_id.h"

namespace mydb {

#define B_PLUS_TREE_INTERNAL_PAGE_TYPE BPlusTreeInternalPage
#define INTERNAL_PAGE_HEADER_SIZE 24
#define INTERNAL_PAGE_SIZE ((PAGE_SIZE - INTERNAL_PAGE_HEADER_SIZE) / (sizeof(std::pair<int, int>)))

/**
 * Store n keys and n+1 child pointers (page_id).
 * Key type: int (Phase 2 requirement simplified)
 * Value type: page_id_t
 * 
 * note: The first key is invalid (or ignored), it acts as a router.
 */
class BPlusTreeInternalPage : public BPlusTreePage {
public:
    void Init(page_id_t page_id, page_id_t parent_id = -1, int max_size = INTERNAL_PAGE_SIZE);

    int KeyAt(int index) const;
    void SetKeyAt(int index, int key);
    
    page_id_t ValueAt(int index) const;
    void SetValueAt(int index, page_id_t value);

    int ValueIndex(page_id_t value) const;
    page_id_t Lookup(const int &key, const int &comparator) const;

    // Split & Merge utils
    void MoveHalfTo(BPlusTreeInternalPage *recipient, char *buffer);
    void MoveAllTo(BPlusTreeInternalPage *recipient, int index_in_parent, char *buffer);

private:
    // Flexible array member
    // In a real implementation we might use a char array and cast it
    // Mapping: array[0].second is the pointer to the left of array[1].first
    std::pair<int, page_id_t> array_[1]; 
};

} // namespace mydb
