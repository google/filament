# Civetweb API Reference

### `mg_get_context( conn );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`const struct mg_connection *`|The connection for which the context has to be returned|

### Return Value

| Type | Description |
| :--- | :--- |
|`struct mg_context *`|A pointer to the context of the given connection|

### Description

The function `mg_get_context()` returns the context associated with a connection.

### See Also

* [`mg_start();`](mg_start.md)
* [`mg_stop();`](mg_stop.md)
* [`mg_get_user_data();`](mg_get_user_data.md)
* [`mg_get_thread_pointer();`](mg_get_thread_pointer.md)
