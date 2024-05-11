# Civetweb API Reference

### `mg_lock_context( ctx );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`ctx`**|`struct mg_context *`|The context to put the lock on|

### Return Value

*none*

### Description

The function `mg_lock_context()` can be used to acquire a lock for exclusive access to resources which are shared between connection of threads. The lock is context wide. The lock must be released with a call to [`mg_unlock_context()`](mg_unlock_context.md).

### See Also

* [`mg_lock_connection();`](mg_lock_connection.md)
* [`mg_unlock_connection();`](mg_unlock_connection.md)
* [`mg_unlock_context();`](mg_unlock_context.md)
