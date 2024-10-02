FROM ubuntu:latest

# IInstalación de dependencias
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git && \
    rm -rf /var/lib/apt/lists/*

# Clonamos Google Test, para las pruebas unitarias
RUN git clone https://github.com/google/googletest.git /usr/src/gtest

# Compilar Google Test
RUN cd /usr/src/gtest && \
    cmake . && \
    make && \
    cp lib/*.a /usr/lib && \
    cp -r googletest/include/gtest /usr/include/

COPY . /app
WORKDIR /app

# Compila tu proyecto
RUN make

# Exponer el puerto
EXPOSE 1708

# Para ejecutar el servidor
CMD ["./http_server"]

