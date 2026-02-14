#pragma once

#include "parser/parser.h"
#include "storage/table_heap.h"
#include <map>
#include <iostream>
#include <iomanip>
#include <vector>

namespace mydb {

class Executor {
    friend class CatalogManager;
public:
    Executor(DiskManager* dm) : disk_manager_(dm) {}

    void Init() {
        // Load catalog if existed (not implemented yet)
    }

    void Execute(const std::string& sql) {
        Statement stmt = Parser::Parse(sql);
        
        if (stmt.type == StatementType::CREATE_TABLE) {
            HandleCreate(stmt);
        } else if (stmt.type == StatementType::INSERT) {
            HandleInsert(stmt);
        } else if (stmt.type == StatementType::SELECT) {
            HandleSelect(stmt);
        } else {
            std::cout << "Unknown command." << std::endl;
        }
    }

private:
    void HandleCreate(const Statement& stmt) {
        if (tables_.find(stmt.table_name) != tables_.end()) {
            std::cout << "Table already exists." << std::endl;
            return;
        }
        
        std::vector<Column> cols;
        uint32_t offset = 0;
        for (const auto& pair : stmt.columns) {
            TypeID type = (pair.second == "INT") ? TypeID::INTEGER : TypeID::VARCHAR;
            cols.emplace_back(pair.first, type, offset);
            // offset update is dummy for now
        }
        
        Schema schema(cols);
        auto heap = TableHeap::Create(disk_manager_, schema);
        
        // We leak heap pointer here in this simple version (should be unique_ptr in map)
        // Converting unique_ptr to raw for map storage is messy, let's keep it simple.
        tables_.emplace(stmt.table_name, std::move(heap));
        schemas_.emplace(stmt.table_name, schema);
        
        std::cout << "Table " << stmt.table_name << " created." << std::endl;
    }
    
    void HandleInsert(const Statement& stmt) {
         if (tables_.find(stmt.table_name) == tables_.end()) {
            std::cout << "Table not found." << std::endl;
            return;
        }
        
        TableHeap* table = tables_[stmt.table_name].get();
        Schema& schema = schemas_.at(stmt.table_name);
        
        std::vector<Value> values;
        for (size_t i = 0; i < stmt.values.size(); ++i) {
             const Column& col = schema.GetColumn(i);
             if (col.GetType() == TypeID::INTEGER) {
                 values.emplace_back(std::stoi(stmt.values[i]));
             } else {
                 values.emplace_back(stmt.values[i]);
             }
        }
        
        Tuple tuple(values);
        if (table->InsertTuple(tuple)) {
            std::cout << "Inserted 1 row." << std::endl;
        } else {
             std::cout << "Failed to insert." << std::endl;
        }
    }
    
    void HandleSelect(const Statement& stmt) {
        if (tables_.find(stmt.table_name) == tables_.end()) {
            std::cout << "Error: Table '" << stmt.table_name << "' not found." << std::endl;
            return;
        }
        
        TableHeap* table = tables_[stmt.table_name].get();
        std::vector<Tuple> tuples = table->Scan();
        Schema& schema = schemas_.at(stmt.table_name);
        
        if (tuples.empty()) {
            std::cout << "(0 rows)" << std::endl;
            return;
        }

        // Calculate column widths
        std::vector<int> col_widths;
        for (const auto& col : schema.GetColumns()) {
            col_widths.push_back(std::max((int)col.GetName().length(), 10)); // Min 10
        }
        
        // Check data width (simple scan)
        for (const auto& tuple : tuples) {
             for (uint32_t c = 0; c < schema.GetColumnCount(); ++c) {
                 int len = 0;
                 const Value& val = tuple.GetValue(c);
                 if (val.GetTypeId() == TypeID::INTEGER) {
                     len = std::to_string(val.GetAsInteger()).length();
                 } else {
                     len = val.GetAsString().length();
                 }
                 if (len > col_widths[c]) col_widths[c] = len;
             }
        }

        // Print Header
        auto PrintLine = [&](const std::vector<int>& widths) {
             std::cout << "+";
             for (int w : widths) {
                 std::cout << std::string(w + 2, '-') << "+";
             }
             std::cout << std::endl;
        };
        
        PrintLine(col_widths);
        std::cout << "|";
        for (uint32_t i = 0; i < schema.GetColumnCount(); ++i) {
            std::cout << " " << std::left << std::setw(col_widths[i]) << schema.GetColumn(i).GetName() << " |";
        }
        std::cout << std::endl;
        PrintLine(col_widths);
        
        // Print Rows
        for (const auto& tuple : tuples) {
             std::cout << "|";
             for (uint32_t c = 0; c < schema.GetColumnCount(); ++c) {
                 const Value& val = tuple.GetValue(c);
                 std::cout << " ";
                 if (val.GetTypeId() == TypeID::INTEGER) {
                     std::cout << std::left << std::setw(col_widths[c]) << val.GetAsInteger();
                 } else {
                     std::cout << std::left << std::setw(col_widths[c]) << val.GetAsString();
                 }
                 std::cout << " |";
             }
             std::cout << std::endl;
        }
        PrintLine(col_widths);
        std::cout << "(" << tuples.size() << " rows)" << std::endl;
    }

    DiskManager* disk_manager_;
    std::map<std::string, std::unique_ptr<TableHeap>> tables_;
    std::map<std::string, Schema> schemas_;
};

} // namespace mydb
