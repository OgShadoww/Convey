CREATE TABLE users (
    id INTEGER PRIMARY KEY,
    username TEXT UNIQUE,
    password_hash TEXT
);

CREATE TABLE files (
    id INTEGER PRIMARY KEY,
    user_id INTEGER,
    filename TEXT,
    size INTEGER,
    uploaded_at INTEGER
);
