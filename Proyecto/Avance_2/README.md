# Como compilar?

Para ipv6
make clean; make; make run-server ARGS="1"
make clean; make; make run-server ARGS="1"
make run-client ARGS="::1 1"

para ipv4
make clean; make; make run-server ARGS="0"
make clean; make; make run-server ARGS="0"
make run-client ARGS="127.0.0.1 0"
