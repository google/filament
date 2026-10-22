# Civetweb API Reference

### `struct mg_response_info;`

### Fields

struct mg_response_info {
        int status_code;          /* E.g. 200 */
        const char *status_text;  /* E.g. "OK" */
        const char *http_version; /* E.g. "1.0", "1.1" */

        long long content_length; /* Length (in bytes) of the request body,
                                     can be -1 if no length was given. */

        int num_headers; /* Number of HTTP headers */
        struct mg_header
            http_headers[MG_MAX_HEADERS]; /* Allocate maximum headers */
};

| Field | Type | Description |
| :--- | :--- | :--- |
|**`status code`**|`int`| The HTTP response code received by the client. |
|**`status_text`**|`const char *`| The textual representation of the HTTP status code. |
|**`http_version`**|`const char *`| The HTTP version as mentioned in the client request. This can be "1.0", "1.1", etc. |
|**`content_length`**|`long long`| The content length of the request body. This value can be -1 if no content length was provided. The request may still have body data, but the server cannot determine the length until all data has arrived (e.g. when the client closes the connection, or the final chunk of a chunked request has been received). |
|**`num_headers`**|`int`| The number of HTTP request headers sent by the client (see http_headers) |
|**`http_headers`**|`struct mg_header[64]`| Array of structures with the HTTP request headers sent by the client. For the number of filled header fields, ee num_headers. |

Note: This structure is not yet feature complete and will be extended in future versions.

### Description

The `mg_response_info` structure contains information on a completed request from a client.

### See Also

* [`struct mg_header;`](mg_header.md)
* [`mg_get_response_info();`](mg_get_response_info.md)
