# V2V Database (mydb)

**V2V Database** is an educational, disk-backed relational database engine written in C++17. It implements core database internals from scratch, including paging, heap storage, B+ Tree indexing, and WAL-based crash recovery.

## Features

- **Storage Engine**: Custom `DiskManager` using 4KB pages.
- **Table System**: Heap file organization supporting variable-length tuples (`INTEGER`, `VARCHAR`).
- **Indexing**: B+ Tree index for efficient O(log n) lookups.
- **Crash Recovery**: Write-Ahead Logging (WAL) with ARIES-style Redo recovery.
- **Persistence**: Catalog saves table schemas across restarts.
- **Interface**: Interactive SQL shell (`mydb>`).

## Prerequisites

- **OS**: Windows (tested), Linux/macOS (adaptable).
- **Compiler**: C++17 compatible (GCC, Clang, MSVC).
- **Build System**: CMake (3.10+).

## Installation

You can install V2V Database using the provided script or manually via CMake.

### Option 1: Quick Install (Command Line)
Run the installation script to build and install the binary locally.

**Windows:**
```cmd
install.bat
```

**Linux / macOS:**
```bash
chmod +x install.sh
./install.sh
```

This will create an `install/bin` folder containing the executable.

### Option 2: Create Package (Deployment)
Generates a distributable package (ZIP on Windows, ZIP/Tarball on Unix).

**Windows:**
```cmd
deploy.bat
```

**Linux / macOS:**
```bash
chmod +x deploy.sh
./deploy.sh
```

### Option 3: Manual Build (CLI)
If you prefer standard CMake commands:

```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
```
*(Note: Replace "MinGW Makefiles" with your generator of choice, e.g., "Visual Studio 17 2022" if using MSVC).*

## Usage

1.  **Start the Database**:
    ```cmd
    .\build\mydb.exe
    ```
    On startup, the system will perform crash recovery and load the catalog.

2.  **Execute SQL**:
    The system supports a subset of SQL:
    
    ```sql
    -- Create a new table
    CREATE TABLE users id INTEGER name VARCHAR

    -- Insert data
    INSERT INTO users VALUES 1 "Alice"
    INSERT INTO users VALUES 2 "Bob"

    -- Query data
    SELECT * FROM users
    
    -- Exit
    exit
    ```

3.  **Persistence**:
    - Data is stored in `mydb.db` (binary pages).
    - Logs are stored in `wal.log` (write-ahead log).
    - Schemas are stored in `mydb.cat` (catalog).
    
    You can close the application and restart it; your data and tables will remain.

## Architecture

*   **src/storage**: Page layout, DiskManager, B+ Tree implementation.
*   **src/execution**: SQL Executor and TableHeap logic.
*   **src/recovery**: LogManager and ARIES Redo logic.
*   **src/cli**: Shell interface.
