# Civetweb API Reference

### `mg_get_request_link( conn, buf, buflen );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`| A pointer referencing the connection |
|**`buf`**|`char *`| A buffer to store the link |
|**`buflen`**|`size_t`| Size of the buffer |

### Return Value

| Type | Description |
| :--- | :--- |
|`int`| Return code: <0 for error, >=0 for success |

### Description

Store a formatted link corresponding to the current request.

E.g., returns
`http://mydomain.com:8080/path/to/callback.ext`
or 
`http://127.0.0.1:8080/path/to/callback.ext`
depending on the auth check settings.

### See Also
