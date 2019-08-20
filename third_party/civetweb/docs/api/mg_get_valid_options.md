# Civetweb API Reference

### `mg_get_valid_options();`

### Parameters

*none*

### Return Value

| Type | Description | 
| :--- | :--- |
|`const struct mg_option *`|An array with all valid configuration options|

### Description

The function `mg_get_valid_options()` returns an array with all valid configuration options of Civetweb. Each element in the array is a structure with three fields which represent the name of the option, the value of the option and the type of the value. The array is terminated with an element for which the name is `NULL`. See for more details about this structure the documentation of [`struct mg_option`](mg_option.md).

### See Also

* [`struct mg_option;`](mg_option.md)
* [`mg_start();`](mg_start.md)
