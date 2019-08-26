# Civetweb API Reference

### `mg_send_mime_file( conn, path, mime_type );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`|The connection over which the file must be sent|
|**`path`**|`const char *`|The full path and filename of the file|
|**`mime_type`**|`const char *`|The mime type of the file, or NULL for automatic detection|

### Return Value

*none*

### Description

The function `mg_send_mime_file()` sends a file over a connection including the HTTP headers. The function is similar to the [`mg_send_file()`](mg_send_file.md) with the additional functionality that the MIME type of the file can be specified. If the `mime_type` parameter is NULL, the routine will try to determine the MIME type based on the extension of the filename.

### See Also

* [`mg_get_builtin_mime_type();`](mg_get_builtin_mime_type.md)
* [`mg_printf();`](mg_printf.md)
* [`mg_send_file();`](mg_send_file.md)
* [`mg_send_mime_file2();`](mg_send_mime_file2.md)
* [`mg_write();`](mg_write.md)
