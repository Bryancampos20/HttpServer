# HttpServer

# Para ejecutarlo

docker-compose build 

# Para ejecutar el servidor 
docker-compose up http_server
# Para ejecutar solo las pruebas
docker-compose up tests


# Para probar el get y post
http://localhost:1708

1. Poner encabezado : Key= Cookie Value= session_id= (un número)

2. Para el post, el json del body es:
{
  "name": "Nombre Apellido",
  "email": "correo@gmail.com"
}
