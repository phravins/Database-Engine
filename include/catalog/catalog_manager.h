#pragma once

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "executor/executor.h"
#include "storage/table_heap.h"
#include "catalog/schema.h"

namespace mydb {

class CatalogManager {
public:
    explicit CatalogManager(Executor* executor) : executor_(executor) {}

    void SaveCatalog() {
        std::ofstream out("mydb.cat", std::ios::binary);
        if (!out.is_open()) return;

        // 1. Write Table Count
        uint32_t count = executor_->tables_.size();
        out.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));

        // 2. Write Each Table
        for (const auto& pair : executor_->tables_) {
            // Name Length + Name
            std::string name = pair.first;
            uint32_t name_len = name.length();
            out.write(reinterpret_cast<const char*>(&name_len), sizeof(uint32_t));
            out.write(name.c_str(), name_len);

            // Root Page ID (TableHeap)
            page_id_t root_page = pair.second->GetFirstPageId();
            out.write(reinterpret_cast<const char*>(&root_page), sizeof(page_id_t));

            // Schema
            const Schema& schema = executor_->schemas_.at(name);
            uint32_t col_count = schema.GetColumnCount();
            out.write(reinterpret_cast<const char*>(&col_count), sizeof(uint32_t));

            for (const auto& col : schema.GetColumns()) {
                // Type
                TypeID type = col.GetType();
                out.write(reinterpret_cast<const char*>(&type), sizeof(TypeID));
                
                // Name
                std::string col_name = col.GetName();
                uint32_t col_name_len = col_name.length();
                out.write(reinterpret_cast<const char*>(&col_name_len), sizeof(uint32_t));
                out.write(col_name.c_str(), col_name_len);
                
                // Offset
                uint32_t offset = col.GetOffset();
                out.write(reinterpret_cast<const char*>(&offset), sizeof(uint32_t));
            }
        }
        out.close();
        std::cout << "Catalog saved to mydb.cat (" << count << " tables)." << std::endl;
    }

    void LoadCatalog() {
        std::ifstream in("mydb.cat", std::ios::binary);
        if (!in.is_open()) return;

        uint32_t count;
        in.read(reinterpret_cast<char*>(&count), sizeof(uint32_t));

        for (uint32_t i = 0; i < count; ++i) {
            // Name
            uint32_t name_len;
            in.read(reinterpret_cast<char*>(&name_len), sizeof(uint32_t));
            std::string name(name_len, '\0');
            in.read(&name[0], name_len);

            // Root Page ID
            page_id_t root_page;
            in.read(reinterpret_cast<char*>(&root_page), sizeof(page_id_t));

            // Schema
            uint32_t col_count;
            in.read(reinterpret_cast<char*>(&col_count), sizeof(uint32_t));
            std::vector<Column> cols;
            for (uint32_t c = 0; c < col_count; ++c) {
                TypeID type;
                in.read(reinterpret_cast<char*>(&type), sizeof(TypeID));
                
                uint32_t col_name_len;
                in.read(reinterpret_cast<char*>(&col_name_len), sizeof(uint32_t));
                std::string col_name(col_name_len, '\0');
                in.read(&col_name[0], col_name_len);
                
                uint32_t offset;
                in.read(reinterpret_cast<char*>(&offset), sizeof(uint32_t));
                
                cols.emplace_back(col_name, type, offset);
            }
            
            Schema schema(cols);
            
            // Reconstruct TableHeap
            // We need a TableHeap constructor that takes an existing root page!
            // Currently TableHeap::Create makes a new one.
            // Let's assume we can make a TableHeap(disk_manager, root_page) constructor.
            // Or we check if TableHeap allows restoring.
            // TableHeap(DiskManager* dm, page_id_t first_page_id) constructor exists? 
            // Phase 2 implementation check needed.
            
            // Assuming we add/have TableHeap(dm, first_page)
            auto heap = std::make_unique<TableHeap>(executor_->disk_manager_, root_page, schema);
            
            executor_->tables_.emplace(name, std::move(heap));
            executor_->schemas_.emplace(name, schema);
        }
        in.close();
        std::cout << "Catalog loaded (" << count << " tables)." << std::endl;
    }

private:
    Executor* executor_;
};

} // namespace mydb
