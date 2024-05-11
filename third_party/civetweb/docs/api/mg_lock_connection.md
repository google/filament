# Civetweb API Reference

### `mg_lock_connection( conn );`

### Parameters

| Parameter | Type | Description |
|**`conn`**|`struct mg_connection *`|The connection to retrieve a lock|

### Return Value

*none*

### Description

The function `mg_lock_connection()` is specifically for websocket connections to lock connection. Using this function in combination with [`mg_unlock_connection();`](mg_unlock_connection.md) is necessary around [`mg_write()`](mg_write.md) and [`mg_printf()`](mg_printf.md) calls if the code has server-initiated communication, as well as with communication in direct response to a message.

### See Also

* [`mg_lock_context();`](mg_lock_context.md)
* [`mg_printf();`](mg_printf.md)
* [`mg_unlock_connection();`](mg_unlock_connection.md)
* [`mg_unlock_context();`](mg_unlock_context.md)
* [`mg_websocket_client_write();`](mg_websocket_client_write.md)
* [`mg_websocket_write();`](mg_websocket_write.md)
* [`mg_write();`](mg_write.md)
