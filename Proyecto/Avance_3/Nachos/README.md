# Para usar el cliente de nachos con el serv de figuras propio use

```.
Primero abra una terminal en la carpeta cliente_servidor y ejecute:
make clean; make; make run-server ARGS="0"
luego:
desde la carpeta userprog en Nachos:
make clean; make depend; make
./nachos -x ../test/socket
```
