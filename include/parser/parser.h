#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>

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
    TIME,
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
    DROP,
    AUTOUPDATE
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
    
    // For WHERE clause
    std::string where_column;
    std::string where_op = "="; // Default, can be "!="
    std::string where_value;
    
    // For ORDER BY clause
    std::string order_by_column;
    
    // For UPDATE (SET col = val)
    std::string update_column;
    std::string update_value;
    
    // For VECTOR_DIST
    bool order_by_vector_dist = false;
    std::string order_by_vector_literal;
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
                        else if (type_up == "VECTOR") type = "VECTOR";
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
                     
                     // Handle vector literals starting with [
                     if (word.front() == '[') {
                         while (word.find(']') == std::string::npos && !ss.eof()) {
                             std::string next_word;
                             ss >> next_word;
                             word += " " + next_word;
                         }
                     }
                     
                     if (word.back() == ',') word.pop_back();
                     CleanValue(word);
                     if (!word.empty()) {
                         stmt.values.push_back(word);
                     }
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
            // Eat remaining part of SQL for robust Where/Order parsing
            std::string remainder;
            std::getline(ss, remainder);
            ParseWhereClause(remainder, stmt);
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
            std::string remainder;
            std::getline(ss, remainder);
            ParseWhereClause(remainder, stmt);
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
        // 7. DELETE
        else if (cmd == "DELETE") {
            stmt.type = StatementType::DELETE;
            ss >> word; 
            if (word == "*") ss >> word; // SKIP *
            stmt.table_name = ""; // Reset just in case
            if (word == "FROM" || word == "from") ss >> stmt.table_name;
            else stmt.table_name = word;
            SanitizeIdentifier(stmt.table_name);
            std::string remainder;
            std::getline(ss, remainder);
            ParseWhereClause(remainder, stmt);
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
        // 7d. TIME
        else if (cmd == "TIME" || cmd == "NOW") {
            stmt.type = StatementType::TIME;
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
                std::string remainder;
                std::getline(ss, remainder);
                ParseWhereClause(remainder, stmt);
            }
        }
        // 9. DESCRIBE
        else if (cmd == "DESCRIBE" || cmd == "DESC") {
            stmt.type = StatementType::DESCRIBE;
            ss >> word;
            std::string sub = word;
            for (auto &c : sub) c = std::toupper(c);
            
            if (sub == "ALL") {
                stmt.table_name = "ALL";
            } else {
                stmt.table_name = word;
                SanitizeIdentifier(stmt.table_name);
            }
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
        // 19. AUTOUPDATE
        else if (cmd == "AUTOUPDATE") {
            stmt.type = StatementType::AUTOUPDATE;
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
        if (val.empty()) return;
        if (val.front() == '"' || val.front() == '\'') val = val.substr(1);
        if (!val.empty() && (val.back() == '"' || val.back() == '\'')) val.pop_back();
        if (!val.empty() && val.back() == ';') val.pop_back();
    }

    static void ParseWhereClause(const std::string& clause, Statement& stmt) {
        if (clause.empty()) return;
        
        // We'll process WHERE and ORDER BY
        std::string upper_clause = clause;
        std::transform(upper_clause.begin(), upper_clause.end(), upper_clause.begin(), ::toupper);
        
        size_t where_pos = upper_clause.find("WHERE ");
        size_t order_pos = upper_clause.find("ORDER BY ");
        
        if (where_pos != std::string::npos) {
            std::string where_part = clause.substr(where_pos + 6);
            if (order_pos != std::string::npos && order_pos > where_pos) {
                where_part = clause.substr(where_pos + 6, order_pos - (where_pos + 6));
            }
            std::stringstream wss(where_part);
            wss >> stmt.where_column;
            SanitizeIdentifier(stmt.where_column);
            wss >> stmt.where_op;
            if (stmt.where_op == "=" || stmt.where_op == "!=") {
                wss >> stmt.where_value;
                CleanValue(stmt.where_value);
            }
        }
        
        if (order_pos != std::string::npos) {
            std::string order_part = clause.substr(order_pos + 9);
            // Handle vector_dist(col, [vals])
            std::string upper_order = order_part;
            std::transform(upper_order.begin(), upper_order.end(), upper_order.begin(), ::toupper);
            
            size_t dist_pos = upper_order.find("VECTOR_DIST");
            if (dist_pos != std::string::npos) {
                size_t open_paren = order_part.find('(', dist_pos);
                size_t close_paren = order_part.find(')', dist_pos);
                if (open_paren != std::string::npos && close_paren != std::string::npos) {
                    std::string args = order_part.substr(open_paren + 1, close_paren - open_paren - 1);
                    size_t comma = args.find(',');
                    if (comma != std::string::npos) {
                        stmt.order_by_vector_dist = true;
                        stmt.order_by_column = args.substr(0, comma);
                        // trim spaces
                        stmt.order_by_column.erase(0, stmt.order_by_column.find_first_not_of(" \t\r\n"));
                        stmt.order_by_column.erase(stmt.order_by_column.find_last_not_of(" \t\r\n") + 1);
                        SanitizeIdentifier(stmt.order_by_column);
                        
                        stmt.order_by_vector_literal = args.substr(comma + 1);
                        // trim spaces
                        stmt.order_by_vector_literal.erase(0, stmt.order_by_vector_literal.find_first_not_of(" \t\r\n"));
                        stmt.order_by_vector_literal.erase(stmt.order_by_vector_literal.find_last_not_of(" \t\r\n") + 1);
                    }
                }
            } else {
                std::stringstream oss(order_part);
                oss >> stmt.order_by_column;
                SanitizeIdentifier(stmt.order_by_column);
            }
        }
    }
};

} // namespace mydb
