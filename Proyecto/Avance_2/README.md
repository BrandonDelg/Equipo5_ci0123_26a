# Como compilar el cliente-intermediario-servidor?

Estas pruebas son con localhost, de probar el codigo en el anexo cambie la ip a las ip correspondientes

## Para ipv6

```.
make clean; make; make run-server ARGS="1"
make run-intermediario ARGS="::1 1"
make run-client ARGS="::1 1"
```

## para ipv4

```.
make clean; make; make run-server ARGS="0"
make run-intermediario ARGS="127.0.0.1 0"
make run-client ARGS="127.0.0.1 0"
```

## Para la simulacion

```.
make clean; make; make run

```
