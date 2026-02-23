# V2V Database User Guide

Welcome to the V2V Database! This guide will help you install, run, and interact with the database using SQL commands.

## Installation

To install V2V Database, use our universal installer script.

**Windows (PowerShell as Admin):**
```powershell
powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; iwr -useb https://raw.githubusercontent.com/phravins/Database-Engine/main/installers/v2vdb-installer.cmd -OutFile install.bat; .\install.bat"
```

**Linux / macOS:**
```bash
curl -fsSL https://raw.githubusercontent.com/phravins/Database-Engine/main/installers/v2vdb-installer.cmd | sudo bash
```

This will download the latest release and add `v2vdb` to your system PATH.

## Running the Database

Once installed, you can start the database shell from any terminal:

```bash
v2vdb [--db custom.db] [--cat custom.cat]
```

By default, it uses `v2v-1.db` and `v2v-1.cat`. Specifying a custom path allows you to manage multiple project databases independently.

This will launch the interactive SQL shell where you can type your commands.

## Supported SQL Commands

The V2V Database supports a subset of standard SQL. Below are the commands you can use.

### Create Table
Create a new table by specifying the table name and column definitions. Currently, types like `INT` (or `INTEGER`) and `VARCHAR` are supported.

**Syntax:**
```sql
CREATE TABLE <table_name> <col1_name> <col1_type> <col2_name> <col2_type> ...
```

**Example:**
```sql
CREATE TABLE users id INT name VARCHAR
```

### Insert Data
Insert rows into a table. Note that string values should be enclosed in double quotes.

**Syntax:**
```sql
INSERT INTO <table_name> VALUES <val1> <val2> ...
```

**Example:**
```sql
INSERT INTO users VALUES 1 "Alice"
INSERT INTO users VALUES 2 "Bob"
```

### Select Data
Retrieve all data from a table. Supports optional filtering.

**Syntax:**
```sql
SELECT * FROM <table_name> [WHERE <column> = <value>]
```

**Example:**
```sql
SELECT * FROM users
SELECT * FROM users WHERE id = 1
```

### Table Management
Explore your database structure with these new commands.

- **`SHOW TABLES`**: Lists all tables in the current database.
- **`DESCRIBE <table_name>`**: Shows column names, types (INT/VARCHAR), and internal offsets.
- **`DROP TABLE <table_name>`**: Deletes a table and its associated data permanently.

### Diagnostics & Backup
Keep your database healthy and backed up.

- **`SYSTEM`**: Displays current active file paths for the database and catalog.
- **`DBINFO`**: Shows statistics including table count, total columns, and file size on disk.
- **`BACKUP <prefix>`**: Creates a copy of your database (e.g., `my_backup.db` and `my_backup.cat`).
- **`RESTORE <prefix>`**: Replaces current files with those from a backup (requires restart/reload).
- **`VERSION`**: Displays engine version and build timestamp.

### Exit
To exit the database shell, type:

```sql
exit
```

## Troubleshooting
- **Catalog Inspection**: The catalog file (`.cat`) is now human-readable. You can open it in any text editor to verify your schemas.
- **File Corruption**: If the database file becomes corrupted, use `RESTORE` if you have a backup, or delete the `.db` and `.cat` files to start fresh.
- **Path Issues**: Ensure you have write permissions in the directory where your `.db` and `.cat` files are located.

Happy Querying!
