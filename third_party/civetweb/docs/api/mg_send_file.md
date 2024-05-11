# Civetweb API Reference

### `mg_send_file( conn, path );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`|The connection over which the file must be sent|
|**`path`**|`const char *`|The full path and filename of the file|

### Return Value

*none*

### Description

The function `mg_send_file()` sends the contents of a file over a connection to the remote peer. The function also adds the necessary HTTP headers.

### See Also

* [`mg_send_file_body();`](mg_send_file_body.md)
* [`mg_send_mime_file();`](mg_send_mime_file.md)
* [`mg_send_mime_file2();`](mg_send_mime_file2.md)
* [`mg_printf();`](mg_printf.md)
* [`mg_write();`](mg_write.md)


