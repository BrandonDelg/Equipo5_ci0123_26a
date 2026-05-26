# Como compilar el cliente-intermediario-servidor?

Estas pruebas son con localhost, de probar el codigo en el anexo cambie la ip a las ip correspondientes

## Para ipv6

```.
make clean; make; make run-server ARGS="1"
make run-client ARGS="::1 1 1234"

Donde ::1 indica la IP
1 indica que es ipv6
1234 indica el puerto a usar
```

## para ipv4

```.
make clean; make; make run-server ARGS="0"
make run-client ARGS="127.0.0.1 0 1234"

Donde 127.0.0.1 indica la IP
0 indica que es ipv4
1234 indica el puerto a usar
```

## Para la simulacion

```.
make clean; make; make run

```

## Insertar figuras en Filesystem

make insertarfs
make run-insertarfs
