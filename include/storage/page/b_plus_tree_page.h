#pragma once

#include "common/config.h"
#include <cstring>
#include <iostream>

namespace mydb {

enum class IndexPageType { INVALID_INDEX_PAGE = 0, LEAF_PAGE, INTERNAL_PAGE };

/**
 * Both Internal and Leaf pages inherit from this.
 *
 * Header format (size in bytes):
 * ----------------------------------------------------------------------------
 * | PageType (4) | LSN (4) | CurrentSize (4) | MaxSize (4) | ParentPageId (4) |
 * ----------------------------------------------------------------------------
 */
class BPlusTreePage {
public:
    bool IsLeafPage() const;
    bool IsRootPage() const;
    void SetPageType(IndexPageType page_type);

    int GetSize() const;
    void SetSize(int size);
    void IncreaseSize(int amount);

    int GetMaxSize() const;
    void SetMaxSize(int max_size);
    int GetMinSize() const;

    page_id_t GetParentPageId() const;
    void SetParentPageId(page_id_t parent_page_id);

    page_id_t GetPageId() const;
    void SetPageId(page_id_t page_id);

    void SetLSN(int lsn = 0); // Log Sequence Number for recovery

private:
    // Member variables - mapped to memory
    IndexPageType page_type_;
    int lsn_;
    int size_;
    int max_size_;
    page_id_t parent_page_id_;
    page_id_t page_id_;
};

} // namespace mydb
