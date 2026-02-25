
# V2V Database Commands

Here is the list of English-like commands you can use to interact with the database.

## 1. Start the Database
Run this command in your terminal:
```powershell
v2vdb
```
*(If that doesn't work, try: `& "$env:LOCALAPPDATA\v2vdb\v2vdb.exe"`)*

## 2. English-like Commands

### Create a Table
**Syntax:** `make table <name> with fields <column> <type>, ...`

**Example:**
```sql
make table users with fields id int, name string
```

### Add Data
**Syntax:** `add to <name> values <val1>, <val2>...`

**Example:**
```sql
add to users values 1, 'Alice'
```

### Show Data
**Syntax:** `show me <name> [where <col> = <val>]`

**Example:**
```sql
show me users
show me users where id = 1
```

### Modify Data
**Syntax:**
- `update <name> set <col> = <val> where <col> = <val>`
- `delete from <name> where <col> = <val>`

**Example:**
```sql
update users set name = 'Bob' where id = 1
delete from users where id = 1
```

### Import / Export
**Syntax:** `export <name> <file>` / `import <name> <file>`

**Example:**
```sql
export users users.csv
import users users.csv
```

## 3. Diagnostics & Management

### Statistics & Info
- `SHOW TABLES` - List all tables
- `DESCRIBE <table_name>` - Show table schema
- `DBINFO` - General database statistics
- `SYSTEM` - Show active file paths
- `VERSION` - Version information

### Backup & Restore
- `BACKUP <prefix>` - Backup to <prefix>.db and <prefix>.cat
- `RESTORE <prefix>` - Restore from backup files
- `DROP TABLE <table_name>` - Delete a table

## 4. Custom Database Files
You can save to a specific file by running:
```powershell
v2vdb --db myProject.db --cat myProject.cat
```
(Legacy support for `v2vdb myProject` still exists and creates `myProject.db` and `myProject.cat`)
