# Civetweb API Reference

### `mg_connect_websocket_client( host, port, use_ssl, error_buffer, error_buffer_size, path, origin, data_func, close_func, user-data);`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`host`**|`const char *`|The hostname or IP address of the server|
|**`port`**|`int`|The port on the server|
|**`use_ssl`**|`int`|Use SSL if this parameter is not equal to zero|
|**`error_buffer`**|`char *`|Buffer to store an error message|
|**`error_buffer_size`**|`size_t`|Size of the error message buffer including the NUL terminator|
|**`path`**|`const char *`|The server path to connect to, for example `/app` if you want to connect to `localhost/app`|
|**`origin`**|`const char *`|The value of the `Origin` HTTP header|
|**`data_func`**|`mg_websocket_data_handler`|Callback which is used to process data coming back from the server|
|**`close_func`**|`mg_websocket_close_handler`|Callback which is called when the connection is to be closed|
|**`user_data`**|`void *`|User supplied argument|

### Return Value

| Type | Description |
| :--- | :--- |
|`struct mg_connection *`|A pointer to the connection structure, or NULL if connecting failed|

### Description

The function `mg_connect_websocket_client()` connects to a websocket on a server as a client. Data and close events are processed with callback functions which must be provided in the call.

Civetweb supports both IPv4 and IPv6 communication, but only if the use if IPv6 has been enabled at compile time. When running an application it is possible to check if IPv6 addressing is available by calling the [`mg_check_feature()`](mg_check_feature.md) function with the `USE_IPV6` parameter.

### See Also

* [`mg_check_feature();`](mg_check_feature.md)
* [`mg_connect_client();`](mg_connect_client.md)
* [`mg_connect_client_secure();`](mg_connect_client_secure.md)
