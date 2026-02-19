#pragma once

#include "parser/parser.h"
#include "storage/table_heap.h"
#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

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
        } else if (stmt.type == StatementType::EXPORT) {
            HandleExport(stmt);
        } else if (stmt.type == StatementType::IMPORT) {
            HandleImport(stmt);
        } else if (stmt.type == StatementType::DELETE) {
            HandleDelete(stmt);
        } else if (stmt.type == StatementType::UPDATE) {
            HandleUpdate(stmt);
        } else {
            std::cout << "\033[1;31mUnknown command.\033[0m" << std::endl;
        }
    }

private:
    void HandleCreate(const Statement& stmt) {
        if (tables_.find(stmt.table_name) != tables_.end()) {
            std::cout << "\033[1;31mTable already exists.\033[0m" << std::endl;
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
        
        std::cout << "\033[1;32mTable " << stmt.table_name << " created.\033[0m" << std::endl;
    }
    
    void HandleInsert(const Statement& stmt) {
         if (tables_.find(stmt.table_name) == tables_.end()) {
            std::cout << "\033[1;31mTable not found.\033[0m" << std::endl;
            return;
        }
        
        TableHeap* table = tables_[stmt.table_name].get();
        Schema& schema = schemas_.at(stmt.table_name);
        
        // Validation: Column Count
        if (stmt.values.size() != schema.GetColumnCount()) {
            std::cout << "\033[1;31mError: Column count mismatch. Expected " 
                      << schema.GetColumnCount() << ", got " << stmt.values.size() << ".\033[0m" << std::endl;
            return;
        }
        
        std::vector<Value> values;
        try {
            for (size_t i = 0; i < stmt.values.size(); ++i) {
                 const Column& col = schema.GetColumn(i);
                 if (col.GetType() == TypeID::INTEGER) {
                     // Safe conversion
                     size_t pos;
                     int val = std::stoi(stmt.values[i], &pos);
                     if (pos != stmt.values[i].length()) {
                          throw std::invalid_argument("Not a valid integer");
                     }
                     values.emplace_back(val);
                 } else {
                     values.emplace_back(stmt.values[i]);
                 }
            }
        
            Tuple tuple(values);
            if (table->InsertTuple(tuple)) {
                std::cout << "\033[1;32mInserted 1 row.\033[0m" << std::endl;
            } else {
                 std::cout << "\033[1;31mFailed to insert.\033[0m" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "\033[1;31mError processing values: " << e.what() << "\033[0m" << std::endl;
        }
    }
    
    void HandleSelect(const Statement& stmt) {
        if (tables_.find(stmt.table_name) == tables_.end()) {
            std::cout << "\033[1;31mError: Table '" << stmt.table_name << "' not found.\033[0m" << std::endl;
            return;
        }
        
        TableHeap* table = tables_[stmt.table_name].get();
        std::vector<Tuple> tuples = table->Scan();
        Schema& schema = schemas_.at(stmt.table_name);
        
        // Filter tuples if WHERE clause exists
        std::vector<Tuple> filtered_tuples;
        if (!stmt.where_column.empty()) {
            int col_idx = -1;
            for (uint32_t i = 0; i < schema.GetColumnCount(); ++i) {
                if (schema.GetColumn(i).GetName() == stmt.where_column) {
                    col_idx = i;
                    break;
                }
            }
            
            if (col_idx == -1) {
                 std::cout << "\033[1;31mError: Column '" << stmt.where_column << "' not found.\033[0m" << std::endl;
                 return;
            }
            
            for (const auto& tuple : tuples) {
                const Value& val = tuple.GetValue(col_idx);
                bool match = false;
                if (val.GetTypeId() == TypeID::INTEGER) {
                    if (std::to_string(val.GetAsInteger()) == stmt.where_value) match = true;
                } else {
                    if (val.GetAsString() == stmt.where_value) match = true;
                }
                
                if (match) {
                    filtered_tuples.push_back(tuple);
                }
            }
        } else {
            filtered_tuples = tuples;
        }
        
        if (filtered_tuples.empty()) {
            std::cout << "(0 rows)" << std::endl;
            return;
        }

        // Calculate column widths
        std::vector<int> col_widths;
        for (const auto& col : schema.GetColumns()) {
            col_widths.push_back(std::max((int)col.GetName().length(), 10)); // Min 10
        }
        
        // Check data width (simple scan)
        for (const auto& tuple : filtered_tuples) {
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
            std::cout << "\033[1;33m " << std::left << std::setw(col_widths[i]) << schema.GetColumn(i).GetName() << " \033[0m|";
        }
        std::cout << std::endl;
        PrintLine(col_widths);
        
        // Print Rows
        for (const auto& tuple : filtered_tuples) {
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
        std::cout << "(" << filtered_tuples.size() << " rows)" << std::endl;
    }
    
    void HandleExport(const Statement& stmt) {
        if (tables_.find(stmt.table_name) == tables_.end()) {
            std::cout << "\033[1;31mError: Table '" << stmt.table_name << "' not found.\033[0m" << std::endl;
            return;
        }

        // Security: Path Traversal Check
        if (stmt.file_path.find("..") != std::string::npos || 
            stmt.file_path.find("/") != std::string::npos || 
            stmt.file_path.find("\\") != std::string::npos) {
             std::cout << "\033[1;31mSecurity Error: File path cannot contain path separators or '..'.\033[0m" << std::endl;
             std::cout << "\033[1;31mExport restricted to current directory for security.\033[0m" << std::endl;
             return;
        }
        
        TableHeap* table = tables_[stmt.table_name].get();
        std::vector<Tuple> tuples = table->Scan();
        Schema& schema = schemas_.at(stmt.table_name);
        
        std::ofstream outfile(stmt.file_path);
        if (!outfile.is_open()) {
            std::cout << "\033[1;31mError: Could not open file '" << stmt.file_path << "' for writing.\033[0m" << std::endl;
            return;
        }
        
        // Write Header
        for (uint32_t i = 0; i < schema.GetColumnCount(); ++i) {
            outfile << schema.GetColumn(i).GetName();
            if (i < schema.GetColumnCount() - 1) outfile << ",";
        }
        outfile << "\n";
        
        // Write Rows
        for (const auto& tuple : tuples) {
            for (uint32_t i = 0; i < schema.GetColumnCount(); ++i) {
                const Value& val = tuple.GetValue(i);
                if (val.GetTypeId() == TypeID::INTEGER) {
                    outfile << val.GetAsInteger();
                } else {
                    outfile << val.GetAsString();
                }
                if (i < schema.GetColumnCount() - 1) outfile << ",";
            }
            outfile << "\n";
        }
        
        outfile.close();
        std::cout << "\033[1;32mExported " << tuples.size() << " rows to " << stmt.file_path << ".\033[0m" << std::endl;
    }

    void HandleImport(const Statement& stmt) {
         if (tables_.find(stmt.table_name) == tables_.end()) {
            std::cout << "\033[1;31mError: Table '" << stmt.table_name << "' not found. Create it first.\033[0m" << std::endl;
            return;
        }

        // Security: Path Traversal Check
        if (stmt.file_path.find("..") != std::string::npos || 
            stmt.file_path.find("/") != std::string::npos || 
            stmt.file_path.find("\\") != std::string::npos) {
             std::cout << "\033[1;31mSecurity Error: File path cannot contain path separators or '..'.\033[0m" << std::endl;
             std::cout << "\033[1;31mImport restricted to current directory for security.\033[0m" << std::endl;
             return;
        }
        
        TableHeap* table = tables_[stmt.table_name].get();
        Schema& schema = schemas_.at(stmt.table_name);
        
        std::ifstream infile(stmt.file_path);
        if (!infile.is_open()) {
            std::cout << "\033[1;31mError: Could not open file '" << stmt.file_path << "' for reading.\033[0m" << std::endl;
            return;
        }
        
        std::string line;
        // Skip header
        std::getline(infile, line);
        
        int imported_count = 0;
        int line_num = 1;
        while (std::getline(infile, line)) {
            line_num++;
            std::stringstream ss(line);
            std::string item;
            std::vector<Value> values;
            
            for (uint32_t i = 0; i < schema.GetColumnCount(); ++i) {
                if (!std::getline(ss, item, ',')) {
                    break;
                }
                
                // Clean item (remove \r if on windows/linux mix)
                if (!item.empty() && item.back() == '\r') item.pop_back();

                const Column& col = schema.GetColumn(i);
                if (col.GetType() == TypeID::INTEGER) {
                     try {
                         values.emplace_back(std::stoi(item));
                     } catch (...) {
                         std::cout << "Warning: Invalid int at line " << line_num << ", col " << i + 1 << ". Using 0." << std::endl;
                         values.emplace_back(0);
                     }
                } else {
                    values.emplace_back(item);
                }
            }
            
            if (values.size() != schema.GetColumnCount()) {
                std::cout << "Warning: Line " << line_num << " has " << values.size() << " columns, expected " << schema.GetColumnCount() << ". Skipping." << std::endl;
                continue;
            }
            
            Tuple tuple(values);
            if (table->InsertTuple(tuple)) {
                imported_count++;
            }
        }
        
        infile.close();
        std::cout << "\033[1;32mImported " << imported_count << " rows from " << stmt.file_path << ".\033[0m" << std::endl;
    }
    
    void HandleDelete(const Statement& stmt) {
        if (tables_.find(stmt.table_name) == tables_.end()) {
            std::cout << "\033[1;31mError: Table '" << stmt.table_name << "' not found.\033[0m" << std::endl;
            return;
        }
        
        TableHeap* table = tables_[stmt.table_name].get();
        int count = table->Delete(stmt.where_column, stmt.where_value);
        std::cout << "\033[1;32mDeleted " << count << " rows.\033[0m" << std::endl;
    }
    
    void HandleUpdate(const Statement& stmt) {
        if (tables_.find(stmt.table_name) == tables_.end()) {
            std::cout << "\033[1;31mError: Table '" << stmt.table_name << "' not found.\033[0m" << std::endl;
            return;
        }
        
        TableHeap* table = tables_[stmt.table_name].get();
        int count = table->Update(stmt.where_column, stmt.where_value, stmt.update_column, stmt.update_value);
        std::cout << "\033[1;32mUpdated " << count << " rows.\033[0m" << std::endl;
    }

    DiskManager* disk_manager_;
    std::map<std::string, std::unique_ptr<TableHeap>> tables_;
    std::map<std::string, Schema> schemas_;
};

} // namespace mydb
