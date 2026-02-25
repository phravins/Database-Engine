#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include "executor/executor.h"
#include "recovery/recovery_manager.h"

namespace mydb {

// External declaration of update checker
void checkAndNotifyUpdate(bool force);

class Shell {
public:
    explicit Shell(Executor* executor) : executor_(executor) {}

    void Run() {
        PrintWelcome();
        
        std::string line;
        while (true) {
            std::cout << "mydb> ";
            if (!std::getline(std::cin, line)) break;
            
            if (line.empty()) continue;
            
            // Convert line to lower case for command matching
            std::string cmd_lower = line;
            for (auto &c : cmd_lower) c = std::tolower(c);
            
            if (cmd_lower == "exit" || cmd_lower == "quit") {
                break;
            }
            
            if (cmd_lower == "update") {
                checkAndNotifyUpdate(true);
                continue;
            }
            
            try {
                // Determine command type for formatting
                // Executor currently prints directly. We need to modify Executor to return results 
                // OR we just wrap Executor output.
                // For Phase 5, let's just let Executor run, but ideally we'd improve Executor to return a Result set.
                // 
                // Let's IMPROVE Executor output by intercepting or just modifying Executor.h?
                // Actually, let's modify Executor to be friendlier or just print better in Executor.
                // 
                // Simplest: Just call Execute.
                executor_->Execute(line);
                
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << std::endl;
            }
        }
        
        std::cout << "Bye." << std::endl;
    }

private:
    void PrintWelcome() {
        std::cout << " __      __ ___  __      __" << std::endl;
        std::cout << " \\ \\    / /|__ \\ \\ \\    / /" << std::endl;
        std::cout << "  \\ \\  / /    ) | \\ \\  / / " << std::endl;
        std::cout << "   \\ \\/ /    / /   \\ \\/ /  " << std::endl;
        std::cout << "    \\__/    /___|   \\__/   " << std::endl;
        std::cout << "V2V Database (mydb) v1.0.6" << std::endl;
        std::cout << "Type 'exit' to quit." << std::endl;
        std::cout << "-----------------------------------" << std::endl;
    }

    Executor* executor_;
};

} // namespace mydb
