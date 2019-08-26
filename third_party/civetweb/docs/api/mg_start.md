# Civetweb API Reference

### `mg_start( callbacks, user_data, options );`

### Parameters

| Parameter | Type | Description |
| :--- | :--- | :--- |
|**`callbacks`**|`const struct mg_callbacks *`| A structure with optional callback functions to process requests from the web server |
|**`user_data`**|`void *`| A pointer to optional user data |
|**`options`**|`char **`| A list of options used to initialize the web server. The list consists of an NULL terminated list of option-value string pairs. |

The option list can be used to set the following options:

| Option | Default | Description |
| :--- | :--- | :--- |
| **`cgi_environment`** | *empty* | The option `cgi_environment` can contain extra variables to be passed to the CGI script in addition to the standard environment variables. The lust must be a comma separated list of name=value pairs like this: `VARIABLE1=VALUE1,VARIABLE2=VALUE2`.|
| **`cgi_interpreter`**| *empty* | The option `cgi_interpreter` can contain a path to an executable which will be used as a CGI interpreter for **all** CGI scripts regardless of the script file extension. If this option is not set (which is the default), CivetWeb looks at the first line of a CGI script to see if an interpreter is defined there. This first line is formatted as a shebang line as common in unix style shell scripts, but this will also work in Windows. For more information about the syntax, please see the Wikipedia page about the [shebang line](http://en.wikipedia.org/wiki/Shebang_(Unix\)).|
| | |For example on a Windows system where both PHP and Perl CGI scripts are used, `#!/path/to/php-cgi.exe` and `#!/path/to/perl.exe` must be the first line of the respective CGI scripts. Note that the paths should be either full file paths, or file paths relative to the current working directory of the CivetWeb server. The current working directory may be dependent on the way the application is started. When started from the command line it is the directory from where the executable was called, but when starting it from a shortcut in a graphical desktop environment, it will be the directory where the executable is located, the default directory of the user or a directory mentioned in the shortcut, depending on the operating system and graphical user interface used.|
| | |If all CGIs use the same interpreter, it is more efficient to set the option `cgi_interpreter` to the path to that executable because in that case no processing of the shebang line is necessary. When using PHP, be sure to point tot php-cgi(.exe) and not the php(.exe) executable, as the latter is a stand alone interpreter which doesn't interface over CGI with CivetWeb.
| **`cgi_pattern`** | `**.cgi$|**.pl$|**.php$` | All files that match `cgi_pattern` are treated as CGI files. The default pattern allows CGI files to be anywhere. To restrict CGIs to a certain directory, use `/path/to/cgi-bin/**.cgi` as a pattern. Note that the full path of the local file is matched against the pattern, not the URI provided in the client request.|
|**`put_delete_auth_file`**| *empty* | The option `put_delete_auth_file` defines the password file to be used for PUT and DELETE requests. Without a password file it is not possible to put new files to the server, or to delete existing ones. This only applies to direct HTTP requests which use the PUT and DELETE methods without server side scripting. PUT and DELETE requests might still be handled by Lua scripts and CGI pages. |

### Return Value

| Type | Description |
| :--- | :--- |
|`struct mg_context *`| A pointer to a context structure when successful, or NULL in case of failure |

### Description

The function `mg_start()` is the only function needed to call to initialize the webserver. After the function returns and a pointer to a context structure is provided, it is guaranteed that the server has started and is listening on the designated ports. In case of failure a NULL pointer is returned.  The behaviour of the web server is controlled by a list of callback functions and a list of options.  The callback functions can do application specific processing of events which are encountered by the webserver. If a specific callback function is set to NULL, the webserver uses their default callback routine. The options list controls how the webserver should be started and contains settings for for example the ports to listen on, the maximum number of threads created to handle requests in parallel and if settings for SSL encryption.

As a side effect on Unix systems, SIGCHLD and SIGPIPE signals will be ignored. If custom processing is needed for these signals, signal handlers must be setup after the call to `mg_start()` has completed.

### See Also

* [`struct mg_callbacks;`](mg_callbacks.md)
* [`mg_stop();`](mg_stop.md)
