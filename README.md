# V2V Database (mydb)

[![User Guide](https://img.shields.io/badge/📖-User_Guide-blue)](userguide.md)

**V2V Database** is an disk-backed relational database engine written in C++17. It implements core database internals from scratch, including paging, heap storage, B+ Tree indexing, and WAL-based crash recovery.

## Features

- **Storage Engine**: Custom `DiskManager` using 4KB pages.
- **Table System**: Heap file organization supporting variable-length tuples (`INT`, `VARCHAR`).
- **Indexing**: B+ Tree index for efficient O(log n) lookups.
- **Crash Recovery**: Write-Ahead Logging (WAL) with ARIES-style Redo recovery.
- **Persistence**: **Human-readable catalog** (`.cat`) for easy inspection.
- **Diagnostics**: Built-in `DBINFO`, `DESCRIBE`, and `SYSTEM` status commands.
- **Interface**: Interactive SQL shell (`mydb>`) with case-insensitive parsing.

## Documentation
- [**V2V Query Language Guide**](V2V_LANGUAGE.md) - Detailed syntax guide.
- [**Quick Command Cheatsheet**](DB_COMMANDS.md) - **<-- Look here for examples!**

## Quick Install (No Compiler Required)

We provide a single universal installer script for Windows, Linux, and macOS.

### Windows
Run this command in PowerShell as Administrator:
```powershell
powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; iwr -useb https://raw.githubusercontent.com/phravins/Database-Engine/main/installers/v2vdb-installer.cmd -OutFile install.bat; .\install.bat"
```

### Linux / macOS
Run this command in your terminal:
```bash
curl -fsSL https://raw.githubusercontent.com/phravins/Database-Engine/main/installers/v2vdb-installer.cmd | sudo bash
```

## Package Manager Updates

If you are using package managers in the future:

### Homebrew (Mac/Linux)
```bash
brew update
brew upgrade v2vdb
```

### Chocolatey (Windows)
```bash
choco upgrade v2vdb
```

## Usage

After installation, simply type:
After installation, simply type:
```bash
v2vdb [--db path.db] [--cat path.cat] [legacy_basename]
```

**Default Credentials:**
- Username: `admin`
- Password: `admin`

1.  **Execute SQL**:
    The system supports a subset of SQL:
    
    ```sql
    -- Create a new table
    CREATE TABLE users id INT, name VARCHAR

    -- Insert data
    INSERT INTO users VALUES 1, 'Alice'
    INSERT INTO users VALUES 2, 'Bob'

    -- List all tables
    SHOW TABLES

    -- View schema
    DESCRIBE users

    -- Query data
    SELECT * FROM users
    
    -- Exit
    exit
    ```

2.  **Persistence**:
    - Data is stored in `mydb.db` (binary pages).
    - Logs are stored in `wal.log` (write-ahead log).
    - Schemas are stored in `mydb.cat` (catalog).
    
    You can close the application and restart it; your data and tables will remain.

## Build from Source (Optional)

If you want to contribute or build from source:

**Windows:**
```cmd
build-all.bat
```

**Linux / macOS:**
```bash
chmod +x build-all.sh
./build-all.sh
```

## Architecture

*   **src/storage**: Page layout, DiskManager, B+ Tree implementation.
*   **src/execution**: SQL Executor and TableHeap logic.
*   **src/recovery**: LogManager and ARIES Redo logic.
*   **src/cli**: Shell interface.

The V2V database system is implemented under the project ID named (Project Cpp)
-------------------------------------------------------------------------------
