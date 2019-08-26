# Civetweb API Reference

### `mg_strncasecmp( s1, s2, len );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`s1`**|`const char *`|First string in the comparison|
|**`s2`**|`const char *`|Second string in the comparison|
|**`len`**|`size_t`|The maximum number of characters to compare|

### Return Value

| Type | Description |
| :--- | :--- |
|`int`|The result of the comparison|

### Description

The function `mg_strncasecmp()` is a helper function to compare two strings. The comparison is case insensitive and only a limited number of characters are compared. This limit is provided as third parameter in the function call. The return value is **0** if both strings are equal, less then zero if the first string is less than the second in a lexical comparison, and greater than zero if the first string is greater than the second.

### See Also

* [`mg_strcasecmp();`](mg_strcasecmp.md)
