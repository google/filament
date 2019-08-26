Embedding CivetWeb
=========

CivetWeb is primarily designed so applications can easily add HTTP and HTTPS server as well as WebSocket (WS and WSS) server functionality.
For example, a C/C++ application could use CivetWeb to enable a web service and configuration interface, to add a HTML5 data visualization interface, for automation or remote control, as a protocol gateway or as a HTTP/WebSocket client for firewall traversal.

It can also be used as a stand-alone executable. It can deliver static files and offers built-in server side Lua, JavaScript and CGI support. Some instructions how to build the stand-alone server can be found in [Building.md](https://github.com/civetweb/civetweb/blob/master/docs/Building.md).


Files
------

There is just a small set of files to compile in to the application,
but if a library is desired, see [Building.md](https://github.com/CivetWeb/CivetWeb/blob/master/docs/Building.md)


#### Regarding the INL file extension
The *INL* file extension represents code that is statically included inline in a source file.  Slightly different from C++ where it means "inline" code which is technically not the same as static code. CivetWeb overloads this extension for the sake of clarity as opposed to having .c extensions on files that should not be directly compiled.


#### HTTP Server Source Files
These files constitute the CivetWeb library.  They do not contain a `main` function,
but all functions required to run a HTTP server.

  - HTTP server API
    - include/civetweb.h
  - C implementation
    - src/civetweb.c
    - src/md5.inl (MD5 calculation)
    - src/sha1.inl (SHA calculation)
    - src/handle\_form.inl (HTML form handling functions)
    - src/timer.inl (optional timer support)
  - Optional: C++ wrapper
    - include/CivetServer.h (C++ interface)
    - src/CivetServer.cpp (C++ wrapper implementation)
  - Optional: Third party components
    - src/third\_party/* (third party components, mainly used for the standalone server)
    - src/mod\_*.inl (modules to access third party components from civetweb)


Note: The C++ wrapper uses the official C interface (civetweb.h) without adding any features to the server itself. Several features available in the C interface are missing in the C++ interface. While all features should be accessible using the C interface, this is not a design goal of the C++ interface.


#### Additional Source Files for Executables
These files can be used to build a server executable. They contain a `main` function
starting the HTTP server.

  - Stand-alone C server
      - src/main.c
  - Reference embedded C server
      - examples/embedded\_c/embedded\_c.c
  - Reference embedded C++ server
      - examples/embedded\_cpp/embedded\_cpp.cpp

Note: The "embedded" example is actively maintained, updated, extended and tested. Other examples in the examples/ folder might be outdated and remain there for reference.


Quick Start
------

By default, the server will automatically serve up files like a normal HTTP server.  An embedded server is most likely going to overload this functionality.

### C
  - Include the C interface ```civetweb.h```.
  - Use `mg_start()` to start the server.
      - Use *options* to select the port and document root among other things.
      - Use *callbacks* to add your own hooks.
  - Use `mg_set_request_handler()` to easily add your own request handlers.
  - Use `mg_stop()` to stop the server.

### C++
  - Note that CivetWeb is Clean C, and C++ interface ```CivetServer.h``` is only a wrapper layer around the C interface.
    Not all CivetWeb features available in C are also available in C++.
  - Create CivetHandlers for each URI.
  - Register the handlers with `CivetServer::addHandler()`
  - `CivetServer` starts on construction and stops on destruction.
  - Use constructor *options* to select the port and document root among other things.
  - Use constructor *callbacks* to add your own hooks.

Alternative quick start: Have a look at the examples embedded\_c and embedded\_cpp


Feature selection
------

CivetWeb is highly customizable at build time, in addition to configuration at start time.

##### start time options
Start time options are passed to `mg_start`. They are documented in the [UserManual.md](https://github.com/civetweb/civetweb/blob/master/docs/UserManual.md).

##### callbacks
Pointers to callback functions are passed to `mg_start` as well. They are documented in [civetweb.h](https://github.com/civetweb/civetweb/blob/master/include/civetweb.h) and the callbacks [API documentation](https://github.com/civetweb/civetweb/blob/master/docs/api/mg_callbacks.md).

##### compiler defines
Several features can be turned "on" or "off" by setting compile defines. CivetWeb builds with a reasonable default feature set. Optional features not including in the default can be added by adding a `USE\_<feature>` define. Default features can be removed by adding a `NO_<feature>` define. E.g., to build with Lua support, set `#define USE_LUA` (-DUSE_LUA), to build without CGI support set `#define NO_CGI` (-DNO_CGI). A list of feature defines is available in [Building.md](https://github.com/civetweb/civetweb/blob/master/docs/Building.md) - some versions may have additional, undocumented feature defines. Undocumented defines may become unavailable in future versions without notice.

##### externally provided functions
In some special cases, it might be meaningful to completely replace an internal function in [civetweb.c](https://github.com/civetweb/civetweb/blob/master/src/civetweb.c) with your own implementation.
Since CivetWeb is free and open source software covered by the MIT license, you can feel free to just edit civetweb.c according to your needs.
However, this might be annoying when updating the server, pulling new features or bug fixes from the main repository. For some selected functions, it is possible to provide your own implementation using a `MG_EXTERNAL_FUNCTION_<internal_function_name>` define. For details on this mechanism, please look directly into the source code [civetweb.c](https://github.com/civetweb/civetweb/blob/master/src/civetweb.c). Interfaces and even names of internal functions may change without notice - when you use these defines, you have to check this every time you update CivetWeb. It might still be less effort than to apply your patches every time.
This customization option is currently in an evaluation phase. In case you need additional function defines, please create an issue on GitHub explaining your use case, to discuss if this would be an appropriate solution - in general, other customization options are preferred.


Lua Support
------

Lua is a server side include functionality.  Files ending in .lua will be processed with Lua.

##### Add the following CFLAGS

  - `-DLUA_COMPAT_ALL`
  - `-DUSE_LUA`
  - `-DUSE_LUA_SQLITE3`
  - `-DUSE_LUA_FILE_SYSTEM`

##### Add the following sources

  - src/mod\_lua.inl
  - src/third\_party/lua-5.2.4/src
     + lapi.c
     + lauxlib.c
     + lbaselib.c
     + lbitlib.c
     + lcode.c
     + lcorolib.c
     + lctype.c
     + ldblib.c
     + ldebug.c
     + ldo.c
     + ldump.c
     + lfunc.c
     + lgc.c
     + linit.c
     + liolib.c
     + llex.c
     + lmathlib.c
     + lmem.c
     + loadlib.c
     + lobject.c
     + lopcodes.c
     + loslib.c
     + lparser.c
     + lstate.c
     + lstring.c
     + lstrlib.c
     + ltable.c
     + ltablib.c
     + ltm.c
     + lundump.c
     + lvm.c
     + lzio.c
  - src/third\_party/sqlite3.c
  - src/third\_party/sqlite3.h
  - src/third\_party/lsqlite3.c
  - src/third\_party/lfs.c
  - src/third\_party/lfs.h

This build is valid for Lua version Lua 5.2. It is also possible to build with Lua 5.1 (including LuaJIT) or Lua 5.3.


JavaScript Support
------

CivetWeb can be built with server side JavaScript support by including the Duktape library.


CivetWeb internals
------

CivetWeb is multithreaded web server. `mg_start()` function allocates
web server context (`struct mg_context`), which holds all information
about web server instance:

- configuration options. Note that CivetWeb makes internal copies of
  passed options.
- SSL context, if any
- user-defined callbacks
- opened listening sockets
- a queue for accepted sockets
- mutexes and condition variables for inter-thread synchronization

When `mg_start()` returns, all initialization is guaranteed to be complete
(e.g. listening ports are opened, SSL is initialized, etc). `mg_start()` starts
some threads: a master thread, that accepts new connections, and several
worker threads, that process accepted connections. The number of worker threads
is configurable via `num_threads` configuration option. That number puts a
limit on number of simultaneous requests that can be handled by CivetWeb.
If you embed CivetWeb into a program that uses SSL outside CivetWeb as well,
you may need to initialize SSL before calling `mg_start()`, and set the pre-
processor define `SSL_ALREADY_INITIALIZED`. This is not required if SSL is
used only within CivetWeb.

When master thread accepts new a connection, a new accepted socket (described
by `struct socket`) it placed into the accepted sockets queue,
which has size of `MGSQLEN` (default 20).
Any idle worker thread can grab accepted sockets from that queue.
If all worker threads are busy, master thread can accept and queue up to
20 more TCP connections, filling up the queue.
In the attempt to queue even more accepted connection, the master thread blocks
until there is space in the queue. When the master thread is blocked on a
full queue, the operating system can also queue incoming connection.
The number is limited by the `listen()` call parameter,
which is `SOMAXCONN` and depends on the platform.

Worker threads are running in an infinite loop, which in a simplified form
looks something like this:

```C
    static void *worker_thread() {
      while (consume_socket()) {
        process_new_connection();
      }
    }
```

Function `consume_socket()` gets a new accepted socket from the CivetWeb socket
queue, atomically removing it from the queue. If the queue is empty,
`consume_socket()` blocks and waits until a new socket is placed in the queue
by the master thread.

`process_new_connection()` actually processes the
connection, i.e. reads the request, parses it, and performs appropriate action
depending on the parsed request.

Master thread uses `poll()` and `accept()` to accept new connections on
listening sockets. `poll()` is used to avoid `FD_SETSIZE` limitation of
`select()`. Since there are only a few listening sockets, there is no reason
to use hi-performance alternatives like `epoll()` or `kqueue()`. Worker
threads use blocking IO on accepted sockets for reading and writing data.
All accepted sockets have `SO_RCVTIMEO` and `SO_SNDTIMEO` socket options set
(controlled by the `request_timeout_ms` CivetWeb option, 30 seconds default)
which specifies a read/write timeout on client connections.


A minimal example
------

Initializing a HTTP server
```C
{
    /* Server context handle */
    struct mg_context *ctx;

    /* Initialize the library */
    mg_init_library(0);

    /* Start the server */
    ctx = mg_start(NULL, 0, NULL);

    /* Add some handler */
    mg_set_request_handler(ctx, "/hello", handler, "Hello world");

    ... Run the application ...
    
    /* Stop the server */
    mg_stop(ctx);

    /* Un-initialize the library */
    mg_exit_library();
}
```

A simple callback
```C
static int
handler(struct mg_connection *conn, void *ignored)
{
	const char *msg = "Hello world";
	unsigned long len = (unsigned long)strlen(msg);

	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\n"
	          "Content-Length: %lu\r\n"
	          "Content-Type: text/plain\r\n"
	          "Connection: close\r\n\r\n",
	          len);

	mg_write(conn, msg, len);

	return 200;
}
```

