# Civetweb API Reference

### `mg_send_http_redirect( conn, target_url, redirect_code );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`|The connection over which the data must be sent|
|**`target_url`**|`const char *`|The new target location|
|**`redirect_code`**|`int`|HTTP redirect response code|

### Return Value

| Type | Description |
|`int`| An integer indicating success (>=0) or failure (<0) |


### Description

The function `mg_send_http_redirect()` can be used to send a "HTTP 30x ..." redirect response.

The new location sent to the client (browser) is specified by `target_url`.  The location could be a relative or an absolute URL.

The parameter `redirect_code` defines, what kind of redirect is sent:

|`redirect_code`|type|HTTP method|
|301|permanent redirect||
|302|temporary redirect||
|303|temporary redirect|always use GET|
|307|temporary redirect|use same method|
|308|permanent redirect|use same method|

Status codes 301 and 302 were defined in HTTP/1.0, status codes 303, 307 and 308 were defined in HTTP/1.1.  For HTTP/1.1 redirects, it is explicitly defined if a redirected POST request should remain a POST request (307, 308) or should be a GET request (303) to the new target location.  Using 302 is not recommended for other requests than GET, since some browsers used to implement it as 303, others as 307.


### See Also

* [`mg_send_http_error();`](mg_send_http_error.md)
* [`mg_send_http_ok();`](mg_send_http_ok.md)

