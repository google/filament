# Civetweb API Reference

### ~~`mg_get_ports( ctx, size, ports, ssl );`~~

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`ctx`**|`const struct mg_context *`||
|**`size`**|`size_t`|The number of ports which can be stored in the buffer|
|**`ports`**|`int *`|Buffer for storage of the port numbers|
|**`ssl`**|`int *`|Buffer used to store if SSL is used for the ports|

### Return Value

| Type | Description |
| :--- | :--- |
|`size_t`|The number of ports stored in the buffer|

### Description

This function is deprecated. Use [`mg_get_server_ports()`](mg_get_server_ports.md) instead.

The function `mg_get_ports()` returns a list of ports the Civetweb server is listening on. The port numbers are stored in a buffer of integers which is supplied by the calling party. The function also stores information if SSL is used on the ports. This information is stored in a second buffer which should be capable of storing the same amount of items as the ports buffer.

The function returns the number of ports actually stored in the buffer.

### See Also

* [`struct mg_server_port;`](mg_server_port.md)
* [`mg_get_server_ports();`](mg_get_server_ports.md)
