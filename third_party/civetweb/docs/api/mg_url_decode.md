# Civetweb API Reference

### `mg_url_decode( src, src_len, dst, dst_len, is_form_url_encoded );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`src`**|`const char *`|Source data to convert|
|**`src_len`**|`int`|Length of the source buffer|
|**`dst`**|`char *`|Destination buffer to store the result|
|**`dst_len`**|`int`|Length of the destination buffer|
|**`is_form_url_encoded`**|`int`|Not equal zero when form decoding must be used|

### Return Value

| Type | Description |
| :--- | :--- |
|`int`|The number of bytes stored in the destination buffer, or **-1** if the buffer doesn't exist or is too small|

### Description

The function `mg_url_decode()` Decodes a in input buffer. Both normal URIs and form URIs can be decoded. In the latter case the space character is converted to a `+` as defined in [RFC 1866](http://ftp.ics.uci.edu/pub/ietf/html/rfc1866.txt) in section 8.2.1.

### See Also

* [`mg_url_encode();`](mg_url_encode.md)
