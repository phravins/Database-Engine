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

namespace mydb {

// Simple update checker and installer
void checkAndNotifyUpdate(bool force = false) {
    std::string remote_url = "https://raw.githubusercontent.com/phravins/Database-Engine/main/version.json";
    std::string temp_file = "version_check.json";
    std::string current_version = "1.0.7"; // Updated version to 1.0.7

    // Download version file
#ifdef _WIN32
    std::string cmd = "powershell -Command \"Invoke-WebRequest -Uri '" + remote_url + "' -OutFile '" + temp_file + "'\" > NUL 2>&1";
#else
    std::string cmd = "curl -s -L -o " + temp_file + " " + remote_url;
#endif

    int ret = system(cmd.c_str());
    if (ret != 0) return;

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
                    std::cout << "\n\033[1;33m[UPDATE] New version v" << latest_version << " detected! (Current: v" << current_version << ")\033[0m" << std::endl;
                    
                    if (force) {
                        std::cout << "\033[1;36mStarting automatic update...\033[0m" << std::endl;
                        
#ifdef _WIN32
                        std::string bin_url = "https://github.com/phravins/Database-Engine/releases/latest/download/v2vdb-windows.exe";
                        std::cout << "Downloading new binary..." << std::endl;
                        std::string dl_cmd = "powershell -Command \"Invoke-WebRequest -Uri '" + bin_url + "' -OutFile 'v2vdb_next.exe'\"";
                        if (system(dl_cmd.c_str()) == 0) {
                            std::cout << "Binary downloaded. Restarting to apply update..." << std::endl;
                            
                            std::ofstream batch("v2v_updater.bat");
                            batch << "@echo off\n";
                            batch << ":loop\n";
                            batch << "tasklist | find /i \"v2vdb.exe\" > nul\n";
                            batch << "if errorlevel 1 (goto :update)\n";
                            batch << "timeout /t 1 /nobreak > nul\n";
                            batch << "goto :loop\n";
                            batch << ":update\n";
                            batch << "copy /y v2vdb_next.exe v2vdb.exe > nul\n";
                            batch << "del v2vdb_next.exe\n";
                            batch << "start v2vdb.exe\n";
                            batch << "del \"%~f0\"\n";
                            batch.close();
                            
                            system("start /b v2v_updater.bat");
                            exit(0);
                        } else {
                            std::cout << "\033[1;31mDownload failed.\033[0m" << std::endl;
                        }
#else
                        std::cout << "\033[1;33mAuto-update is currently supported on Windows only.\033[0m" << std::endl;
                        std::cout << "\033[1;33mPlease download manually from: https://github.com/phravins/Database-Engine/releases/latest\033[0m" << std::endl;
#endif
                    } else {
                        std::cout << "\033[1;33mType 'update' in the shell to install it automatically.\033[0m\n" << std::endl;
                    }
                } else if (force) {
                    std::cout << "\033[1;32mDatabase is already up to date (v" << current_version << ").\033[0m" << std::endl;
                }
            }
        }
    }
}

void printHelp(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " [options] [basename]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --db <file>    Path to database file (default: v2v-1.db)" << std::endl;
    std::cout << "  --cat <file>   Path to catalog file (default: v2v-1.cat)" << std::endl;
    std::cout << "  --help, -h     Show this help message" << std::endl;
    std::cout << "  [basename]     Legacy support: <basename>.db and <basename>.cat" << std::endl;
}

} // namespace mydb

int main(int argc, char* argv[]) {
    // 0. Parse Arguments
    std::string db_file = "v2v-1.db";
    std::string cat_file = "v2v-1.cat";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--db" && i + 1 < argc) {
            db_file = argv[++i];
        } else if (arg == "--cat" && i + 1 < argc) {
            cat_file = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            mydb::printHelp(argv[0]);
            return 0;
        } else if (i == 1 && arg[0] != '-') {
            // Support legacy positional argument: v2vdb.exe <basename>
            db_file = arg + ".db";
            cat_file = arg + ".cat";
        }
    }

    // 0.5 check for updates
    mydb::checkAndNotifyUpdate();

    std::cout << "Using database: " << db_file << std::endl;

    try {
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
    std::cout << "\033[1;34m-----------------------------------\033[0m" << std::endl;
    std::cout << "       \033[1;35mV2V Database Security\033[0m       " << std::endl;
    std::cout << "\033[1;34m-----------------------------------\033[0m" << std::endl;
    
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
    } catch (const std::exception& e) {
        std::cerr << "\n\033[1;31m[FATAL ERROR]\033[0m " << e.what() << std::endl;
        std::cerr << "Cannot initialize the database. Are you running this in a protected folder?" << std::endl;
        std::cerr << "Try moving the executable to a user folder or use '--db <path>' to specify a valid database location.\n" << std::endl;
        return 1;
    }
    
    return 0;
}
