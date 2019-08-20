# Civetweb API Reference

### `mg_store_body( conn, path );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`|connection on which to read the data|
|**`path`**|`const char *`|file to store the request body|

### Return Value

| Type | Description |
| :--- | :--- |
|`long long`|Number of bytes written to the file, or an error code|

### Description

The function `mg_store_body()` stores the body of an incoming request to a data file. The function returns the number of bytes stored in the file, or a negative value to indicate an error.

### See Also

* [`mg_read();`](mg_read.md)
