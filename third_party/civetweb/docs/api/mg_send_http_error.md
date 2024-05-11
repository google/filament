# Civetweb API Reference

### `mg_send_http_error( conn, status_code, fmt, ... );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`|The connection over which the data must be sent|
|**`status_code`**|`int`|The HTTP status code (see HTTP standard)|
|**`fmt`**|`const char *`|Format string for an error message|
|**`...`**|*various*|Parameters as specified in the format string|

### Return Value

| Type | Description |
|`int`| An integer indicating success (>=0) or failure (<0) |


### Description

The function `mg_send_http_error()` can be used to send HTTP error messages from a server to a client.
The `status_code` must be one of the predefined HTTP standard error codes (e.g., "404" for "Not Found").
The status text (e.g., "Not Found") for standard error codes is known by this function.
A body of the error message, to explain the error in more detail, can be specified using the `fmt` format specifier and additional arguments. The `fmt` format specifier works like for the `printf()` function in the standard C library.


### See Also

* [`mg_send_http_ok();`](mg_send_http_ok.md)
* [`mg_send_http_redirect();`](mg_send_http_redirect.md)

