FROM ubuntu:22.04

RUN apt update && apt install -y \
    clang \
    make \
    libssl-dev \
    libsqlite3-dev \
    pkg-config

WORKDIR /app

COPY . .

WORKDIR /app/server
RUN make clean && make

EXPOSE 8080

CMD ["./server"]
