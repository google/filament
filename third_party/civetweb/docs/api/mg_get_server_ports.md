# Civetweb API Reference

### `mg_get_server_ports( ctx, size, ports );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`ctx`**|`const struct mg_context *`|The context for which the server ports are requested|
|**`size`**|`int`|The size of the buffer to store the port information|
|**`ports`**|`struct mg_server_port *`|Buffer to store the port information|

### Return Value

| Type | Description |
| :--- | :--- |
|`int`|The actual number of ports returned, or an error condition|

### Description

The `mg_get_server_ports()` returns a list with server ports on which the Civetweb server is listening. The ports are returned for a given context and stored with additional information like the SSL and redirection state in a list of structures. The list of structures must be allocated by the calling routine. The size of the structure is also passed to `mg_get_server_ports()`.

The function returns the number of items in the list, or a negative value if an error occurred.

### See Also

* [~~`mg_get_ports();`~~](mg_get_ports.md)
* [`struct mg_server_port;`](mg_server_port.md)
