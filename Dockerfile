FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    build-essential \
    curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN make

EXPOSE 1708

CMD ["./http_server"]
