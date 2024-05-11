# Civetweb API Reference

### `mg_websocket_write( conn, opcode, data, data_len );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`|Connection on which the data must be written|
|**`opcode`**|`int`|Opcode|
|**`data`**|`const char *`|Data to be written to the client|
|**`data_len`**|`size_t`|Length of the data|

### Return Value

| Type | Description |
| :--- | :--- |
|`int`|Number of bytes written or an error code|

### Description

The function `mg_websocket_write()` sends data to a websocket client wrapped in a websocket frame. The function issues calls to [`mg_lock_connection()`](mg_lock_connection.md) and [`mg_unlock_connection()`](mg_unlock_connection.md) to ensure that the transmission is not interrupted. Data corruption can otherwise happen if the application is proactively communicating and responding to a request simultaneously.

The function is available only when Civetweb is compiled with the `-DUSE_WEBSOCKET` option.

The function returns the number of bytes written, **0** when the connection has been closed and **-1** if an error occurred.

### See Also

* [`mg_lock_connection();`](mg_lock_connection.md)
* [`mg_printf();`](mg_printf.md)
* [`mg_unlock_connection();`](mg_unlock_connection.md)
* [`mg_websocket_client_write();`](mg_websocket_client_write.md)
* [`mg_write();`](mg_write.md)
