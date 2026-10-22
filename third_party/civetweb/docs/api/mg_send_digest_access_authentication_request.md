# Civetweb API Reference

### `mg_send_digest_access_authentication_request( conn, realm );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`| A pointer to the connection to be used to send data |
|**`realm`**|`const char *`| The requested authentication realm or NULL |

### Return Value

| Type | Description |
| :--- | :--- |
|`int`| An integer indicating success or failure |

### Description

This function can be used to send a HTTP Digest Authentication request to the client.
Browsers will react with repeating the request with user authentication data.
If they do not yet know the user authentication for the requested realm, they will show
a dialog to query username and password.
In case the authentication realm (also called domain) is NULL, the parameter
`authentication_domain` from the server configuration is used.
The function returns a negative number on errors.

### See Also

* [`mg_check_digest_access_authentication();`](mg_check_digest_access_authentication.md)
* [`mg_modify_passwords_file();`](mg_modify_passwords_file.md)
* [`mg_send_http_error();`](mg_send_http_error.md)
* [`mg_write();`](mg_write.md)
* [`mg_printf();`](mg_print.md)

