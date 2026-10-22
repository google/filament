# Civetweb API Reference

### `mg_get_thread_pointer( conn );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`const struct mg_connection *`|The connection, for which the context has to be returned, or NULL|

### Return Value

| Type | Description |
| :--- | :--- |
|`void *`|A pointer to the context of the given connection|

### Description

The function `mg_get_thread_pointer()` returns the user defined pointer associated with the thread.
This pointer is set as return value of the `mg_init_thread` callback (see [`struct mg_callbacks`](mg_callbacks.md)).

The `conn` parameter can be NULL - in this case, the user defined pointer is taken from the thread local storage.
For callbacks executed by server worker threads, `conn` can be the connection handle - in this case,
the user defined pointer is taken from the connection handle.
In both cases, the result will be the same pointer. Reading the pointer from the connection handle might be
faster on some systems.

### See Also

* [`struct mg_callbacks`](mg_callbacks.md)
* [`mg_get_user_data();`](mg_get_user_data.md)
* [`mg_get_context();`](mg_get_context.md)
