# Civetweb API Reference

### `mg_set_auth_handler( ctx, uri, handler, cbdata );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`ctx`**|`struct mg_context *`|The context on which the handler must be set|
|**`uri`**|`const char *`|The URI for the authorization handler|
|**`handler`**|`mg_authorization_handler`|Callback function doing the actual authorization|
|**`cbdata`**|`void *`|Optional user data|

`int mg_authorization_handler( struct mg_connection *conn, void *cbdata );`

### Return Value

*none*

### Description

The function `mg_set_auth_handler()` hooks an authorization function to an URI to check if a user is authorized to visit that URI. The check is performed by a callback function of type `mg_authorization_handler`. The callback function is passed two parameters: the current connection and a pointer to optional user defined data which was passed to `mg_set_auth_handler()` when the callback was hooked to the URI.

The callback function can return **0** to deny access, and **1** to allow access.

The `mg_set_auth_handler()` function is very similar in use to [`mg_set_request_handler()`](mg_set_request_handler.md).

### See Also

* [`mg_set_request_handler();`](mg_set_request_handler.md)
