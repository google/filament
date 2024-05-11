# Civetweb API Reference

### `mg_get_response( conn, ebuf, ebuf_len, timeout );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`|The connection to listen on|
|**`ebuf`**|`char *`|Buffer to store an error message|
|**`ebuf_len`**|`size_t`|Size of the error message buffer including the terminating NUL|
|**`timeout`**|`int`|Time to wait for a response in milliseconds|

### Return Value

| Type | Description |
| :--- | :--- |
|`int`|Success value of the wait|

### Description

The function `mg_get_reponse()` wait for a response from a remote server. A return value equal or greater than zero is an indication for success, a negative value us used to signal an error condition. A timeout can be specified which lets the function return after a specified number of milliseconds, even if no data is received from the remote party. If the timeout value is negative, the function will not return until data has been read or an unrecoverable error occurs.

Error messages are stored in a caller supplied error message buffer.

### See Also

* [`mg_connect_client();`](mg_connect_client.md)
* [`mg_connect_client_secure();`](mg_connect_client_secure.md)
