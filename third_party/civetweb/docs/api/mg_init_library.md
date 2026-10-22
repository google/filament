# Civetweb API Reference

### `mg_init_library( feature );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`feature`**|`unsigned`| A bitmask indicating the features to be ininialized |

### Return Value

| Type | Description |
| :--- | :--- |
|`unsigned`| A value indicating the initialized features is available. **0** is returned or error |

### Description

The function `mg_init_library()` should be called from an application program before using any other function.
It must be called only from one thread (it is not guaranteed to be thread safe).

This function is new in version 1.9 (as dummy implementation) and effective only from version 1.10.
For compatibility reasons, other functions (such as [`mg_start();`](mg_start.md)) will initialize the required features as well,
but they will no longer do a de-initialization, leaving a memory leak when the library is unloaded.

The following parameter values can be used:

| Value | Compilation option | Description |
| :---: | :---: | :--- |
| **1** | NO_FILES | *Able to serve files*.  If this feature is available, the webserver is able to serve files directly from a directory tree. |
| **2** | NO_SSL | *Support for HTTPS*. If this feature is available, the webserver can use encryption in the client-server connection. SSLv2, SSLv3, TLSv1.0, TLSv1.1 and TLSv1.2 are supported depending on the SSL library CivetWeb has been compiled with, but which protocols are used effectively when the server is running is dependent on the options used when the server is started. |
| **4** | NO_CGI | *Support for CGI*. If this feature is available, external CGI scripts can be called by the webserver. |
| **8** | USE_IPV6 | *Support IPv6*. The CivetWeb library is capable of communicating over both IPv4 and IPv6, but IPv6 support is only available if it has been enabled at compile time. |
| **16** | USE_WEBSOCKET | Support for web sockets. WebSockets support is available in the CivetWeb library if the proper options has been used during cimpile time. |
| **32** | USE_LUA | *Support for Lua scripts and Lua server pages*. CivetWeb supports server side scripting through the Lua language, if that has been enabled at compile time. Lua is an efficient scripting language which is less resource heavy than for example PHP. |
| **64** | USE_DUKTAPE | *Support for server side JavaScript*. Server side JavaScript can be used for dynamic page generation if the proper options have been set at compile time. Please note that client side JavaScript execution is always available if it has been enabled in the connecting browser. |
| **128** | NO_CACHING | *Support for caching*. The webserver will support caching, if it has not been disabled while compiling the library. |

The parameters can added using bitwise or. Values above 255 are reserved, the behavior of the function is undefined if any unknown bit is set.

### See Also

* [`mg_check_feature( feature );`](mg_check_feature.md)
* [`mg_exit_library( feature );`](mg_exit_library.md)
