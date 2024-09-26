# HttpServer

# Para ejecutarlo

docker build -t http_server .
docker run -p 1708:1708 http_server

# Para probar el get y post
http://localhost:1708

1. Poner encabezado : Key= Cookie Value= session_id= (un número)

2. Para el post, el json del body es:
{
  "name": "Nombre Apellido",
  "email": "correo@gmail.com"
}
