#!/usr/bin/env bash
set -e

docker build -t convey-server -f server/Dockerfile .

docker rm -f convey-server 2>/dev/null || true

docker run -d \
  --name convey-server \
  -p 8080:8080 \
  -v "$(pwd)/server/certs:/app/server/certs" \
  -v "$(pwd)/server/data:/app/server/data" \
  -v "$(pwd)/server/logs:/app/server/logs" \
  convey-server
