# Civetweb API Reference

### `mg_get_var2( data, data_len, var_name, dst, dst_len, occurrence );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`data`**|`const char *`|Encoded data buffer from either POST data or a GET URI|
|**`data_len`**|`size_t`|The size of the encoded data buffer|
|**`var_name`**|`const char *`|The name of the variable to search for|
|**`dst`**|`char *`|Destination buffer to store the variable content|
|**`dst_len`**|`size_t`|The size of the destination buffer including the terminating NUL|
|**`occurrence`**|`size_t`|The instance index of the wanted variable|

### Return Value

| Type | Description |
| :--- | :--- |
|`int`|Length of the variable contents, or an error code|

### Description

The function `mg_get_var2()` can be used to return the contents of a variable passed to the server as either POST data, or in the URI in a GET call. The function is somilar to [`mg_get_var()`](mg_get_var.md) but the difference is that `mg_get_var2()` can be used if the same variable is present multiple times in the data. The `occurrence` parameter is used to identify which instance of the variable must be returned where **`0`** is used for the first variable with the specified name, **`1`** for the second and so on.

The function returns the length of the variable content in the return buffer, **`-1`** if a variable with the specified name could not be found and **`-2`** if the pointer to the result buffer is NULL, the size of the result buffer is zero or when the result buffer is too small to contain the variable content and terminating NUL.

### See Also

* [`mg_get_cookie();`](mg_get_cookie.md)
* [`mg_get_var();`](mg_get_var.md)
