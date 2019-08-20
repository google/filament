# Civetweb API Reference

### `mg_send_http_ok( conn, mime_type, content_len );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`|The connection over which the data must be sent|
|**`mime_type`**|`const char *`|The content type to be sent|
|**`content_len`**|`long long`|Length of the following body data|

### Return Value

| Type | Description |
|`int`| An integer indicating success (>=0) or failure (<0) |


### Description

The function `mg_send_http_ok()` can be used to send a "HTTP 200 OK" response header, including typical response headers like date, length, content type, "no caching" and additional headers from the server configuration.
A proper content type (`mime_type`) has to be supplied (e.g., "text/plain", "text/html", "application/json", ...)
The. `status_code` must be one of the predefined HTTP standard error codes (e.g., "404" for "Not Found").
The `content_len` specifies the size of the response body in bytes. If the size is not yet known, a value of -1 could be supplied to select chunked transfer encoding. After `mg_send_http_ok()`, the server callback must send body, either using [`mg_write`](mg_write.md) (typical for known `content_len`), or using [`mg_send_chunk`](mg_send_chunk.md) (chunked encoding for unknown content length).


### See Also

* [`mg_send_http_error();`](mg_send_http_error.md)
* [`mg_send_http_redirect();`](mg_send_http_redirect.md)
* [`mg_write();`](mg_write.md)
* [`mg_send_chunk();`](mg_send_chunk.md)

