#include "storage/page/b_plus_tree_internal_page.h"
#include <iostream>

namespace mydb {

void BPlusTreeInternalPage::Init(page_id_t page_id, page_id_t parent_id, int max_size) {
    SetPageType(IndexPageType::INTERNAL_PAGE);
    SetSize(0);
    SetMaxSize(max_size);
    SetParentPageId(parent_id);
    SetPageId(page_id);
}

int BPlusTreeInternalPage::KeyAt(int index) const {
    return array_[index].first;
}

void BPlusTreeInternalPage::SetKeyAt(int index, int key) {
    array_[index].first = key;
}

page_id_t BPlusTreeInternalPage::ValueAt(int index) const {
    return array_[index].second;
}

void BPlusTreeInternalPage::SetValueAt(int index, page_id_t value) {
    array_[index].second = value;
}

int BPlusTreeInternalPage::ValueIndex(page_id_t value) const {
    for (int i = 0; i < GetSize(); ++i) {
        if (ValueAt(i) == value) {
            return i;
        }
    }
    return -1;
}

page_id_t BPlusTreeInternalPage::Lookup(const int &key, const int &comparator) const {
    // Basic linear search for phase 3, binary search later
    // The keys are sorted. We want the last key <= target.
    // array[0] key is invalid. 
    // keys: [X, 10, 20, 30]
    // ptrs: [p0, p1, p2, p3]
    // if key < 10 -> p0
    // if 10 <= key < 20 -> p1
    
    // For now simple scan from 1
    for (int i = 1; i < GetSize(); ++i) {
        if (key < KeyAt(i)) {
            return ValueAt(i - 1);
        }
    }
    return ValueAt(GetSize() - 1);
}

void BPlusTreeInternalPage::MoveHalfTo(BPlusTreeInternalPage *recipient, char *buffer) {
    // Not implemented for Phase 3 prototype yet
}

void BPlusTreeInternalPage::MoveAllTo(BPlusTreeInternalPage *recipient, int index_in_parent, char *buffer) {
    // Not implemented
}

} // namespace mydb
