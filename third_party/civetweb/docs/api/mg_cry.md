# Civetweb API Reference

### `mg_cry( conn, fmt, ... );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`const struct mg_connection *`|The connection on which a problem occurred|
|**`fmt`**|`const char *`|Format string without a line return|
|**`...`**|*various*|Parameters depending on the format string|

### Return Value

*none*

### Description

The function `mg_cry()` is called when something happens on a connection. The function takes a format string similar to the `printf()` series of functions with parameters and creates a text string which can then be used for logging. The `mg_cry()` function prints the output to the opened error log stream. Log messages can be processed with the `log_message()` callback function specified in the `struct mg_callbacks` structure.

### See Also

* [`struct mg_callbacks;`](mg_callbacks.md)
