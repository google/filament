# Civetweb API Reference

### `mg_check_feature( feature );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`feature`**|`unsigned`| A value indicating the feature to be checked |

### Return Value

| Type | Description |
| :--- | :--- |
|`unsigned`| A value indicating if a feature is available. A positive value indicates available, while **0** is returned for an unavailable feature |

### Description

The function `mg_check_feature()` can be called from an application program to check of specific features have been compiled in the civetweb version which the application has been linked to. The feature to check is provided as an unsigned integer parameter. If the function is available in the currently linked library version, a value **> 0** is returned. Otherwise the function `mg_check_feature()` returns the value **0**.

The following parameter values can be used:

| Value | Compilation option | Description |
| :---: | :---: | :--- |
| **1** | NO_FILES | *Able to serve files*.  If this feature is available, the webserver is able to serve files directly from a directory tree. |
| **2** | NO_SSL | *Support for HTTPS*. If this feature is available, the webserver van use encryption in the client-server connection. SSLv2, SSLv3, TLSv1.0, TLSv1.1 and TLSv1.2 are supported depending on the SSL library CivetWeb has been compiled with, but which protocols are used effectively when the server is running is dependent on the options used when the server is started. |
| **4** | NO_CGI | *Support for CGI*. If this feature is available, external CGI scripts can be called by the webserver. |
| **8** | USE_IPV6 | *Support IPv6*. The CivetWeb library is capable of communicating over both IPv4 and IPv6, but IPv6 support is only available if it has been enabled at compile time. |
| **16** | USE_WEBSOCKET | Support for web sockets. WebSockets support is available in the CivetWeb library if the proper options has been used during cimpile time. |
| **32** | USE_LUA | *Support for Lua scripts and Lua server pages*. CivetWeb supports server side scripting through the Lua language, if that has been enabled at compile time. Lua is an efficient scripting language which is less resource heavy than for example PHP. |
| **64** | USE_DUKTAPE | *Support for server side JavaScript*. Server side JavaScript can be used for dynamic page generation if the proper options have been set at compile time. Please note that client side JavaScript execution is always available if it has been enabled in the connecting browser. |
| **128** | NO_CACHING | *Support for caching*. The webserver will support caching, if it has not been disabled while compiling the library. |

Parameter values other than the values mentioned above will give undefined results. Therefore&mdash;although the parameter values for the `mg_check_feature()` function are effectively bitmasks, you should't assume that combining two of those values with an OR to a new value will give any meaningful results when the function returns.

### See Also

* [`mg_get_option();`](mg_get_option.md)
* [~~`mg_get_valid_option_names();`~~](mg_get_valid_option_names.md)
* [`mg_get_valid_options();`](mg_get_valid_options.md)
