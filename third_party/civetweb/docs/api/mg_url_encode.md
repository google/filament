# Civetweb API Reference

### `mg_url_encode( src, dst, des_len );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`src`**|`const char *`|Input string to encode|
|**`dst`**|`char *`|Destination buffer to store the encoded result|
|**`dst_len`**|`size_t`|Length of the destination buffer including the terminating NUL|

### Return Value

| Type | Description |
| :--- | :--- |
|`int`|The number of characters written in the destination buffer|

### Description

The function `mg_url_encode()` encodes a in input buffer. Both normal URIs and form URIs can be encoded. In the latter case the space character is converted to a `+` as defined in [RFC 1866](http://ftp.ics.uci.edu/pub/ietf/html/rfc1866.txt) in section 8.2.1.

### See Also

* [`mg_url_decode();`](mg_url_decode.md)
