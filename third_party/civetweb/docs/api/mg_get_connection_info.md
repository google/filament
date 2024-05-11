# Civetweb API Reference

### `mg_get_connection_info( ctx, idx, buffer, buflen );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`ctx`**|`struct mg_context *`|The server context handle|
|**`idx`**|`int`|Connection index within the context|
|**`buffer**|`char *`|A string buffer to store the information|
|**`buflen**|`int`|Size of the string buffer (including space for a terminating 0)|

### Return Value

| Type | Description |
| :--- | :--- |
|`int`|Available context information in bytes (excluding the terminating 0)|

### Description

The function `mg_get_connection_info()` returns statistics information collected for 
a server connection index.  This may be empty if the server has not been built with 
statistics support (`#define USE_SERVER_STATS`). 
If data is available, the returned string is in JSON format. The exact content may
vary, depending on the connection state and server version.

### Note

This is an experimental interface and may be changed, replaced
or even removed in the future. Currently the index `idx` must be
between `0` and `num_threads-1`. The thread is not locked for
performance reasons, so the information may be inconsistent 
in rare cases.

### See Also

* [`mg_get_system_info();`](mg_get_system_info.md)
* [`mg_get_context_info();`](mg_get_context_info.md)

