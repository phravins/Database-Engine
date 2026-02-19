#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

namespace mydb {

enum class StatementType {
    INVALID,
    CREATE_TABLE,
    INSERT,
    SELECT,
    EXPORT,
    IMPORT,
    DELETE,
    UPDATE
};

struct Statement {
    StatementType type;
    std::string table_name;
    // For CREATE
    std::vector<std::pair<std::string, std::string>> columns; // name, type
    // For INSERT
    std::vector<std::string> values;
    
    // For EXPORT/IMPORT
    std::string file_path;
    
    // For WHERE clause (simple: col = val)
    std::string where_column;
    std::string where_value;
    
    // For UPDATE (SET col = val)
    std::string update_column;
    std::string update_value;
};

class Parser {
public:
    static Statement Parse(const std::string& sql) {
        Statement stmt;
        stmt.type = StatementType::INVALID;
        
        std::stringstream ss(sql);
        std::string word;
        ss >> word;
        
        // 1. CREATE / MAKE TABLE
        if (word == "CREATE" || word == "make" || word == "create") {
            ss >> word; // TABLE or table
            if (word == "TABLE" || word == "table") {
                stmt.type = StatementType::CREATE_TABLE;
                ss >> stmt.table_name;
                SanitizeIdentifier(stmt.table_name);
                
                while (ss >> word) {
                    if (word == "with" || word == "fields" || word == "properties" || word == "(" || word == ")" || word == ",") {
                        continue; // Skip noise words
                    }
                    
                    std::string col_name = word;
                    SanitizeIdentifier(col_name);
                    
                    if (ss >> word) {
                        std::string type = word;
                        // Type Aliases
                        if (type == "string" || type == "text" || type == "std::string") type = "VARCHAR";
                        if (type == "int" || type == "integer" || type == "num" || type == "number") type = "INT";
                        if (type == ",") continue; 
                        
                        stmt.columns.emplace_back(col_name, type);
                    }
                }
            }
        } 
        // 2. INSERT / ADD TO
        else if (word == "INSERT" || word == "add" || word == "insert" || word == "put") {
             ss >> word; // INTO or to
             if (word == "INTO" || word == "to" || word == "in") {
                 stmt.type = StatementType::INSERT;
                 ss >> stmt.table_name;
                 SanitizeIdentifier(stmt.table_name);
                 
                 // consume "values" if present
                 while (ss >> word) {
                     if (word == "VALUES" || word == "values" || word == "(") continue;
                     if (word == ")") break;
                     
                     // Remove quotes/commas
                     if (word.back() == ',') word.pop_back();
                     if (word.front() == '"' || word.front() == '\'') word = word.substr(1);
                     if (word.back() == '"' || word.back() == '\'') word.pop_back();
                     
                     stmt.values.push_back(word);
                 }
             }
        } 
        // 3. SELECT / SHOW ME
        else if (word == "SELECT" || word == "show" || word == "list" || word == "get") {
            if (word == "SELECT") {
                ss >> word; // *
                ss >> word; // FROM
                if (word == "FROM") {
                    stmt.type = StatementType::SELECT;
                    ss >> stmt.table_name;
                    // Remove semicolon if present
                    if (stmt.table_name.back() == ';') stmt.table_name.pop_back();
                    SanitizeIdentifier(stmt.table_name);
                }
            } else {
                ss >> word; // "me" or table name
                if (word == "me" || word == "all" || word == "from") {
                     ss >> stmt.table_name;
                } else {
                     stmt.table_name = word;
                }
                stmt.type = StatementType::SELECT;
                SanitizeIdentifier(stmt.table_name);
            }
            
            // Check for WHERE clause
            ParseWhereClause(ss, stmt);
        }
        // 4. EXPORT
        else if (word == "EXPORT" || word == "export") {
            stmt.type = StatementType::EXPORT;
            ss >> stmt.table_name;
            SanitizeIdentifier(stmt.table_name);
            ss >> stmt.file_path;
        }
        // 5. IMPORT
        else if (word == "IMPORT" || word == "import") {
            stmt.type = StatementType::IMPORT;
            ss >> stmt.table_name;
            SanitizeIdentifier(stmt.table_name);
            ss >> stmt.file_path;
        }
        // 6. DELETE
        else if (word == "DELETE" || word == "delete") {
            stmt.type = StatementType::DELETE;
            ss >> word; // FROM
            ss >> stmt.table_name;
            SanitizeIdentifier(stmt.table_name);
            ParseWhereClause(ss, stmt);
        }
        // 7. UPDATE
        else if (word == "UPDATE" || word == "update") {
            stmt.type = StatementType::UPDATE;
            ss >> stmt.table_name;
            SanitizeIdentifier(stmt.table_name);
            ss >> word; // SET
            if (word == "SET" || word == "set") {
                ss >> stmt.update_column;
                SanitizeIdentifier(stmt.update_column);
                ss >> word; // =
                ss >> stmt.update_value;
                CleanValue(stmt.update_value);
                ParseWhereClause(ss, stmt);
            }
        }
        
        return stmt;
    }

private:
    // Security: Auto Removal of Vulnerability
    static void SanitizeIdentifier(std::string& id) {
        std::string safe;
        for (char c : id) {
            if (isalnum(c) || c == '_') {
                safe += c;
            }
        }
        if (safe != id) {
            // Optional: Warn user? For now just silent fix as per "Auto Removal" request
            // std::cout << "Warning: Invalid characters removed from identifier '" << id << "' -> '" << safe << "'" << std::endl;
        }
        id = safe;
    }

    static void CleanValue(std::string& val) {
        if (val.front() == '"' || val.front() == '\'') val = val.substr(1);
        if (val.back() == '"' || val.back() == '\'') val.pop_back();
        if (val.back() == ';') val.pop_back();
    }

    static void ParseWhereClause(std::stringstream& ss, Statement& stmt) {
        std::string word;
        while (ss >> word) {
            if (word == "WHERE" || word == "where") {
                ss >> stmt.where_column;
                SanitizeIdentifier(stmt.where_column);
                ss >> word; // =
                if (word == "=") {
                   ss >> stmt.where_value;
                   CleanValue(stmt.where_value);
                }
            }
        }
    }
};

} // namespace mydb
