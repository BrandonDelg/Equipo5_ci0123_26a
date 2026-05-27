# Simulación — Protocolo v3.0

Simula la interacción entre Cliente, Servidor Intermediario (SI) y Servidor de Piezas (SP) usando el protocolo v3.0.

Todo el tráfico utiliza el formato `P/[Cmd]/[args]`:

| Mensaje | Significado |
|---|---|
| `P/R/` | Solicitar lista de figuras |
| `P/G/figura/mitad` | Solicitar piezas de una figura (mitad 1 o 2) |
| `P/Q/` | Cerrar conexión |
| `P/C/` | Registro de componente |
| `P/H/` | Heartbeat |
| `P/A/` | ALIVE (respuesta al heartbeat) |
| `P/D/datos` | Respuesta con datos |
| `P/D/400` | Error en la solicitud |
| `P/D/404` | Figura no encontrada |

## Instrucciones de compilación
```
make clean
make
```

## Instrucciones de ejecución
```
# Simulación interactiva
make run

# Correr todos los casos de prueba
make run-pruebas
```

## Casos de prueba

Los casos de prueba están en `casos_de_prueba/` como archivos de texto con las entradas de consola:

```
# Correr un caso individual
./bin/simulacion < casos_de_prueba/test_01_lista.txt
```

| Archivo | Descripción |
|---|---|
| `test_01_lista.txt` | Solicitar lista de figuras |
| `test_02_perro_mitad1.txt` | Piezas de Perro mitad 1 |
| `test_03_perro_mitad2.txt` | Piezas de Perro mitad 2 |
| `test_04_carro_mitad1.txt` | Piezas de Carro mitad 1 |
| `test_05_carro_mitad2.txt` | Piezas de Carro mitad 2 |
| `test_06_figura_invalida.txt` | Figura inexistente → `P/D/404` |
| `test_07_mitad_invalida.txt` | Mitad inválida → `P/D/400` |
