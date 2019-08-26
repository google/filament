# Civetweb API Reference

### `struct mg_form_data_handler;`

### Fields

|Field|Description|
|:---|:---|
|**`field_found`**|**`int field_found( const char *key, const char *filename, char *path, size_t pathlen, void *user_data )`**;|
||The callback function `field_found()` is called when a new field has been found. The return value of this callback is used to define how the field should be processed. The parameters contain the following information:|
||**`key`** - The name of the field as it was named with the `name` tag in the HTML source.|
||**`filename`** - The name of the file to upload. Please not that this parameter is only valid when the input type was set to `file`. Otherwise this parameter has the value `NULL`.|
||**`path`** - This is an output parameter used to store the full name of the file including the path to store an incoming file at the computer. This parameter must be provided by the application to Civetweb when a form field of type `file` is found. Please not that together with setting this parameter, the callback function must return `FORM_FIELD_STORAGE_STORE`.i With any other return value the contents of the `path` buffer is ignored by Civetweb.|
||**`pathlen`** - The length of the buffer where the output path can be stored.|
||**`user_data`** - A pointer to the value of the field `user_data` of the structure `struct mg_form_data_handler`.|
||The callback function `field_found()` can return the following values back to Civetweb:|
||**`FORM_FIELD_STORAGE_SKIP`** - Ignore the field and continue with processing the next field|
||**`FORM_FIELD_STORAGE_GET`** - Call the callback function `field_get()` to receive the form data|
||**`FORM_FIELD_STORAGE_STORE`** - Store a file as `path` and overwrite that file if it already exists|
||**`FORM_FIELD_STORAGE_ABORT`** - Stop parsing the request and ignore all remaining form fields|
|**`field_get`**|**`int field_get( const char *key, const char *value, size_t valuelen, void *user_data );`**|
||If the callback function `field_found()` returned `FORM_FIELD_STORAGE_GET`, Civetweb will call `field_get()` one or more times to pass back the data for this field.|
||**`key`** - the name of the field being decoded, note this is only passed on the first call for file parameters (bug?)|
||**`value`** - a pointer to the data read so far from the field's value|
||**`valuelen`** - the size of the data in the value for this call to `field_get()`|
||**`user_data`** - A pointer to the value of the field `user_data` of the structure `struct mg_form_data_handler`.|
||**`return` `MG_FORM_FIELD_HANDLE_GET`** - to continue calling get until all data has been received. |
||**`return` `MG_FORM_FIELD_HANDLE_NEXT`** - to skip further calls to get for this field and move on to the next field. |
||**`return` `MG_FORM_FIELD_HANDLE_ABORT`** - to stop parsing this form and abandon the search for further fields. |
|**`field_store`**|**`int field_store( const char *path, long long file_size, void *user_data );`**|
||If the callback function `field_found()` returned `FORM_FIELD_STORAGE_STORE`, Civetweb will try to store the received data in a file. If writing the file is successful, the callback function `field_store()` is called. This function is only called after completion of a full upload, not if a file has only partly been uploaded. When only part of a file is received, Civetweb will delete that partly upload in the background and not inform the main application through this callback. The following parameters are provided in the function call|
||**`path`** - The path on the server where the file was stored|
||**`file_size`** - The size of the stored file in bytes|
||**`user_data`** - The value of the field `user_data` when the callback functions were registered with a call to `mg_handle_form_request();`|
|**`user_data`**|**`void *`** |
||The `user_data` field is a user supplied argument that will be passed as parameter to each of callback functions|

### Description

The structure `struct mg_form_data_handler` contains callback functions for handling form fields. Form fields give additional information back from a web page to the server which can be processed by these callback functions.

### See Also

* [`mg_handle_form_request();`](mg_handle_form_request.md)
