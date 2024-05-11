# Civetweb API Reference

### `mg_get_context_info( ctx, buffer, buflen );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`ctx`**|`struct mg_context *`|The server context handle|
|**`buffer`**|`char *`|A string buffer to store the information|
|**`buflen`**|`int`|Size of the string buffer (including space for a terminating 0)|

### Return Value

| Type | Description |
| :--- | :--- |
|`int`|Available context information in bytes (excluding the terminating 0)|

### Description

The function `mg_get_context_info()` returns statistics information collected for
the server context.  This may be empty if the server has not been built with
statistics support (`#define USE_SERVER_STATS`).
If data is available, the returned string is in JSON format. The exact content may
vary, depending on the server state and server version.

### See Also

* [`mg_get_system_info();`](mg_get_system_info.md)
* [`mg_get_connection_info();`](mg_get_connection_info.md)


