# Civetweb API Reference

### `struct mg_option;`

### Fields

| Field | Type | Description |
| :--- | :--- | :--- |
|**`name`**|`const char *`|Name of the option|
|**`type`**|`int`|Type of the option|
|**`default_value`**|`const char *`|Value of the option|

### Description

A list of valid configuration options of the Civetweb instance can be retrieved with a call to [`mg_get_valid_options()`](mg_get_valid_options.md). This function fills a list of `struct mg_option` structures where the content of each structure represents a configuration option. Each structure contains three fields. One field contains the name of the option, the second contains the value of the option and the third is an identifier used to define the type of the option and how the value contents should be interpreted.

The field `type` can be one of the following values:

|Value|Description|
| :--- | :--- |
|**`CONFIG_TYPE_UNKNOWN`**|The type of the option value is unknown|
|**`CONFIG_TYPE_NUMBER`**|The option value is an integer|
|**`CONFIG_TYPE_STRING`**|The option value is a number|
|**`CONFIG_TYPE_FILE`**|The option value is a file name|
|**`CONFIG_TYPE_DIRECTORY`**|The option value is a directory name|
|**`CONFIG_TYPE_BOOLEAN`**|The option value is a boolean|
|**`CONFIG_TYPE_EXT_PATTERN`**|The option value is a list of regular expression patterns|

### See Also

* [`mg_get_valid_options();`](mg_get_valid_options.md)
