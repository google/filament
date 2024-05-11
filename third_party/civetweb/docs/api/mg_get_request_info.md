# Civetweb API Reference

### `mg_get_request_info( conn );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`const struct mg_connection *`|The connection for which the request info is needed|

### Return Value

| Type | Description |
| :--- | :--- |
|`const struct mg_request_info *`|Pointer to the requested info, or NULL if an error occurred|

### Description

The function `mg_get_request_info()` returns information about the request on a given connection. This information is returned as a pointer to a [`mg_request_info`](mg_request_info.md) structure. If an error occurs, a NULL pointer is returned instead.

Use this function when implementing a server.

### See Also

* [`struct mg_request_info;`](mg_request_info.md)
* [`mg_get_response_info();`](mg_get_response_info.md)
* [`struct mg_response_info;`](mg_response_info.md)

