#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <iterator>
#include <limits>
#include "storage/disk_manager.h"
#include "recovery/log_manager.h"
#include "recovery/recovery_manager.h"
#include "catalog/catalog_manager.h"
#include "cli/shell.h"

// Simple update checker using system commands to avoid dependencies
void checkAndNotifyUpdate() {
    std::string remote_url = "https://raw.githubusercontent.com/phravins/Database-Engine/main/version.json";
    std::string temp_file = "version_check.json";
    std::string current_version = "1.0.5"; // Current version

    // Download file
#ifdef _WIN32
    std::string cmd = "powershell -Command \"Invoke-WebRequest -Uri '" + remote_url + "' -OutFile '" + temp_file + "'\" > NUL 2>&1";
#else
    std::string cmd = "curl -s -L -o " + temp_file + " " + remote_url;
#endif

    int ret = system(cmd.c_str());
    if (ret != 0) {
        // Silently fail if no internet or command missing
        return;
    }

    std::ifstream ifs(temp_file);
    if (ifs.is_open()) {
        std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        ifs.close();
        std::remove(temp_file.c_str());

        // Simple string search for version to avoid JSON dependency
        std::string key = "\"latest_version\": \"";
        size_t pos = content.find(key);
        if (pos != std::string::npos) {
            size_t start = pos + key.length();
            size_t end = content.find("\"", start);
            if (end != std::string::npos) {
                std::string latest_version = content.substr(start, end - start);
                if (latest_version != current_version) {
                    std::cout << "\n\033[1;33m[UPDATE AVAILABLE] v" << latest_version << " is available! (Current: v" << current_version << ")\033[0m" << std::endl;
                    std::cout << "\033[1;33mDownload at: https://github.com/phravins/Database-Engine/releases/latest\033[0m\n" << std::endl;
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    // 0. check for updates
    checkAndNotifyUpdate();

    // 0.5 Parse Arguments
    std::string db_file = "v2v-1.db";
    std::string cat_file = "v2v-1.cat";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--db" && i + 1 < argc) {
            db_file = argv[++i];
        } else if (arg == "--cat" && i + 1 < argc) {
            cat_file = argv[++i];
        } else if (i == 1 && arg[0] != '-') {
            // Support legacy positional argument: v2vdb.exe <basename>
            db_file = arg + ".db";
            cat_file = arg + ".cat";
        }
    }

    std::cout << "Using database: " << db_file << std::endl;

    // 1. Initialize Components
    mydb::DiskManager disk_manager(db_file);
    mydb::LogManager log_manager(&disk_manager);
    mydb::RecoveryManager recovery_manager(&log_manager, &disk_manager);
    
    // 2. Run Recovery
    // In Phase 7, we should probably run recovery first, then load catalog.
    // Ideally Catalog is also recoverable via WAL, but keeping them separate for now.
    recovery_manager.ARIES();
    
    // 3. Initialize Executor & Shell
    mydb::Executor executor(&disk_manager);
    executor.SetFiles(db_file, cat_file);
    
    // 4. Catalog Persistence
    mydb::CatalogManager catalog_manager(&executor, cat_file);
    catalog_manager.LoadCatalog();
    
    mydb::Shell shell(&executor);
    
    // 5. Run Shell
    // Security: Simple Login
    std::string username, password;
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "       V2V Database Security       " << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    
    // Simple 3-try mechanism
    bool authenticated = false;
    for (int i = 0; i < 3; ++i) {
        std::cout << "Username: ";
        std::cin >> username;
        std::cout << "Password: ";
        std::cin >> password;
        
        // Clear newline from buffer
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (username == "admin" && password == "admin") {
            authenticated = true;
            std::cout << "\033[1;32mLogin successful.\033[0m" << std::endl;
            break;
        } else {
             std::cout << "\033[1;31mInvalid credentials. Try again.\033[0m" << std::endl;
        }
    }

    if (authenticated) {
        shell.Run();
    } else {
        std::cout << "\033[1;31mToo many failed attempts. Exiting.\033[0m" << std::endl;
    }
    
    // 6. Save Catalog on Exit
    catalog_manager.SaveCatalog();
    
    return 0;
}
