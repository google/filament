# Civetweb API Reference

### `mg_download( host, port, use_ssl, error_buffer, error_buffer_size, fmt, ... );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`host`**|`const char *`|The hostname or IP address of the server|
|**`port`**|`int`|The port number on the server|
|**`use_ssl`**|`int`|Use SSL if this value is not equal zero|
|**`error_buffer`**|`char *`|Buffer to store an error message|
|**`error_buffer_size`**|`size_t`|Size of the error message buffer including the terminating NUL|
|**`fmt`**|`const char *`|Format string specifying the remote command to execute|
|**`...`**|*various*|Parameters used in the format string|

### Return Value

| Type | Description |
| :--- | :--- |
|`struct mg_connection *`|A pointer to the connection structure if successful and NULL otherwise|

### Description

The `mg_download()` function is used to download data from a remote webserver. The server address can either be specified as a hostname or IP address and SSL can be used if needed. If the function succeeds, a pointer is returned to a connection structure. The connection must be closed with a call to the [`mg_close_connection()`](mg_close_connection.md) function.

The format string is a format string from the `printf()` series of functions to specify the remote command. An example to get the main index page from Google is the following call:

`conn = mg_download( "google.com", 80, 0, ebuf, sizeof(ebuf),
                     "%s", "GET / HTTP/1.0\r\nHost: google.com\r\n\r\n" );`

Please note that although Civetweb supports both IPv4 and IPv6 communication that IPv6 addressing is only available if it was enabled at compile time. When running an application it is possible to check if IPv6 support has been compiled in by using the [`mg_check_feature()`](md_check_feature.md) function with the parameter `USE_IPV6`.

### See Also

* [`mg_check_feature();`](mg_check_feature.md)
* [`mg_close_connection();`](mg_close_connection.md)
