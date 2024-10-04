## HTTP Server Project

Este proyecto implementa un servidor HTTP v1.x funcional desde cero, utilizando C++. El servidor soporta las principales operaciones HTTP (GET, POST, DELETE, PUT y UPDATE), maneja múltiples conexiones concurrentes utilizando hilos y gestiona sesiones de usuarios mediante cookies.

## Características principales
- **Operaciones HTTP**: Soporte para GET, POST, DELETE, PUT y UPDATE.
- **Concurrencia**: Manejo eficiente de múltiples conexiones simultáneas utilizando hilos (multithreading).
- **Sesiones con Cookies**: El servidor gestiona sesiones de usuarios mediante cookies de sesión.
- **Manejo de JSON**: El servidor solo parsea archivos JSON y texto plano.
  
## Requisitos del sistema
- **Lenguaje**: C++
- **Herramientas**:
  - Docker
  - Docker Compose
  - Makefile
  - Postman

## Estructura del proyecto
- **server.cpp**: Módulo principal del servidor, donde se manejan las conexiones de red y se crean los hilos para atender las solicitudes.
- **request_handler.h / request_handler.cpp**: Aquí se implementan las operaciones HTTP y se gestionan las cookies y la concurrencia con mutex.
- **tests.cpp**: Contiene pruebas unitarias para verificar las operaciones HTTP, el manejo de cookies y la concurrencia.
- **docker-compose.yml**: Define los servicios para ejecutar el servidor y las pruebas dentro de contenedores Docker.
- **Makefile**: Script para compilar y ejecutar el proyecto de manera automática.
  
## Instrucciones de instalación y ejecución

### 1. Requisitos previos
- **Docker**: Asegúrate de tener Docker instalado en tu sistema. Puedes instalarlo siguiendo las instrucciones oficiales en [https://www.docker.com/get-started](https://www.docker.com/get-started).

### 2. Construcción del proyecto
Para compilar el servidor y preparar el entorno, ejecuta:
```bash
docker-compose build
```

### 3. Ejecutar el servidor
Para levantar el servidor, usa:
```bash
docker-compose up http_server
```
El servidor estará escuchando en el puerto 1708.

### 4. Ejecutar pruebas
Para correr las pruebas unitarias:
```bash
docker-compose up tests
```

## Manejo de Cookies
El servidor gestiona las sesiones de usuario utilizando cookies, generando una nueva si no encuentra una existente.

## Descripción técnica

### Arquitectura del servidor
El servidor sigue una arquitectura cliente-servidor, en la cual un socket escucha en el puerto `1708`. Cada vez que un cliente se conecta, se crea un hilo para manejar la solicitud. Las operaciones HTTP se gestionan a través de la función `handle_request`, implementada en `request_handler.cpp`, la cual también gestiona las cookies y utiliza mutex para evitar condiciones de carrera en la concurrencia.

### Concurrencia
El servidor crea un nuevo hilo para cada conexión de cliente usando `pthread_create`, lo que permite manejar múltiples conexiones simultáneas sin bloquear el servidor. Los accesos a recursos compartidos, como el vector `entries`, están protegidos mediante mutexes.

### Pruebas unitarias
Las pruebas cubren las principales operaciones del servidor:
- **POST**: Añade una nueva entrada.
- **GET**: Recupera todas las entradas.
- **DELETE**: Elimina una entrada existente.
- **PUT**: Crea o modifica una entrada existente.
- **UPDATE**: Actualiza una entrada existente.
- **Manejo de cookies**: Verifica que las cookies se gestionan correctamente.