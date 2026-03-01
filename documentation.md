# V2V Database (v2vdb) - Full Documentation

**V2V Database** is a highly efficient, disk-backed relational database engine explicitly written in C++17 from scratch. It builds up custom-written internals for typical RDBMS functionalities such as paging, variable-length tuples execution, indexing, and crash recovery. Its core philosophy provides robust SQL compatibility with an innovative English-like command interface (`V2V Query Language`).

---

## Architecture & Core Components

V2V Database operates with full ACID principles based on modular implementations:

1.  **Storage Engine (`src/storage`)**
    *   **DiskManager**: Uses strictly 4KB binary pages to control disk I/O efficiently, keeping memory allocation clean.
    *   **Buffer Pool & Paging**: Handles in-memory pages mirroring disk structures.
    *   **B+ Tree implementation**: B+ tree data structures are integrated for highly efficient index tree-traversals resulting in O(log n) lookups.
2.  **Execution Engine (`src/execution`)**
    *   **TableHeap logic**: Features Heap file organizations allowing operations over variable-length and fixed-length tuples (e.g. `VARCHAR`, `INT`).
    *   **Parser & SQL Executor**: Features robust case-insensitive natural-language parsing for SQL variants and custom `make table`, `show me` commands.
3.  **Logs & Recovery (`src/recovery`)**
    *   **LogManager and Write-Ahead Logging (WAL)**: All database transactions generate logs mapped into `wal.log`.
    *   **ARIES Redo Logic**: Capable of protecting the database from sudden ungraceful system crashes.
4.  **Client Interface (`src/cli`)**
    *   Interactive CLI interface functioning natively across all operating systems offering dynamic tables and data feedback.

---

## Key Features

- **Relational Tables System**: Create tables, define data structures (`INT`, `VARCHAR`), insert tuples, modify parameters and query tables structurally.
- **Persistent Human-Readable Schemas**: Saves database metadata persistently in a `.cat` (catalog) format allowing external inspection using any text editor while actual page binaries sit securely in `.db`.
- **Fast Deployments**: Distributed securely as single compiled executable `v2vdb.exe` dynamically statically-linked on build, removing third-party service dependencies.
- **Diagnostic Utilities**: Real-time tools internally evaluating page disk sizes, records inserted count, backups routines (`DBINFO`, `SYSTEM`, `BACKUP`).

---

## Installation & Build Details

We provide universal one-command installations that require absolutely no compilers on target systems:

**Windows PowerShell (Run as Administrator):**
```powershell
powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; iwr -useb https://raw.githubusercontent.com/phravins/Database-Engine/main/installers/v2vdb-installer.cmd -OutFile install.bat; .\install.bat"
```

**Linux / macOS (Terminal):**
```bash
curl -fsSL https://raw.githubusercontent.com/phravins/Database-Engine/main/installers/v2vdb-installer.cmd | sudo bash
```

### Build from Source
If building directly from source (`C++17` standard requires `CMake` & `MinGW`/`GCC`):
- **Windows**: `build-all.bat`
- **Linux/macOS**: `chmod +x build-all.sh && ./build-all.sh`

---

## Running the Shell

Start the database engine from anywhere globally once installed:
```bash
v2vdb [--db custom.db] [--cat custom.cat]
```
If executed without arguments, it defaults to using `v2v-1.db` and `v2v-1.cat` locally. Data structures persist automatically on exit.

---

## Query Syntax & Commands

You can interact using either **Standard SQL** or **V2V Natural Commands**. Both are officially parsed by the engine interchangeably!

### 1. Data Type Definitions

| Type | Alternate Alias | Category |
| :--- | :--- | :--- |
| `VARCHAR` | `string`, `text` | Text or dynamically-scoped strings. |
| `INT` | `num`, `number`, `integer` | 32-bit Numeric integers. |

### 2. Table Creation

**V2V Language**:
```sql
make table users with fields id int, name string
```
**Standard SQL**:
```sql
CREATE TABLE users id INT, name VARCHAR
```

### 3. Inserting Records

**V2V Language**:
```sql
add to users values 1, "Alice"
```
**Standard SQL**:
```sql
INSERT INTO users VALUES 1 "Alice"
```

### 4. Querying and Fetching Data

**V2V Language**:
```sql
show me users
show me users where id = 1
```
**Standard SQL**:
```sql
SELECT * FROM users
SELECT * FROM users WHERE id = 1
```

### 5. Modifying & Deleting Target Data

**V2V Language & SQL Combination**:
```sql
update users set name = "Bob" where id = 1
delete from users where id = 1
```

### 6. CSV Export / Import Operations
Export internal unformatted DB records straight to `.csv` and vice-versa.
```sql
export users my_backup.csv
import users old_data.csv
```

---

## Diagnostic Administration commands

V2V Engine also provides strict metadata commands built to observe the database environment health.

*   `SHOW TABLES`: Lists all successfully stored tables under the active application's context.
*   `DESCRIBE <table_name>`: Inspects internal variable offsets, columns strings types internally.
*   `DROP TABLE <table_name>`: Will completely unlink variables and memory associated securely.
*   `DBINFO`: Tracks analytical usage mapping table count and allocated memory structures size.
*   `SYSTEM`: Display path definitions of `mydb.db` buffer output and `mydb.cat`.
*   `VERSION`: Checks internal CLI interface versions and database engine dependencies dates.

---

## Backup, Restore & Failsafe Strategies

To ensure transactional limits are not reached blindly:

*   **`BACKUP <prefix>`**: Safely mirrors the current state (`.db` and `.cat`) into `<prefix>.db` and `<prefix>.cat`.
*   **`RESTORE <prefix>`**: Restores previously compiled table structures logically.
*   **Corruption Handling**: Since `.cat` is fundamentally structured clearly in ASCII, unresolvable data conflicts or page corruptions can be soft-reset simply by manually deleting `mydb.db` and allowing structure schemas to overwrite next connection.

**To exit safely and automatically apply buffers:**
```sql
exit
```
