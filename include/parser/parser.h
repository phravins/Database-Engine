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
    SELECT
};

struct Statement {
    StatementType type;
    std::string table_name;
    // For CREATE
    std::vector<std::pair<std::string, std::string>> columns; // name, type
    // For INSERT
    std::vector<std::string> values;
};

class Parser {
public:
    static Statement Parse(const std::string& sql) {
        Statement stmt;
        stmt.type = StatementType::INVALID;
        
        std::stringstream ss(sql);
        std::string word;
        ss >> word;
        
        if (word == "CREATE") {
            ss >> word; // TABLE
            if (word == "TABLE") {
                stmt.type = StatementType::CREATE_TABLE;
                ss >> stmt.table_name;
                
                // Hacky parsing: (col1 INT, col2 VARCHAR)
                // Just assume user types: CREATE TABLE name col1 INT col2 VARCHAR
                while (ss >> word) {
                    std::string type;
                    ss >> type;
                    stmt.columns.emplace_back(word, type);
                }
            }
        } else if (word == "INSERT") {
             ss >> word; // INTO
             if (word == "INTO") {
                 stmt.type = StatementType::INSERT;
                 ss >> stmt.table_name;
                 ss >> word; // VALUES
                 // VALUES (1, "a") -> assume simple space separated: INSERT INTO name VALUES 1 "a"
                 while (ss >> word) {
                     // Remove quotes if present
                     if (word.front() == '"') word = word.substr(1);
                     if (word.back() == '"') word.pop_back();
                     stmt.values.push_back(word);
                 }
             }
        } else if (word == "SELECT") {
            ss >> word; // *
            ss >> word; // FROM
            if (word == "FROM") {
                stmt.type = StatementType::SELECT;
                ss >> stmt.table_name;
            }
        }
        
        return stmt;
    }
};

} // namespace mydb
