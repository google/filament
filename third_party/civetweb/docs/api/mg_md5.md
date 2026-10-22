# Civetweb API Reference

### `mg_md5( buf, ... );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`buf`**|`char[33]`|Storage buffer for the calculated MD5 sum|
|**`...`**|`char *, ...`|NULL terminated list of pointers to strings with data|

### Return Value

| Type | Description |
| :--- | :--- |
|`char *`|Pointer to the resulting MD5 string|

### Description

The function `mg_md5()` caluclates the MD5 checksum of a NULL terminated list of NUL terminated ASCII strings. The MD5 checksum is returned in human readable format as an MD5 string in a caller supplied buffer.

The function returns a pointer to the supplied result buffer.

### See Also
