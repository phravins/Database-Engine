#include "index/b_plus_tree.h"
#include <iostream>

namespace mydb {

BPlusTree::BPlusTree(std::string name, DiskManager* disk_manager)
    : index_name_(std::move(name)), root_page_id_(-1), disk_manager_(disk_manager) {}

// Helper to cast raw page data
template <typename N>
N* CastPage(char* raw_data) {
    return reinterpret_cast<N*>(raw_data);
}

bool BPlusTree::GetValue(const int &key, RID &result) {
    if (root_page_id_ == -1) return false;

    char buf[PAGE_SIZE]; // Only holds one page at a time (inefficient but works)
    page_id_t current_page_id = root_page_id_;
    
    // Traverse down
    while (true) {
        disk_manager_->ReadPage(current_page_id, buf);
        BPlusTreePage* page = CastPage<BPlusTreePage>(buf);
        
        if (page->IsLeafPage()) {
             BPlusTreeLeafPage* leaf = CastPage<BPlusTreeLeafPage>(buf);
             RID rid = leaf->Lookup(key, 0);
             if (rid.GetPageId() != -1) {
                 result = rid;
                 return true;
             }
             return false;
        } else {
             BPlusTreeInternalPage* internal = CastPage<BPlusTreeInternalPage>(buf);
             current_page_id = internal->Lookup(key, 0);
        }
    }
}

bool BPlusTree::Insert(const int &key, const RID &value) {
    if (root_page_id_ == -1) {
        StartNewTree(key, value);
        return true;
    }
    return InsertIntoLeaf(key, value);
}

void BPlusTree::StartNewTree(const int &key, const RID &value) {
    // 1. Allocate page
    int file_size = disk_manager_->GetFileSize("mydb.db");
    page_id_t page_id = file_size / PAGE_SIZE;
    root_page_id_ = page_id;
    
    // 2. Init Leaf
    char buf[PAGE_SIZE];
    std::memset(buf, 0, PAGE_SIZE);
    BPlusTreeLeafPage* leaf = CastPage<BPlusTreeLeafPage>(buf);
    leaf->Init(page_id, -1);
    leaf->Insert(key, value, 0);
    
    // 3. Write
    disk_manager_->WritePage(page_id, buf);
}

bool BPlusTree::InsertIntoLeaf(const int &key, const RID &value) {
    // Traverse to leaf
    char buf[PAGE_SIZE];
    page_id_t current_page_id = root_page_id_;
    
    // 1. Find leaf
    // We ideally need to track the path to update parents, but for now let's assume no split for simplicity 
    // OR we just find leaf and handle split (which is complex without holding parent pointers in memory)
    
    // Hack: Implementation without split for now (Fixed Limit) to pass Phase 3 basic requirements
    // Real split logic requires BufferPool to pin pages. With just DiskManager it's hard to modify parent.
    // For this "Step 2" of development, we can stick to single-root leaf or simplified logic.
    // Let's implement full traversal.
    
    while (true) {
        disk_manager_->ReadPage(current_page_id, buf);
        BPlusTreePage* page = CastPage<BPlusTreePage>(buf);
        
        if (page->IsLeafPage()) {
            break; 
        } else {
            BPlusTreeInternalPage* internal = CastPage<BPlusTreeInternalPage>(buf);
            current_page_id = internal->Lookup(key, 0);
        }
    }
    
    // Now buf contains the leaf
    BPlusTreeLeafPage* leaf = CastPage<BPlusTreeLeafPage>(buf);
    
    if (leaf->GetSize() < leaf->GetMaxSize()) {
        leaf->Insert(key, value, 0);
        disk_manager_->WritePage(current_page_id, buf);
        return true;
    } else {
        std::cerr << "Split logic not fully implemented in this phase yet!" << std::endl;
        return false;
    }
}

void BPlusTree::Remove(const int &key) {
    // empty
}

void BPlusTree::Print(page_id_t page_id) {
    // empty
}

} // namespace mydb
