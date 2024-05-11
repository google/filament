# Civetweb API Reference

### `mg_get_builtin_mime_type( file_name );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`file_name`**|`const char *`|The name of the file for which the MIME type has to be determined|

### Return Value

| Type | Description |
| :--- | :--- |
|`const char *`|A text string describing the MIME type|

### Description

The function `mg_get_builtin_mime_type()` tries to determine the MIME type of a given file. If the MIME type cannot be determined, the value `text/plain` is returned. Please note that this function does not an intelligent check of the file contents. The MIME type is solely determined based on the file name extension.

### See Also

* [`mg_send_mime_file();`](mg_send_mime_file.md)
* [`mg_send_mime_file2();`](mg_send_mime_file2.md)
