# Civetweb API Reference

### `mg_read( conn, buf, len );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`conn`**|`struct mg_connection *`| A pointer referencing the connection |
|**`buf`**|`void *`| A pointer to the location where the received data can be stored |
|**`len`**|`size_t`| The maximum number of bytes to be stored in the buffer |

### Return Value

| Type | Description |
| :--- | :--- |
|`int`| The number of read bytes, or a status indication |

### Description

The function `mg_read()` receives data over an existing connection. The data is handled as binary and is stored in a buffer whose address has been provided as a parameter. The function returns the number of read bytes when successful, the value **0** when the connection has been closed by peer and a negative value when no more data could be read from the connection.

### Example

```
#define RECV_BUF_SIZE 1 << 20

size_t read_data(struct mg_connection* conn,
                        uint8_t* buff,
                        size_t buff_len) {
  size_t read_len = 0;
  while (read_len < buff_len) {
    size_t sz_to_read = std::min<size_t>(RECV_BUF_SIZE, buff_len - read_len);
    int this_read = mg_read(conn, buff + read_len, sz_to_read);
    if (this_read < 0) {
      std::cerr << "[error] Failed to read data" << std::endl;
      break;
    } else {
      read_len += size_t(this_read);
      if (this_read > 0) {
        std::cout << "[debug] Received " << this_read << " more bytes" << std::endl;
      }
    }
  }

  return read_len;
}
```

### See Also

* [`mg_printf();`](mg_printf.md)
* [`mg_write();`](mg_write.md)
