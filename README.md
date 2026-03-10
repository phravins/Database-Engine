# V2V Database (mydb)

[![User Guide](https://img.shields.io/badge/User_Guide-blue)](userguide.md)
[![Installation Guide](https://img.shields.io/badge/Installation_Guide-green)](installation.md)

**V2V Database** is an disk-backed relational database engine written in C++17. It implements core database internals from scratch, including paging, heap storage, B+ Tree indexing, and WAL-based crash recovery.

## Features

- **Storage Engine**: Custom `DiskManager` using 4KB pages.
- **Table System**: Heap file organization supporting variable-length tuples (`INT`, `VARCHAR`, `VECTOR`).
- **Vector Database**: First-class support for storing float arrays (`[1.0, 2.5]`) and sorting by Euclidean distance (`ORDER BY VECTOR_DIST(...)`).
- **Indexing**: B+ Tree index for efficient O(log n) lookups.
- **Server Mode**: Run as a REST API (`--server`) to easily connect with tools like **nginx**, web apps, or `curl`.
- **Crash Recovery**: Write-Ahead Logging (WAL) with ARIES-style Redo recovery.
- **Persistence**: **Human-readable catalog** (`.cat`) for easy inspection.
- **Diagnostics**: Built-in `DBINFO`, `DESCRIBE`, and `SYSTEM` status commands.
- **Interface**: Interactive SQL shell (`mydb>`) with case-insensitive parsing.

## Documentation
- [**Documentation**](documentation.md) - Comprehensive guide to all features, architecture, and syntax.
- [**V2V Query Language Guide**](V2V_LANGUAGE.md) - Detailed syntax guide for the natural language interface.
- [**Quick Command Cheatsheet**](DB_COMMANDS.md) - **<-- Look here for command examples!**
- [**Developers Documentation**](developers_documentation.md) - Deep dive into the C++ codebase, headers, and how to contribute.

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
v2vdb [--db path.db] [--cat path.cat] [--server] [--port 8080] [--api-key my-secret] [legacy_basename]
```

### 1. Interactive Shell Mode (Default)

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
    
    -- Create a table with a Vector column
    CREATE TABLE movies id INT, title VARCHAR, embedding VECTOR

    -- Insert vectors inline
    INSERT INTO movies VALUES 1, 'Inception', [0.5, 0.8, 0.2]
    
    -- Query by Vector Distance (KNN Search)
    SELECT * FROM movies ORDER BY VECTOR_DIST(embedding, [0.4, 0.9, 0.1])
    
    -- Exit
    exit
    ```

### 2. HTTP Server Mode (for Web/Nginx)
You can run V2VDB as an HTTP server to receive SQL queries via REST API, perfect for backing web applications or running behind an Nginx reverse proxy.

```bash
v2vdb --server --port 8080 --api-key "my_secret_key"
```

Then query it using `curl` or any HTTP client:
```bash
# Health check
curl http://localhost:8080/health

# Run a query
curl -X POST http://localhost:8080/query \
     -H "X-Api-Key: my_secret_key" \
     -H "Content-Type: application/json" \
     -d '{"sql": "SELECT * FROM users"}'
```
See the included `nginx.conf.example` file for a reverse proxy template.

### 3. Persistence
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


**Note:** 
  _Ensure you build the release as 64-bit (which is the default on modern compilers). Almost all Windows OS Works on 64-bit Windows, so it     will be perfectly compatible!_



_The V2V database system is implemented under the project ID - (Project CPP)_

