# Civetweb API Reference

### `mg_get_user_data( ctx );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`ctx`**|`const struct mg_context *`|The context for which the user data is requested|

### Return Value

| Type | Description |
| :--- | :--- |
|`void *`||

### Description

The function `mg_get_user_data()` returns the user data associated with a Civetweb context. This is a pointer value which has previously been used in the call to [`mg_start()`](mg_start.md) to initialize the server context.

### See Also

* [`mg_start();`](mg_start.md)
