# Civetweb API Reference

### `mg_send_chunk( conn, buf, len );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`| A pointer to the connection to be used to send data |
|**`chunk`**|`const void *`| A pointer to the blob of information to be sent |
|**`chunk_len`**|`size_t`| The amount of bytes to be sent |

### Return Value

| Type | Description |
| :--- | :--- |
|`int`| An integer indicating the amount of bytes sent, or failure |

### Description

The function `mg_send_chunk()` can be used to send a blob of arbitrary data over a connection. 
Only use this function after sending a complete HTTP request or response header with "Transfer-Encoding: chunked" set. Otherwise: use `mg_write()`.
The function returns a number **>0** if data was sent, the value **0** when the connection has been closed, and **-1** in case of an error.

### See Also

* [`mg_write();`](mg_write.md)
* [`mg_printf();`](mg_print.md)
* [`mg_lock_connection();`](mg_lock_connection.md)
* [`mg_unlock_connection();`](mg_unlock_connection.md)

