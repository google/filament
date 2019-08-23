# Civetweb API Reference

### `mg_websocket_client_write( conn, opcode, data, data_len );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`|Connection on which to send data|
|**`opcode`**|`int`|Opcode|
|**`data const`**|`char *`|The data to be written|
|**`data_len`**|`size_t`|Length of the data buffer|

### Return Value

| Type | Description |
| :--- | :--- |
|`int`|Number of bytes written or an error code|

### Description

The function `mg_websocket_client_write()` sends data to a websocket server wrapped in a masked websocket frame. The function issues calls to [`mg_lock_connection()`](mg_lock_connection.md) and [`mg_unlock_connection()`](mg_unlock_connection.md) to ensure that the transmission is not interrupted. Interruption can happen the the application is proactively communicating and responding to a request simultaneously. This function is available only, if Civetweb is compiled with the option `-DUSE_WEBSOCKET`.

The return value is the number of bytes written on success, **0** when the connection has been closed and **-1** if an error occurred.

### See Also

* [`mg_lock_connection();`](mg_lock_connection.md)
* [`mg_printf();`](mg_printf.md)
* [`mg_unlock_connection();`](mg_unlock_connection.md)
* [`mg_websocket_write();`](mg_websocket_write.md)
* [`mg_write();`](mg_write.md)
