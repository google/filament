# Civetweb API Reference

### `mg_unlock_context( ctx );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`ctx`**|`struct mg_context *`|The context to remove the lock from|

### Return Value

*none*

### Description

The function `mg_unlock_contect()` removes a lock put previously on a context with a call to [`mg_lock_context()`](mg_lock_context.md). Locking a context may be necessary when accessing shared resources.

### See Also

* [`mg_lock_connection();`](mg_lock_connection.md)
* [`mg_lock_context();`](mg_lock_context.md)
* [`mg_unlock_connection();`](mg_unlock_connection.md)
