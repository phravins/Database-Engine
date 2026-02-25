
# V2V Query Language

The **V2V Query Language** is a natural English interface for interacting with the database. It allows you to perform standard database operations using simple, conversational sentences.

## Commands

### 1. Create a Table (`make table`)
Create a new table by defining its name and fields.

**Syntax:**
```typescript
make table <table_name> with fields <column1> <type>, <column2> <type>...
```

**Example:**
```sql
make table users with fields id int, name string, email text
```

### 2. Add Data (`add to`)
Insert a new row of data into a table.

**Syntax:**
```typescript
add to <table_name> values <value1>, <value2>...
```

**Example:**
```sql
add to users values 1, 'Alice', 'alice@example.com'
```

### 3. View Data (`show me`)
Retrieve all data from a table. Supports filtering.

**Syntax:**
```typescript
show me <table_name> [where <column> = <value>]
```

**Example:**
```sql
show me users
show me users where id = 1
```

### 4. Update Data (`update`)
Modify existing data in a table.

**Syntax:**
```typescript
update <table_name> set <column> = <value> where <column> = <value>
```

**Example:**
```sql
update users set name = 'Bob' where id = 1
```

### 5. Delete Data (`delete`)
Remove rows from a table.

**Syntax:**
```typescript
delete from <table_name> where <column> = <value>
```

**Example:**
```sql
delete from users where id = 1
```

### 6. Export/Import (`export`, `import`)
Save table data to a CSV file or load data from one.

**Syntax:**
```typescript
export <table_name> <filename>
import <table_name> <filename>
```

**Example:**
```sql
export users users.csv
import new_users users.csv
```

### 7. Diagnostics & Help (`help`, `system`, `dbinfo`)
Access administrative information about the database engine.

**Syntax:**
```typescript
help
system
dbinfo
version
```

**Example:**
- `system` - Shows active .db and .cat file paths.
- `dbinfo` - Shows table stats and disk usage.

### 8. Maintenance (`backup`, `restore`, `drop table`, `reload`)
Manage database lifecycle and safety.

**Syntax:**
```typescript
backup <prefix>
restore <prefix>
drop table <table_name>
reload
```

---

## Data Types & Aliases
V2V supports friendly aliases for standard SQL types.

| V2V Type | SQL Equivalent | Description |
| :--- | :--- | :--- |
| `string`, `text` | `VARCHAR` | Text or string data |
| `int`, `num`, `number` | `INTEGER` | Whole numbers |

---

## SQL Compatibility
The standard SQL commands are still fully supported.

| Action | V2V Command | SQL Equivalent |
| :--- | :--- | :--- |
| **Create** | `make table users ...` | `CREATE TABLE users ...` |
| **Insert** | `add to users ...` | `INSERT INTO users ...` |
| **Select** | `show me users` | `SELECT * FROM users` |
| **Delete** | `delete from users ...` | `DELETE FROM users ...` |
| **Update** | `update users ...` | `UPDATE users ...` |

## Identifiers

Table names and column names can be alphanumeric strings. 
**Note:** Special characters will be automatically removed for security.
e.g. `my-table` becomes `mytable`.
