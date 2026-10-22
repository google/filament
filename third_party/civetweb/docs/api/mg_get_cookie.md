# Civetweb API Reference

### `mg_get_cookie( cookie, var_name, buf, buf_len );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`cookie`**|`const char *`|The cookie name|
|**`var_name`**|`const char *`|The variable name|
|**`buf`**|`char *`|The buffer where to store the contents of the cookie|
|**`buf_len`**|`size_t`|The length of the cookie buffer, including the terminating NUL|

### Return Value

| Type | Description |
| :--- | :--- |
|`int`|The length of the cookie or an error code|

### Description

The function `mg_get_cookie()` tries to fetch the value of a certain cookie variable. The contents will either be stored in an application provided buffer, or an error code will be returned. The destination buffer is guaranteed to be NUL terminated if the pointer of the buffer is not a NULL pointer and the size of the buffer is at least one byte.

If the function succeeds, the return value of the function is the length in bytes of the cookie. The value **`-1`** is returned if the requested cookie could not be found and **`-2`** if the destination buffer is represented by a NULL pointer, is zero length or too short to store the whole cookie.

### See Also

* [`mg_get_var();`](mg_get_var.md)
* [`mg_get_var2();`](mg_get_var2.md)
