#pragma once

#include "storage/disk_manager.h"
#include "storage/table_page.h"
#include "catalog/schema.h"
#include <memory>

namespace mydb {

class TableHeap {
public:
    TableHeap(DiskManager* diff_manager, page_id_t first_page_id, const Schema& schema)
        : disk_manager_(diff_manager), first_page_id_(first_page_id), schema_(schema) {
        }
    
    page_id_t GetFirstPageId() const { return first_page_id_; }

    // Create a new table heap (allocates first page)
    static std::unique_ptr<TableHeap> Create(DiskManager* disk_manager, const Schema& schema) {
         page_id_t first_page = 0; 
         // Find a free page (HACK: just use file size / page size for new pages, 
         // but for the very first page of the very first table, we might overwrite 0 if not careful.
         // Let's assume we append to end.)
         int file_size = disk_manager->GetFileSize("mydb.db");
         if (file_size < 0) file_size = 0;
         first_page = file_size / PAGE_SIZE;

         char buf[PAGE_SIZE];
         std::memset(buf, 0, PAGE_SIZE);
         
         TablePage page;
         page.Init(first_page, -1, buf);
         page.InitNewPage();
         disk_manager->WritePage(first_page, buf);
         
         return std::make_unique<TableHeap>(disk_manager, first_page, schema);
    }

    bool InsertTuple(const Tuple& tuple) {
        page_id_t current_page_id = first_page_id_;
        char buf[PAGE_SIZE];
        
        // Loop until inserted
        while (true) {
            disk_manager_->ReadPage(current_page_id, buf);
            TablePage page;
            page.Init(current_page_id, -1, buf);
            
            if (page.InsertTuple(tuple)) {
                disk_manager_->WritePage(current_page_id, buf);
                return true;
            }
            
            page_id_t next = page.GetNextPageId();
            if (next == -1) {
                // Determine new page ID
                int file_size = disk_manager_->GetFileSize("mydb.db");
                page_id_t new_page_id = file_size / PAGE_SIZE;
                
                // Link current to new
                page.SetNextPageId(new_page_id);
                disk_manager_->WritePage(current_page_id, buf);
                
                // Init new page
                char new_buf[PAGE_SIZE];
                std::memset(new_buf, 0, PAGE_SIZE);
                TablePage new_page;
                new_page.Init(new_page_id, current_page_id, new_buf);
                new_page.InitNewPage();
                
                // Retry insert on new page
                new_page.InsertTuple(tuple);
                disk_manager_->WritePage(new_page_id, new_buf);
                return true;
            }
            current_page_id = next;
        }
    }
    
    // Full scan
    std::vector<Tuple> Scan() {
        std::vector<Tuple> results;
        page_id_t current_page_id = first_page_id_;
        char buf[PAGE_SIZE];
        
        while (current_page_id != -1) {
             disk_manager_->ReadPage(current_page_id, buf);
             TablePage page;
             page.Init(current_page_id, -1, buf);
             
             std::vector<Tuple> tuples = page.GetAllTuples(schema_);
             results.insert(results.end(), tuples.begin(), tuples.end());
             
             current_page_id = page.GetNextPageId();
        }
        return results;
    }

private:
    DiskManager* disk_manager_;
    page_id_t first_page_id_;
    Schema schema_;
};

} // namespace mydb
