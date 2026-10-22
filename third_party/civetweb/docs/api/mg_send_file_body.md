# Civetweb API Reference

### `mg_send_file_body( conn, path );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`|The connection over which the file must be sent|
|**`path`**|`const char *`|The full path and filename of the file|

### Return Value

| Type | Description |
|`int`| An integer indicating success (>=0) or failure (<0) |

### Description

The function `mg_send_file_body()` sends the contents of a file over a connection to the remote peer without adding any HTTP headers. The code must send all required HTTP response headers before using this function.

### See Also

* [`mg_send_file();`](mg_send_file.md)
* [`mg_send_mime_file();`](mg_send_mime_file.md)
* [`mg_send_mime_file2();`](mg_send_mime_file2.md)
* [`mg_printf();`](mg_printf.md)
* [`mg_write();`](mg_write.md)

