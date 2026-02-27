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

    // Delete tuples matching where col = val
    // Returns number of deleted tuples
    int Delete(const std::string& col_name, const std::string& op_str, const std::string& val_str) {
        int deleted_count = 0;
        
        // Resolve column index
        int col_idx = -1;
        TypeID col_type = TypeID::INVALID;
        
        for (uint32_t i = 0; i < schema_.GetColumnCount(); ++i) {
            if (schema_.GetColumn(i).GetName() == col_name) {
                col_idx = i;
                col_type = schema_.GetColumn(i).GetType();
                break;
            }
        }
        
        if (col_idx == -1 && !col_name.empty()) {
            return 0; // Column not found
        }

        page_id_t current_page_id = first_page_id_;
        char buf[PAGE_SIZE];
        
        while (current_page_id != -1) {
             disk_manager_->ReadPage(current_page_id, buf);
             TablePage page;
             page.Init(current_page_id, -1, buf);
             
             std::vector<Tuple> tuples = page.GetAllTuples(schema_);
             std::vector<Tuple> remaining_tuples;
             bool changed = false;
             
             for (const auto& tuple : tuples) {
                 bool match = false;
                 if (col_name.empty()) {
                     match = true; 
                 } else {
                     const Value& v = tuple.GetValue(col_idx);
                     if (v.GetTypeId() == TypeID::INTEGER) {
                         if (std::to_string(v.GetAsInteger()) == val_str) match = true;
                     } else {
                         if (v.GetAsString() == val_str) match = true;
                     }
                     if (op_str == "!=") match = !match;
                 }
                 
                 if (match) {
                     deleted_count++;
                     changed = true;
                 } else {
                     remaining_tuples.push_back(std::move(tuple));
                 }
             }
             
             if (changed) {
                 // Rewrite page
                 // Preserve next page id
                 page_id_t next_page_id = page.GetNextPageId();
                 
                 // Clear/Init Page
                 std::memset(buf, 0, PAGE_SIZE);
                 page.Init(current_page_id, -1, buf); // re-init wrapper
                 page.InitNewPage(next_page_id);
                 
                 // Insert remaining
                 for (const auto& t : remaining_tuples) {
                     page.InsertTuple(t);
                 }
                 
                 disk_manager_->WritePage(current_page_id, buf);
             }
             
             current_page_id = page.GetNextPageId();
         }
         
         return deleted_count;
    }
    
    // Update tuples matching where col = val
    // Returns number of updated tuples
    int Update(const std::string& where_col, const std::string& where_op, const std::string& where_val,
               const std::string& set_col, const std::string& set_val) {
        int updated_count = 0;
        
        // Resolve columns
        int where_idx = -1;
        TypeID where_type = TypeID::INVALID;
        
        int set_idx = -1;
        TypeID set_type = TypeID::INVALID;

        for (uint32_t i = 0; i < schema_.GetColumnCount(); ++i) {
            std::string col = schema_.GetColumn(i).GetName();
            if (col == where_col) {
                where_idx = i;
                where_type = schema_.GetColumn(i).GetType();
            }
            if (col == set_col) {
                set_idx = i;
                set_type = schema_.GetColumn(i).GetType();
            }
        }
        
        if (set_idx == -1) return 0; // Target col not found
        // If where_col is empty but val provided, we might have issue. 
        // But if where_col is not empty and not found, return 0.
        if (!where_col.empty() && where_idx == -1) return 0;

        page_id_t current_page_id = first_page_id_;
        char buf[PAGE_SIZE];
         
         while (current_page_id != -1) {
             disk_manager_->ReadPage(current_page_id, buf);
             TablePage page;
             page.Init(current_page_id, -1, buf);
             
             std::vector<Tuple> tuples = page.GetAllTuples(schema_);
             std::vector<Tuple> new_tuples;
             bool changed = false;
             
             for (const auto& tuple : tuples) {
                 bool match = false;
                 if (where_col.empty()) {
                     match = true;
                 } else {
                     const Value& v = tuple.GetValue(where_idx);
                     if (v.GetTypeId() == TypeID::INTEGER) {
                         if (std::to_string(v.GetAsInteger()) == where_val) match = true;
                     } else {
                         if (v.GetAsString() == where_val) match = true;
                     }
                     if (where_op == "!=") match = !match;
                 }
                 
                 if (match) {
                     // Create new tuple with updated value
                     std::vector<Value> vals;
                     for (uint32_t i = 0; i < schema_.GetColumnCount(); ++i) {
                         if (i == (uint32_t)set_idx) {
                             if (set_type == TypeID::INTEGER) {
                                  vals.emplace_back(std::stoi(set_val));
                             } else {
                                  vals.emplace_back(set_val);
                             }
                         } else {
                             vals.push_back(tuple.GetValue(i));
                         }
                     }
                     new_tuples.emplace_back(vals);
                     updated_count++;
                     changed = true;
                 } else {
                     new_tuples.push_back(tuple);
                 }
             }
             
             if (changed) {
                 page_id_t next_page_id = page.GetNextPageId();
                 std::memset(buf, 0, PAGE_SIZE);
                 page.Init(current_page_id, -1, buf);
                 page.InitNewPage(next_page_id);
                 
                 for (const auto& t : new_tuples) {
                     page.InsertTuple(t);
                 }
                 disk_manager_->WritePage(current_page_id, buf);
             }
             
             current_page_id = page.GetNextPageId();
         }
         return updated_count;
    }

private:
    DiskManager* disk_manager_;
    page_id_t first_page_id_;
    Schema schema_;
};

} // namespace mydb
