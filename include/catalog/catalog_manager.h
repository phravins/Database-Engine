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
    CatalogManager(Executor* executor, std::string catalog_file) 
        : executor_(executor), catalog_file_(std::move(catalog_file)) {}

    void SaveCatalog() {
        std::ofstream out(catalog_file_);
        if (!out.is_open()) return;

        out << "V2VDB_CATALOG_V1" << std::endl;
        out << "TABLE_COUNT " << executor_->tables_.size() << std::endl;

        for (const auto& pair : executor_->tables_) {
            const std::string& name = pair.first;
            page_id_t root_page = pair.second->GetFirstPageId();
            const Schema& schema = executor_->schemas_.at(name);

            out << "TABLE " << name << std::endl;
            out << "ROOT " << root_page << std::endl;
            out << "COLUMNS " << schema.GetColumnCount() << std::endl;

            for (const auto& col : schema.GetColumns()) {
                // We use static_cast to int for TypeID
                out << "COLUMN " << col.GetName() << " " << static_cast<int>(col.GetType()) << " " << col.GetOffset() << std::endl;
            }
        }
        out.close();
        std::cout << "\033[1;32mCatalog saved to " << catalog_file_ << " (Readable text format).\033[0m" << std::endl;
    }

    void LoadCatalog() {
        std::ifstream in(catalog_file_);
        if (!in.is_open()) return;

        std::string line;
        if (!(in >> line) || line != "V2VDB_CATALOG_V1") {
             // Fallback for old binary catalog if needed? Or just error.
             // Given this is a dev task, let's assume we start fresh or user is fine with it.
             return;
        }

        std::string key;
        uint32_t table_count = 0;
        in >> key >> table_count;

        for (uint32_t i = 0; i < table_count; ++i) {
            std::string table_name;
            page_id_t root_page;
            uint32_t col_count;

            in >> key >> table_name; // TABLE name
            in >> key >> root_page;  // ROOT root_page
            in >> key >> col_count; // COLUMNS col_count

            std::vector<Column> cols;
            for (uint32_t c = 0; c < col_count; ++c) {
                std::string col_name;
                int type_int;
                uint32_t offset;
                in >> key >> col_name >> type_int >> offset; // COLUMN name type offset
                cols.emplace_back(col_name, static_cast<TypeID>(type_int), offset);
            }

            Schema schema(cols);
            auto heap = std::make_unique<TableHeap>(executor_->disk_manager_, root_page, schema);
            executor_->tables_.emplace(table_name, std::move(heap));
            executor_->schemas_.emplace(table_name, schema);
        }
        in.close();
        std::cout << "Catalog loaded from " << catalog_file_ << " (" << table_count << " tables)." << std::endl;
    }

private:
    Executor* executor_;
    std::string catalog_file_;
};

} // namespace mydb
