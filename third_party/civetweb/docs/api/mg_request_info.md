# Civetweb API Reference

### `struct mg_request_info;`

### Fields

| Field | Type | Description |
| :--- | :--- | :--- |
|**`request_method`**|`const char *`| The request method used by the client for the connection this can be **GET**, **POST** or one of the other common HTTP request methods |
|**`request_uri`**|`const char *`| The absolute, relative or URL-encoded URI as it was sent in the request.  Example: "http://mydomain.com:8080/path/to/file.ext" or "/path/to/file.ext", depending on the client. |
|**`local_uri`**|`const char *`| The relative URL-encoded URI as it references the local resource. If the request URI does not reference a resource on the local server, this field is NULL.  Example: "/path/to/file.ext" (even if the client used "http://mydomain.com:8080/path/to/file.ext" in the request) |
|~~`uri`~~|`const char *`| *Deprecated. Use* `local_uri` *instead* |
|**`http_version`**|`const char *`| The HTTP version as mentioned in the client request. This can be "1.0", "1.1", etc. |
|**`query_string`**|`const char *`| The HTTP query string, defined as URL part after the first '?' character, not including '?'. NULL if there is no '?'. |
|**`remote_user`**|`const char *`| The name of the authenticated remote user, or NULL if no authentication was used. Only used for HTTP (digest) authentication, not for cookie based authentication. |
|**`remote addr`**|`char[48]`| The IP address of the remote client as a string. This can either represent an IPv4 or an IPv6 address.  Example: "127.0.0.1" |
|~~`remote_ip`~~|`long`| *Deprecated. Use* `remote_addr` *instead* |
|**`content_length`**|`long long`| The content length of the request body. This value can be -1 if no content length was provided. The request may still have body data, but the server cannot determine the length until all data has arrived (e.g. when the client closes the connection, or the final chunk of a chunked request has been received). |
|**`remote_port`**|`int`| The port number at the client's side (an integer number between 1 and 65535). |
|**`is_ssl`**|`int`| 1 if the connection is over SSL (https), and 0 if it is a plain connection (http) |
|**`user_data`**|`void *`| A pointer to the `user_data` information which was provided as a parameter to `mg_start()`. |
|**`conn_data`**|`void *`| A pointer to connection specific user data |
|**`num_headers`**|`int`| The number of HTTP request headers sent by the client (see http_headers) |
|**`http_headers`**|`struct mg_header[64]`| Array of structures with the HTTP request headers sent by the client. For the number of filled header fields, ee num_headers. |
|**`client_cert`**|`struct mg_client_cert *`| Pointer to the client certificate information, when available. This field is only filled for https connections using client certificates. |

### Description

The `mg_request_info` structure contains the client information of an existing connection.

### See Also

* [`struct mg_client_cert;`](mg_client_cert.md)
* [`struct mg_header;`](mg_header.md)
* [`mg_get_request_info();`](mg_get_request_info.md)
