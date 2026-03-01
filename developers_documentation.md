# V2V Database: Developer & Contributor Documentation

Welcome to the internal engineering guide for the **V2V Database Engine**! This document is designed for C++ developers, open-source contributors, and students who want to understand the deeply integrated theoretical concepts and core source code of `v2vdb`.

---

## System Architecture Overview

The V2V database is an educational, disk-backed relational database written entirely from scratch in **C++17**. It avoids using standard libraries for storage (like SQLite) and implements a 4-tier relational database architecture natively:

1.  **Storage Manager (`src/storage`)**: Handles raw disk I/O, writing 4KB blocks, and Slotted Page layouts.
2.  **Access Methods (`src/index`)**: Implements B+ Trees for O(log n) index lookups.
3.  **Execution Engine (`src/executor`, `src/parser`)**: Parses queries (SQL/V2V Language), validates types, and orchestrates operations.
4.  **Catalog & Recovery (`src/catalog`, `src/recovery`)**: Handles persistent ASCII catalog saving schemas and ARIES logging.

---

## Core Concepts & Header Files Explained

To contribute to V2VDB, you must understand the core C++ header files. Here is a breakdown of the most critical `.h` files and how they interact.

### 1. `storage/disk_manager.h` (The File System Layer)
The `DiskManager` abstracts the OS-level file system. The database file (`mydb.db`) is not a text file; it is a contiguous array of exactly **4096-byte (4KB) pages**.

**Core Concept:** The database never reads individual bytes from the disk. It reads/writes entire 4KB pages at once using `page_id_t`.

```cpp
// include/storage/disk_manager.h
class DiskManager {
public:
    // Write 4KB of memory to the disk file securely using mutex locks
    void WritePage(page_id_t page_id, const char* page_data);

    // Read 4KB from the disk file into the memory pointer
    void ReadPage(page_id_t page_id, char* page_data);
};
```
*   **Contribution Tip:** If you implement a Buffer Pool Manager to cache these pages in RAM, `DiskManager` is the only class that should ever perform `std::fstream` operations.

### 2. `storage/table_page.h` (Physical Page Layout)
Once a 4KB block is loaded into RAM by the DiskManager, the `TablePage` class interprets those 4096 bytes.

**Core Concept:** A page contains a **Header** (metadata) and **Payload** (the actual tuples/rows). V2VDB uses an append-only serialization format for simplicity, but accommodates variable lengths.

```cpp
// include/storage/table_page.h
class TablePage {
    // Header offsets manually defined internally:
    static constexpr uint32_t OFFSET_NEXT_PAGE = 0;
    static constexpr uint32_t OFFSET_TUPLE_COUNT = 4;
    static constexpr uint32_t OFFSET_FREE_SPACE = 8;
    
    // Example: Trying to insert a Tuple physically into the 4KB memory layout
    bool InsertTuple(const Tuple& tuple) {
        uint32_t size = tuple.GetSerializedSize();
        uint32_t free_offset = GetFreeSpaceOffset();
        
        // If it exceeds 4096 bytes, return false. The TableHeap must allocate a new Page!
        if (free_offset + size > PAGE_SIZE) return false;
        
        tuple.Serialize(data_ + free_offset);
        SetFreeSpaceOffset(free_offset + size);
        return true;
    }
};
```

### 3. `index/b_plus_tree.h` (Query Optimization)
Instead of scanning every page (`O(N)` loop), `BPlusTree` allows lightning-fast `O(log N)` point lookups.

**Core Concept:** Trees are split into **Internal Pages** (which guide the search left or right based on > or < evaluations) and **Leaf Pages** (which contain the actual Record IDs pointing to the physical row).

```cpp
// include/index/b_plus_tree.h
class BPlusTree {
public:
    // Search the tree using a key. Returns the Record ID (RID) 
    bool GetValue(const int &key, RID &result);

    // Insert a new index mapping
    bool Insert(const int &key, const RID &value);
};
```

