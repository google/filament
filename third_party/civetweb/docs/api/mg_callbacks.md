# Civetweb API Reference

### `struct mg_callbacks;`

### Fields

| Field | Description |
| :--- | :--- |
|**`begin_request`**|**`int (*begin_request)( struct mg_connection *conn );`**|
| |The `begin_request()` callback function is called when CivetWeb has received a new HTTP request. If the callback function does not process the request, it should return 0. In that case CivetWeb will handle the request with the default callback routine. If the callback function returns a value between 1 and 999, CivetWeb does nothing and the callback function should do all the processing, including sending the proper HTTP headers etc. Starting at CivetWeb version 1.7, the function `begin_request()` is called before any authorization is done. If an authorization check is required, `request_handler()` should be used instead. The return value of the callback function is not only used to signal CivetWeb to not further process the request. The returned value is also stored as HTTP status code in the access log. |
|**`connection_close`**|**`void (*connection_close)( const struct mg_connection *conn );`**|
| |The callback function `connection_close()` is called when CivetWeb is closing a connection. The per-context mutex is locked when the callback function is invoked. The function is primarily useful for noting when a websocket is closing and removing it from any application-maintained list of clients. *Using this callback for websocket connections is deprecated. Use* `mg_set_websocket_handler()` *instead.*|
|**`end_request`**|**`void (*end_request)(const struct mg_connection *conn, int reply_status_code);`**|
| |The callback function `end_request()` is called by CivetWeb when a request has been completely processed. It sends the reply status code which was sent to the client to the application.|
|**`exit_context`**|**`void (*exit_context)( const struct mg_context *ctx );`**|
| |The callback function `exit_context()` is called by CivetWeb when the server is stopped. It allows the application to do some cleanup on the application side.|
|**`http_error`**|**`int (*http_error)( struct mg_connection *conn, int status, const char *msg );`**|
| |The callback function `http_error()` is called by CivetWeb just before an HTTP error is to be sent to the client. The function allows the application to send a custom error page. The status code of the error is provided as a parameter. If the application sends their own error page, it must return 1 to signal CivetWeb that no further processing is needed. If the returned value is 0, CivetWeb will send a built-in error page to the client.|
|**`init_context`**|**`void (*init_context)( const struct mg_context *ctx );`**|
| |The callback function `init_context()` is called after the CivetWeb server has been started and initialized, but before any requests are served. This allowes the application to perform some initialization activities before the first requests are handled.|
|**`init_lua`**|**`void (*init_lua)( const struct mg_connection *conn, void *lua_context );`**|
| |The callback function `init_lua()` is called just before a Lua server page is to be served. Lua page serving must have been enabled at compile time for this callback function to be called. The parameter `lua_context` is a `lua_State *` pointer.|
|**`external_ssl_ctx`**|**`int (*external_ssl_ctx)(void **ssl_ctx, void *user_data);`**|
| |The callback function `external_ssl_ctx()` is called when civetweb is about to create (`*ssl_ctx` is `NULL`) or free (`*ssl_ctx` is not `NULL`) a SSL context. The parameter `user_data` contains a pointer to the data which was provided to `mg_start()` when the server was started. The callback function can return 0 to signal that CivetWeb should setup the SSL context. With a return value of 1 the callback function signals CivetWeb that the SSL context has already been setup and no further processing is necessary. Also with a return value of 1 another callback function `init_ssl()` is not called. The value -1 should be returned when the SSL context initialization fails.|
|**`init_ssl`**|**`int (*init_ssl)( void *ssl_context, void *user_data );`**|
| |The callback function `init_ssl()` is called when CivetWeb initializes the SSL library. The parameter `user_data` contains a pointer to the data which was provided to `mg_start()` when the server was started. The callback function can return 0 to signal that CivetWeb should setup the SSL certificate. With a return value of 1 the callback function signals CivetWeb that the certificate has already been setup and no further processing is necessary. The value -1 should be returned when the SSL initialization fails.|
|**`init_thread`**|**`void * (*init_thread)( const struct mg_context *ctx, int thread_type );`**|
| |The callback function `init_thread()` is called when a new thread is created by CivetWeb. The `thread_type` parameter indicates which type of thread has been created. following thread types are recognized:|
| |**0** - The master thread is created |
| |**1** - A worker thread which handles client connections has been created|
| |**2** - An internal helper thread (timer thread) has been created|
| |The returned `void *` is stored as user defined pointer in the thread local storage.|
|**`exit_thread`**|**`void (*exit_thread)( const struct mg_context *ctx, int thread_type, void * user_ptr);`**|
| |The callback function `exit_thread()` is called when a thread is about to exit. The parameters correspond to `init_thread`, with `user_ptr` being the return value.|
|**`log_access`**|**`int (*log_access)( const struct mg_connection *conn, const char *message );`**|
| |The callback function `log_access()` is called when CivetWeb is about to log a message. If the callback function returns 0, CivetWeb will use the default internal access log routines to log the access. If a non-zero value is returned, CivetWeb assumes that access logging has already been done and no further action is performed.|
|**`log_message`**|**`int (*log_message)( const struct mg_connection *conn, const char *message );`**|
| |The callback function `log_message()` is called when CivetWeb is about to log a message. If the callback function returns 0, CivetWeb will use the default internal log routines to log the message. If a non-zero value is returned CivetWeb assumes that logging has already been done and no further action is performed.|
|**`open_file`**|**`const char *(*open_file)( const struct mg_connection *conn, const char *path, size_t *data_len );`**|
| |The callback function `open_file()` is called when a file is to be opened by CivetWeb. The callback can return a pointer to a memory location and set the memory block size in the variable pointed to by `data_len` to signal CivetWeb that the file should not be loaded from disk, but that instead a stored version in memory should be used. If the callback function returns NULL, CivetWeb will open the file from disk. This callback allows caching to be implemented at the application side, or to serve specific files from static memory instead of from disk.|
|~~`upload`~~|**`void (*upload)( struct mg_connection * conn, const char *file_name );`**|
| |*Deprecated. Use* `mg_handle_form_request()` *instead.* The callback function `upload()` is called when CivetWeb has uploaded a file to a temporary directory as result of a call to `mg_upload()`. The parameter `file_name` contains the full file name including path to the uploaded file.|
|~~`websocket_connect`~~|**`int (*websocket_connect)( const struct mg_connection *conn );`**|
| |*Deprecated. Use* `mg_set_websocket_handler()` *instead.* The callback function `websocket_connect()` is called when a websocket request is received, before the actual websocket handshake has taken place. The callback function can signal to CivetWeb if it should accept or deny the incoming request with one of the following return values: |
| |**0** - CivetWeb can proceed with the handshake to accept the connection |
| |**1** - CivetWeb must close the connection immediately without performing a handshake |
|~~`websocket_data`~~|**`int (*websocket_data)( struct mg_connection *conn, int bits, char *data, size_t data_len );`**|
| |*Deprecated. Use* `mg_set_websocket_handler()` *instead.* The callback function `websocket_data()` is called when a data frame has been received from the client. The parameters contain the following information: |
| | **`bits`** - The first byte of the websocket frame. See [RFC-6455](http://tools.ietf.org/html/rfc6455) at section 5.2 for more information. |
| | **`data`** - The pointer to the received data block. Masks--if any--have already been applied. |
| | **`data_len`** - The length of the received data block |
| | If the application wants to keep the websocket open to receive more data, the callback function should return the value **1**. If the value **0** is returned by the callback function, CivetWeb will close the websocket connection and no more frames will be received.|
|~~`websocket_ready`~~|**`int (*websocket_ready)( struct mg_connection *conn );`**|
| |*Deprecated. Use* `mg_set_websocket_handler()` *instead.* The callback function `websocket_ready()` is called after the handshake of a websocket connection has succeeded successfully to signal the application that the connection is ready for use. |

### Description

Much of the functionality in the Civetweb library is provided through callback functions. The application registers their own processing functions with the Civetweb library and when an event happens, the appropriate callback function is called. In this way an application is able to have their processing code right at the heart of the webserver, without the need to change the code of the webserver itself. A number of callback functions are registered when the civetweb subsystem is started. Other may be added or changed at runtime with helper functions.

A pointer to a `mg_callbacks` structure is passed as parameter to the [`mg_start()`](mg_start.md) function to provide links to callback functions which the webserver will call at specific events. If a specific callback function is not supplied, CivetWeb will fallback to default internal callback routines. Callback functions give the application detailed control over how specific events should be handled.

### See Also

* [`mg_start();`](mg_start.md)
* [`mg_stop();`](mg_stop.md)
