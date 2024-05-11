# Civetweb API Reference

### `mg_check_digest_access_authentication( conn, realm, filename );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`| A pointer to the connection to be used to send data |
|**`realm`**|`const char *`| The requested authentication realm or NULL |
|**`filename`**|`const char *`| The path to the passwords file |

### Return Value

| Type | Description |
| :--- | :--- |
|`int`| An integer indicating success or failure |

### Description

This function can be used to check if a request header contains HTTP digest authentication
information, matching user and password encoded within the password file.
If the authentication realm (also called authentication domain) is NULL, the parameter
`authentication_domain` as specified in the server configuration (`mg_start()`) is used.

A positive return value means, the user name, realm and a correct password hash have been
found in the passwords file.
A return of 0 means, reading the password file succeeded, but there was no matching user,
realm and password.
The function returns a negative number on errors.

### See Also

* [`mg_send_digest_access_authentication_request();`](mg_send_digest_access_authentication_request.md)
* [`mg_modify_passwords_file();`](mg_modify_passwords_file.md)
* [`mg_start();`](mg_start.md)

