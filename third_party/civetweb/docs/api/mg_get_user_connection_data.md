# Civetweb API Reference

### `mg_get_user_connection_data( conn );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`const struct mg_connection *`|The connection for which to return the user data|

### Return Value

| Type | Description | 
| :--- | :--- |
|`void *`|A pointer to the user data, or NULL if no user data was registered with the connection|

### Description

The function `mg_get_user_connection_data()` returns the user data associated with a connection. This user data is represented with a pointer which has been prevously registered with a call to [`mg_set_user_connection_data();`](mg_set_user_connection_data.md). With this function it is possible to pass state information between callback functions referring to a specific connection.

### See Also

* [`mg_set_user_connection_data();`](mg_set_user_connection_data.md)
