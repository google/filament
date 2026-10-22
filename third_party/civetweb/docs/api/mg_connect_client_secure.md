# Civetweb API Reference

### `mg_connect_client_secure( client_options, error_buffer, error_buffer_size );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`client_options`**|`const struct mg_client_options *`|Settings about the server connection|
|**`error_buffer`**|`char *`|Buffer to store an error message|
|**`error_buffer_size`**|`size_t`|Size of the error message buffer including the NUL terminator|

### Return Value

| Type | Description |
| :--- | :--- |
|`struct mg_connection *`||

### Description

The function `mg_connect_client_secure()` creates a secure connection with a server. The information about the connection and server is passed in a structure and an error message may be returned in a local buffer. The function returns a pointer to a `struct mg_connection` structure when successful and NULL otherwise.

Please note that IPv6 communication is supported by Civetweb, but only if the use of IPv6 was enabled at compile time. The check while running a program if IPv6 communication is possible you can call [`mg_check_feature()`](mg_check_feature.md) with the `USE_IPV6` parameter to check if IPv6 communications can be used.

### See Also

* [`struct mg_client_options;`](mg_client_options.md)
* [`mg_check_feature();`](mg_check_feature.md)
* [`mg_connect_client();`](mg_connect_client.md)
* [`mg_connect_websocket_client();`](mg_connect_websocket_client.md)
