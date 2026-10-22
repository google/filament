# Civetweb API Reference

### `mg_get_var( data, data_len, var_name, dst, dst_len );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`data`**|`const char *`|Encoded buffer from either POST data or the URI of a GET call|
|**`data_len`**|`size_t`|Size of the encode buffer including the terminating NULL|
|**`var_name`**|`const char *`|Name of the variable to search for|
|**`dst`**|`char *`|Output buffer to store the content of the variable|
|**`dst_len`**|`size_t`|Length of the output buffer|

### Return Value

| Type | Description |
| :--- | :--- |
|`int`|The length of the variable or an error code|

### Description

The function `mg_get_var()` returns the value of a variable which is passed to the server with either a POST method, or as a parameter in the URI of a GET call. The data pointer passed to the function points to a form-URI encoded buffer. This can either be POST data or the `request_info.query_string`. The name of the searched variable and a buffer to store the results are also parameters to the function.

The function either returns the length of the variable when successful, **`-1`** if the variable could not be found and **`-2`** if the destination buffer is NULL, has size zero or is too small to store the resulting variable value.

### See Also

* [`mg_get_cookie();`](mg_get_cookie.md)
* [`mg_get_var2();`](mg_get_var2.md)
