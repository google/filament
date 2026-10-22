# Civetweb API Reference

### `mg_get_response_code_text( conn, response_code );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`| A pointer referencing the connection |
|**`response_code`**|`int`| Response code for which the text is queried |

### Return Value

| Type | Description |
| :--- | :--- |
|`const char *`| A pointer to a human readable text explaining the response code. |

### Description

The function `mg_get_response_code_text()` returns a pointer to a human readable text describing the HTTP response code which was provided as a parameter.

### See Also

* [`mg_get_builtin_mime_type();`](mg_get_builtin_mime_type.md)
* [`mg_version();`](mg_version.md)
