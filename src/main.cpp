#include <iostream>
#include "storage/disk_manager.h"
#include "recovery/log_manager.h"
#include "recovery/recovery_manager.h"
#include "catalog/catalog_manager.h"
#include "cli/shell.h"

int main() {
    // 1. Initialize Components
    mydb::DiskManager disk_manager("mydb.db");
    mydb::LogManager log_manager(&disk_manager);
    mydb::RecoveryManager recovery_manager(&log_manager, &disk_manager);
    
    // 2. Run Recovery
    // In Phase 7, we should probably run recovery first, then load catalog.
    // Ideally Catalog is also recoverable via WAL, but keeping them separate for now.
    recovery_manager.ARIES();
    
    // 3. Initialize Executor & Shell
    mydb::Executor executor(&disk_manager);
    
    // 4. Catalog Persistence
    mydb::CatalogManager catalog_manager(&executor);
    catalog_manager.LoadCatalog();
    
    mydb::Shell shell(&executor);
    
    // 5. Run Shell
    shell.Run();
    
    // 6. Save Catalog on Exit
    catalog_manager.SaveCatalog();
    
    return 0;
}
