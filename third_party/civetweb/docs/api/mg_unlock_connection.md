# Civetweb API Reference

### `mg_unlock_connection( conn );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`|Connection to remove the lock from|

### Return Value

*none*

### Description

The function `mg_unlock_connection()` removes the lock on a connection which was previously set with a call to [`mg_lock_connection()`](mg_lock_connection.md). Locking may be necessary when using [`mg_write()`](mg_write.md) or [`mg_printf()`](mg_printf.md) on websocket connections to prevent data corruption.

### See Also

* [`mg_lock_connection();`](mg_lock_connection.md)
* [`mg_lock_context();`](mg_lock_context.md)
* [`mg_printf();`](mg_printf.md)
* [`mg_unlock_context();`](mg_unlock_context.md)
* [`mg_websocket_client_write();`](mg_websocket_client_write.md)
* [`mg_websocket_write();`](mg_websocket_write.md)
* [`mg_write();`](mg_write.md)
