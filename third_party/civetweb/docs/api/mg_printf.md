# Civetweb API Reference

### `mg_printf( conn, fmt, ... );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`|The connection over which the data must be sent|
|**`fmt`**|`const char *`|Format string|
|**`...`**|*various*|Parameters as specified in the format string|

### Return Value

| Type | Description |
| :--- | :--- |
|`int`|Number of bytes written or an error code|

### Description

The function `mg_printf()` can be used to send formatted strings over a connection. The functionality is comparable to the `printf()` family of functions in the standard C library. The function returns **0** when the connection has been closed, **-1** if an error occurred and otherwise the number of bytes written over the connection. Except for the formatting part, the `mg_printf()` function is identical to the function [`mg_write()`](mg_write.md).

### See Also

* [`mg_websocket_client_write();`](mg_websocket_client_write.md)
* [`mg_websocket_write();`](mg_websocket_write.md)
* [`mg_write();`](mg_write.md)
