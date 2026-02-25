#pragma once

#include "storage/page/b_plus_tree_leaf_page.h"
#include "storage/page/b_plus_tree_internal_page.h"
#include "storage/disk_manager.h"
#include <string>

namespace mydb {

class BPlusTree {
public:
    explicit BPlusTree(std::string name, DiskManager* buffer_pool_manager);

    // Returns true if the key exists
    bool GetValue(const int &key, RID &result);

    // Insert a key-value pair
    bool Insert(const int &key, const RID &value);

    // Remove key (optional for Phase 3)
    void Remove(const int &key);
    
    // Debug
    void Print(page_id_t page_id = -1);

private:
    void StartNewTree(const int &key, const RID &value);
    bool InsertIntoLeaf(const int &key, const RID &value);
    void InsertIntoParent(BPlusTreePage *old_node, const int &key, BPlusTreePage *new_node);
    
    template <typename N>
    N *FetchPage(page_id_t page_id);

    std::string index_name_;
    page_id_t root_page_id_;
    DiskManager* disk_manager_; // We really need a BufferPoolManager here, but sticking to DiskManager for now means manual memory management mess.
    // HACK: We will just read/write pages directly. 
};

} // namespace mydb
