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
    CLEAR,
    ECHO,
    UPDATE,
    DESCRIBE,
    SHOW_TABLES,
    HELP,
    SYSTEM,
    DBINFO,
    BACKUP,
    RESTORE,
    RELOAD,
    VERSION,
    CONNECT,
    DROP
};

struct Statement {
    StatementType type;
    std::string table_name;
    // For CREATE
    std::vector<std::pair<std::string, std::string>> columns; // name, type
    // For INSERT
    std::vector<std::string> values;
    
    // For EXPORT/IMPORT/BACKUP/RESTORE
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
        if (!(ss >> word)) return stmt;
        
        // Convert to uppercase for command matching
        std::string cmd = word;
        for (auto &c : cmd) c = std::toupper(c);

        // 1. CREATE / MAKE TABLE
        if (cmd == "CREATE" || cmd == "MAKE") {
            ss >> word; 
            std::string sub = word;
            for (auto &c : sub) c = std::toupper(c);
            if (sub == "TABLE") {
                stmt.type = StatementType::CREATE_TABLE;
                ss >> stmt.table_name;
                SanitizeIdentifier(stmt.table_name);
                
                while (ss >> word) {
                    if (word == "with" || word == "fields" || word == "properties" || word == "(" || word == ")" || word == ",") {
                        continue; 
                    }
                    
                    std::string col_name = word;
                    if (col_name.back() == ',') col_name.pop_back();
                    SanitizeIdentifier(col_name);
                    
                    if (ss >> word) {
                        std::string type = word;
                        if (type.back() == ',') type.pop_back();
                        // Case-insensitive type check
                        std::string type_up = type;
                        for (auto &c : type_up) c = std::toupper(c);

                        if (type_up == "STRING" || type_up == "TEXT" || type_up == "VARCHAR") type = "VARCHAR";
                        else if (type_up == "INT" || type_up == "INTEGER" || type_up == "NUMBER") type = "INT";
                        else if (type == ",") continue; 
                        
                        stmt.columns.emplace_back(col_name, type);
                    }
                }
            }
        } 
        // 2. INSERT / ADD TO
        else if (cmd == "INSERT" || cmd == "ADD" || cmd == "PUT") {
             ss >> word; 
             std::string sub = word;
             for (auto &c : sub) c = std::toupper(c);
             if (sub == "INTO" || sub == "TO" || sub == "IN") {
                 stmt.type = StatementType::INSERT;
                 ss >> stmt.table_name;
                 SanitizeIdentifier(stmt.table_name);
                 
                 while (ss >> word) {
                     std::string val_cmd = word;
                     for (auto &c : val_cmd) c = std::toupper(c);
                     if (val_cmd == "VALUES" || word == "(") continue;
                     if (word == ")") break;
                     
                     if (word.back() == ',') word.pop_back();
                     CleanValue(word);
                     stmt.values.push_back(word);
                 }
             }
        } 
        // 3. SELECT / SHOW ME
        else if (cmd == "SELECT" || cmd == "GET" || cmd == "LIST") {
            if (cmd == "SELECT") {
                ss >> word; // *
                ss >> word; // FROM
                std::string sub = word;
                for (auto &c : sub) c = std::toupper(c);
                if (sub == "FROM") {
                    stmt.type = StatementType::SELECT;
                    ss >> stmt.table_name;
                    if (stmt.table_name.back() == ';') stmt.table_name.pop_back();
                    SanitizeIdentifier(stmt.table_name);
                }
            } else if (cmd == "LIST" || cmd == "GET") {
                ss >> word; 
                std::string next = word;
                for (auto &c : next) c = std::toupper(c);
                if (next == "TABLES") {
                    stmt.type = StatementType::SHOW_TABLES;
                } else {
                    if (next == "ME" || next == "ALL" || next == "FROM") {
                        ss >> stmt.table_name;
                    } else {
                        stmt.table_name = word;
                    }
                    stmt.type = StatementType::SELECT;
                    SanitizeIdentifier(stmt.table_name);
                }
            }
            ParseWhereClause(ss, stmt);
        }
        // 4. SHOW (Legacy/New)
        else if (cmd == "SHOW") {
            ss >> word;
            std::string sub = word;
            for (auto &c : sub) c = std::toupper(c);
            if (sub == "TABLES") {
                stmt.type = StatementType::SHOW_TABLES;
            } else if (sub == "ME") {
                stmt.type = StatementType::SELECT;
                ss >> stmt.table_name;
                SanitizeIdentifier(stmt.table_name);
            } else {
                // assume show <table_name> is select
                stmt.type = StatementType::SELECT;
                stmt.table_name = word;
                SanitizeIdentifier(stmt.table_name);
            }
            ParseWhereClause(ss, stmt);
        }
        // 5. EXPORT
        else if (cmd == "EXPORT") {
            stmt.type = StatementType::EXPORT;
            ss >> stmt.table_name;
            SanitizeIdentifier(stmt.table_name);
            ss >> stmt.file_path;
        }
        // 6. IMPORT
        else if (cmd == "IMPORT") {
            stmt.type = StatementType::IMPORT;
            ss >> stmt.table_name;
            SanitizeIdentifier(stmt.table_name);
            ss >> stmt.file_path;
        }
        }
        // 7. DELETE
        else if (cmd == "DELETE") {
            stmt.type = StatementType::DELETE;
            ss >> word; 
            if (word == "*") ss >> word; // SKIP *
            stmt.table_name = ""; // Reset just in case
            if (word == "FROM" || word == "from") ss >> stmt.table_name;
            else stmt.table_name = word;
            SanitizeIdentifier(stmt.table_name);
            ParseWhereClause(ss, stmt);
        }
        // 7b. CLEAR
        else if (cmd == "CLEAR" || cmd == "TRUNCATE") {
            stmt.type = StatementType::CLEAR;
            ss >> word;
            std::string sub = word;
            for (auto &c : sub) c = std::toupper(c);
            if (sub == "TABLE") ss >> stmt.table_name;
            else stmt.table_name = word;
            SanitizeIdentifier(stmt.table_name);
        }
        // 7c. ECHO
        else if (cmd == "ECHO") {
            stmt.type = StatementType::ECHO;
            std::string remainder;
            std::getline(ss, remainder);
            if (!remainder.empty() && remainder[0] == ' ') remainder = remainder.substr(1);
            stmt.file_path = remainder; // Using file_path to hold the message
        }
        // 8. UPDATE
        else if (cmd == "UPDATE") {
            stmt.type = StatementType::UPDATE;
            ss >> stmt.table_name;
            SanitizeIdentifier(stmt.table_name);
            ss >> word; 
            std::string sub = word;
            for (auto &c : sub) c = std::toupper(c);
            if (sub == "SET") {
                ss >> stmt.update_column;
                SanitizeIdentifier(stmt.update_column);
                ss >> word; // =
                ss >> stmt.update_value;
                CleanValue(stmt.update_value);
                ParseWhereClause(ss, stmt);
            }
        }
        // 9. DESCRIBE
        else if (cmd == "DESCRIBE" || cmd == "DESC") {
            stmt.type = StatementType::DESCRIBE;
            ss >> stmt.table_name;
            SanitizeIdentifier(stmt.table_name);
        }
        // 10. HELP
        else if (cmd == "HELP" || cmd == "?" || cmd == "--HELP" || cmd == "-H") {
            stmt.type = StatementType::HELP;
        }
        // 11. SYSTEM
        else if (cmd == "SYSTEM") {
            stmt.type = StatementType::SYSTEM;
        }
        // 12. DBINFO
        else if (cmd == "DBINFO" || cmd == "STAT") {
            stmt.type = StatementType::DBINFO;
        }
        // 13. BACKUP
        else if (cmd == "BACKUP") {
            stmt.type = StatementType::BACKUP;
            ss >> stmt.file_path;
        }
        // 14. RESTORE
        else if (cmd == "RESTORE") {
            stmt.type = StatementType::RESTORE;
            ss >> stmt.file_path;
        }
        // 15. RELOAD
        else if (cmd == "RELOAD") {
            stmt.type = StatementType::RELOAD;
        }
        // 16. VERSION
        else if (cmd == "VERSION") {
            stmt.type = StatementType::VERSION;
        }
        // 17. CONNECT
        else if (cmd == "CONNECT") {
            stmt.type = StatementType::CONNECT;
            ss >> stmt.file_path; // Using file_path as db basename
        }
        // 18. DROP
        else if (cmd == "DROP") {
            ss >> word;
            std::string sub = word;
            for (auto &c : sub) c = std::toupper(c);
            if (sub == "TABLE") {
                stmt.type = StatementType::DROP;
                ss >> stmt.table_name;
                SanitizeIdentifier(stmt.table_name);
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
