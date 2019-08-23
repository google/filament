# Civetweb API Reference

### ~~`mg_get_valid_option_names();`~~

### Parameters

*none*

### Return Value

| Type | Description |
| :--- | :--- |
|`const char **`|An array with strings where the even elements represent the option names, and the odd element the option values The array is NULL terminated.|

### Description

The function `mg_get_valid_option_names()` is deprecated. Use [`mg_get_valid_options()`](mg_get_valid_options.md) instead.

This function returns an array with option/value pairs describing the valid configuration options for Civetweb. En element value of NULL signals the end of the list.

### See Also

* [`struct mg_option;`](mg_option.md)
* [`mg_get_valid_options();`](mg_get_valid_options.md)
