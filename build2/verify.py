import urllib.request
import json
import sys

URL = "http://localhost:8080/query"
HEADERS = {
    "X-Api-Key": "testkey",
    "Content-Type": "application/json"
}

def query(sql):
    data = json.dumps({"sql": sql}).encode('utf-8')
    req = urllib.request.Request(URL, data=data, headers=HEADERS, method='POST')
    try:
        with urllib.request.urlopen(req) as res:
            resp = json.loads(res.read().decode('utf-8'))
            print("SQL:", sql)
            print("RESULT:", repr(resp.get('result', resp)))
    except Exception as e:
        print("SQL:", sql)
        print("ERROR:", e)

# 2. Insert vectors into movies
query("INSERT INTO movies VALUES 1, 'Inception', [0.5, 0.8, 0.2]")
query("INSERT INTO movies VALUES 2, 'Matrix', [0.1, 0.9, 0.9]")
query("INSERT INTO movies VALUES 3, 'Interstellar', [0.4, 0.9, 0.15]")

# 3. Query
query("SELECT * FROM movies ORDER BY VECTOR_DIST(embedding, [0.4, 0.9, 0.1])")
query("SHOW TABLES")
