# Civetweb API Reference

### `mg_stop( ctx );`

#### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`ctx`**|**`struct mg_context *`**| A pointer to the current webserver context |

### Return Value

*none*

### Description

The function `mg_stop()` is used to stop and cleanup a running webserver. A pointer to the context of the running webserver is provided as a parameter. The execution of this function may take some time because it waits until all threads have stopped and returns all memory to the heap. After the function returns, the location the context pointer points to is invalid. The function does not return a return value and it is therefore not possible to know if stopping the webserver succeeded or not.

### See Also

* [`mg_start();`](mg_start.md)
* [`mg_start_thread();`](mg_start_thread.md)
