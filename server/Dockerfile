FROM ubuntu:22.04

RUN apt update && apt install -y \
    clang \
    make \
    libssl-dev \
    pkg-config

WORKDIR /app

COPY . .

WORKDIR /app/server
RUN make

EXPOSE 8080

CMD ["./server"]
