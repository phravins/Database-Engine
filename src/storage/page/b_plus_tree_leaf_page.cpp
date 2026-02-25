#include "storage/page/b_plus_tree_leaf_page.h"

namespace mydb {

void BPlusTreeLeafPage::Init(page_id_t page_id, page_id_t parent_id, int max_size) {
    SetPageType(IndexPageType::LEAF_PAGE);
    SetSize(0);
    SetMaxSize(max_size);
    SetParentPageId(parent_id);
    SetPageId(page_id);
    SetNextPageId(-1);
}

page_id_t BPlusTreeLeafPage::GetNextPageId() const {
    return next_page_id_;
}

void BPlusTreeLeafPage::SetNextPageId(page_id_t next_page_id) {
    next_page_id_ = next_page_id;
}

int BPlusTreeLeafPage::KeyAt(int index) const {
    return array_[index].first;
}

RID BPlusTreeLeafPage::ValueAt(int index) const {
    return array_[index].second;
}

const std::pair<int, RID>& BPlusTreeLeafPage::GetItem(int index) {
    return array_[index];
}

int BPlusTreeLeafPage::KeyIndex(const int &key, const int &comparator) const {
    for (int i = 0; i < GetSize(); ++i) {
        if (key == KeyAt(i)) {
            return i;
        }
    }
    return -1;
}

RID BPlusTreeLeafPage::Lookup(const int &key, const int &comparator) const {
    int index = KeyIndex(key, comparator);
    if (index != -1) {
        return ValueAt(index);
    }
    return RID();
}

int BPlusTreeLeafPage::Insert(const int &key, const RID &value, const int &comparator) {
    // Simple sorted insert
    int i = 0;
    for (; i < GetSize(); ++i) {
        if (KeyAt(i) >= key) break;
    }
    // Check duplicate
    if (i < GetSize() && KeyAt(i) == key) return 0; // Already exists

    // Shift
    for (int j = GetSize(); j > i; --j) {
        array_[j] = array_[j - 1];
    }
    array_[i] = {key, value};
    IncreaseSize(1);
    return GetSize();
}

void BPlusTreeLeafPage::MoveHalfTo(BPlusTreeLeafPage *recipient) {
    // Not implemented
}

} // namespace mydb