### 4. `common/rid.h` (Record Identifiers)
Every single row in the database has a unique physical address known as an `RID` (Record ID).
```cpp
// include/common/rid.h
class RID {
    page_id_t page_id_;    // Which 4KB page holds the data (e.g., Page 12)
    uint32_t slot_num_;      // Which position inside that page (e.g., Slot 3)
};
```

### 5. `catalog/catalog_manager.h` (Persistence)
When the database shuts down, the memory is wiped. The `CatalogManager` saves the schemas (Column names, Variable Types, and Root Page offsets) into a human-readable text file (`mydb.cat`).

```cpp
// include/catalog/catalog_manager.h
class CatalogManager {
public:
    void SaveCatalog(); // Serializes schema to DBInfo (.cat text format)
    void LoadCatalog(); // Parses .cat file to recreate C++ objects on boot
};
```
*   **Contribution Tip:** If you add a new data type (like `FLOAT` or `BOOLEAN`), you MUST update `SaveCatalog()` and `LoadCatalog()` to serialize that new type ID safely.

### 6. `executor/executor.h` (The Brain of the Database)
This bridges the user's SQL commands with the low-level physical engine. It verifies types, checks syntax, and drives the loops.

```cpp
// include/executor/executor.h
void HandleSelect(const Statement& stmt) {
    // 1. Fetch the physical TableHeap pointer
    TableHeap* table = tables_[stmt.table_name].get();
    
    // 2. Perform a Scan (or index lookup)
    std::vector<Tuple> tuples = table->Scan();
    
    // 3. Filter using the WHERE clause (Evaluated here internally!)
    for (const auto& tuple : tuples) {
        if (tuple.GetValue(col_idx).GetAsInteger() == stmt.where_value) {
            filtered_tuples.push_back(tuple);
        }
    }
}
```

---

## How to Contribute & Code

V2VDB relies heavily on modern C++17 paradigms.

### 1. Build Requirements
- **Compiler**: GCC 8+, Clang 9+, or MSVC 2019+ (Requires full `std::filesystem` support).
- **CMake**: Version 3.10 or higher.

```bash
git clone https://github.com/phravins/Database-Engine.git
cd Database-Engine
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

### 2. C++ Development Rules
*   **Encapsulation**: Never expose raw member variables. Use `Getters` and `Setters` explicitly.
*   **Smart Pointers**: Minimize raw pointer mapping. When injecting components, use `std::unique_ptr` exclusively inside maps unless passing context via un-owned `*ptr`.
*   **Console Coloring**: When printing errors to the CLI shell, use standard ANSI escape codes. Errors should always be `\033[1;31m` (Red).

### 3. Example Contribution: Adding a New Command

Imagine you want to add a new command: `TRUNCATE TABLE <name>`.

1.  **Update the Parser (`parser.h`)**: Add `TRUNCATE` to the `StatementType` enum. Add regex parsing logic to identify `"TRUNCATE TABLE (\w+)"`.
2.  **Add Handler (`executor.h`)**: Create `void HandleTruncate(const Statement& stmt)`. Look up the `TableHeap` inside the `tables_` generic map, delete the physical link, and initialize a brand-new page mapping for it!
3.  **Update the CLI (`shell.h`)**: Ensure the command prompt handles exceptions safely if `TRUNCATE` throws a disk error.

---

## Debugging Guide

When working with low-level page modifications, **segmentation faults** are common if pointers stray past the 4096-byte boundary.

1.  **Log Outputs**: Insert `std::cout` traces extensively when writing to physical buffers. 
2.  **Hex Dumps**: If the catalog states `page_id_t = 5` has corrupted tuples, use a Hex Editor on the raw `mydb.db` file. Jump to offset `(5 * 4096)` and manually inspect the integer layouts to trace serialization bugs in `TablePage::InsertTuple`.
3.  **Buffer Bounds**: Always use `sizeof()` logic instead of hardcoded numbers to avert pointer collisions.
