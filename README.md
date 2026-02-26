# Convey (C) — File Transfer Backend + Custom Protocol

**Convey** is a work-in-progress **file transfer backend server** written in **C**, designed around an **application-specific protocol** (not HTTP) and intended to support multiple clients (initially C, later Swift for a mobile app).

The long-term goal is a cohesive system:
- a backend server
- persistent storage (DB / metadata)
- a simple, purpose-built protocol optimized for transferring files and managing sessions

---

## Status

**WIP / actively evolving.**  
Expect breaking protocol changes, incomplete commands, and missing hardening.

---

## Motivation

General-purpose protocols (HTTP/S3/etc.) are great, but Convey is built to:
- learn network programming and systems design in C
- control the full stack (wire format → storage → client UX)
- experiment with:
  - streaming large files
  - resumable transfers
  - authentication/session handling
  - metadata indexing
  - databases
  - tls 
