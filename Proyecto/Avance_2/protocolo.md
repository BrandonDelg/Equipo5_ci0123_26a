# Protocolo de comunicacion entre servidores intermediarios

## Comunicación SP ↔ SI

- `JOIN <isla_id> <sp_puerto>`
  El servidor de piezas solicita enlazarse con el servidor intermediario correspondiente.

- `WELCOME`
  El servidor intermediario acepta la solicitud de conexión.

- `REJECT <reason>`
  El servidor intermediario rechaza la solicitud e indica la razón.

- `HEARTBEAT`
  El servidor intermediario consulta periódicamente si el servidor de piezas continúa activo.

- `ALIVE`
  El servidor de piezas responde al mensaje `HEARTBEAT` para confirmar que sigue en funcionamiento.

---

## Comunicación SI ↔ SI

- `NOTIFY_NEW <isla_id> <sp_ip> <sp_puerto>`
  El servidor intermediario notifica a los demás servidores intermediarios que un nuevo servidor de piezas se ha conectado.

- `NOTIFY_DROP <isla_id> <sp_ip> <sp_puerto>`
  El servidor intermediario notifica a los demás servidores intermediarios que un servidor de piezas se ha desconectado.

- `SYNC_REQUEST <isla_id>`
  El servidor intermediario solicita la información actualizada de los servidores de piezas registrados en los demás intermediarios.

- `SYNC_REPLY <isla_id> <sp_ip> <sp_puerto>`
  El servidor intermediario responde con la información actual de su servidor de piezas conectado.

## Registro de eventos

Cada evento se registrará en un archivo `.log` utilizando el siguiente formato:

`[FECHA] [TIPO] [EVENTO]: detalle`

### Descripción de formato

- `FECHA`: fecha y hora exacta del evento.
- `TIPO`: categoría del evento.
- `EVENTO`: nombre del evento.
- `detalle`: información adicional relevante.

### Tipos de eventos

- **INFO**:
  Registra eventos informativos, como solicitudes entre servidores intermediarios, conexiones de servidores de piezas y solicitudes realizadas por el cliente.

- **WARN**:
  Registra situaciones relevantes, como solicitudes sin respuesta (por ejemplo, `HEARTBEAT`), desconexión de servidores o solicitudes rechazadas.

- **ERROR**:
  Registra problemas graves, como múltiples solicitudes sin respuesta, errores de ejecución o errores en el protocolo de comunicación.
