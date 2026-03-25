# Convey — Secure File Transfer (WIP)

**Convey** is a work-in-progress **file transfer utility** written in **C**, featuring a **custom binary protocol** over **TCP**, with **TLS encryption (OpenSSL)** and a **Dockerized server** for easy deployment.

The long-term goal is a small ecosystem:
- a CLI client (`convey`)
- a backend server (`server`)
- authenticated sessions (token-based)
- file inbox workflow: send → list inbox → accept/decline → download

---

## Features (current)
- **Custom binary protocol** with a fixed-size, versioned header and framed payloads
- **TLS encryption** using OpenSSL (`SSL_accept`, `SSL_connect`, `SSL_read`, `SSL_write`)
- **Concurrent server** using `poll()` to handle multiple client connections
- **Docker server build** (Ubuntu 22.04 + clang + OpenSSL dev libs)

---

## Status
This project is **actively evolving**. Expect:
- breaking protocol changes
- incomplete commands / missing message types
- missing validation + security hardening (auth, storage, limits)

---

## Repository layout
- `server/` — TLS server + polling loop + request handling
- `client/` — TLS client + simple interactive CLI (currently supports `login`)
- `protocol/` — protocol definitions + codec + transport helpers
- `run.sh` — build & run server container, mounts `server/certs`, `server/data`, `server/logs`

---

## Quick start (Docker server)

### 1) Create certificates
The server expects:
- `server/certs/cert.pem`
- `server/certs/key.pem`

Example (self-signed):
```bash
mkdir -p server/certs
openssl req -x509 -newkey rsa:2048 -nodes \
  -keyout server/certs/key.pem \
  -out server/certs/cert.pem \
  -days 365 \
  -subj "/CN=convey"
```

### 2) Build & run server
```bash
./run.sh
```

Server listens on port `8080` (mapped to host `8080`).

---

## Quick start (local client)

### Build
```bash
make -C client
```

### Run
```bash
./client/client
```

Commands (current):
- `login` — sends a `MSG_AUTH_LOGIN` frame (username + password)
- `exit`

---

## Protocol overview (high level)

Convey uses a **framed binary protocol**:
- fixed-size header (magic + version + type + payload length + token)
- payload bytes follow the header (`payload_len` bytes)

Notes:
- The client currently sends `token = 0` before authentication.
- The server reads header first, then allocates a payload buffer of `payload_len` and reads it fully.

