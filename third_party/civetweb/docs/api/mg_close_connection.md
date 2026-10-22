# Civetweb API Reference

### `mg_close_connection( conn );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`|The connection which must be closed|

### Return Value

*none*

### Description

The function `mg_close_connection()` is used to close a connection which was opened with the [`mg_download()`](mg_download.md) function. Use of this function to close a connection which was opened in another way is undocumented and may give unexpected results.

### See Also

* [`mg_download();`](mg_download.md)
