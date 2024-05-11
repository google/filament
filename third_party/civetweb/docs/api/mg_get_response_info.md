# Civetweb API Reference

### `mg_get_response_info( conn );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`const struct mg_connection *`|The connection for which the response info is needed|

### Return Value

| Type | Description |
| :--- | :--- |
|`const struct mg_response_info *`|Pointer to the response info, or NULL if an error occurred|

### Description

The function `mg_response_info()` returns information about a response on a client connection opened by `mg_connect_client()`. If an error occurs, a NULL pointer is returned instead.

Use this function when implementing a client.

### See Also

* [`struct mg_response_info;`](mg_response_info.md)
* [`mg_connect_client();`](mg_connect_client.md)
* [`mg_get_request_info();`](mg_get_request_info.md)
* [`struct mg_request_info;`](mg_request_info.md)

