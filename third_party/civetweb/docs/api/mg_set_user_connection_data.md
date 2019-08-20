# Civetweb API Reference

### `mg_set_user_connection_data( conn, data );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`|connection to add the user data|
|**`data`**|`void *`|Pointer to the user data|

### Return Value

*none*

### Description

The function `mg_set_user_connection_data()` can be used to set a user defined
data pointer attached to a connection.  This value can be read using 
`mg_get_user_connection_data()`.
Any call to `mg_set_user_connection_data()` will overwrite a previously
assigned user data pointer.

`mg_set_user_connection_data()` requires a non-const 
`struct mg_connection *` to set the user data pointer.  It is save to use the
`const struct mg_connection *` passed to a websocket connect handler (with a
const cast), since `const` just means you must not use `mg_read()` or
`mg_write()` in this context.

Alternatively, you can use the `init_connection` callback in 
`struct mg_callbacks` to set the user data pointer.
In this case, typically `init_connection` is used to allocate memory for
a user defined `struct`, while `connection_close` is used to free this
memory again.


### See Also

* [`mg_get_user_connection_data();`](mg_get_user_connection_data.md)
* [`struct mg_callbacks`](mg_callbacks.md)
* [`mg_set_websocket_handler();`](mg_set_websocket_handler.md)
* [`mg_read();`](mg_read.md)
* [`mg_write();`](mg_write.md)

