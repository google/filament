
Examples
=====

Two examples show how to embed civetweb into a C ([embedded_c](https://github.com/civetweb/civetweb/tree/master/examples/embedded_c)) or a C++ ([embedded_cpp](https://github.com/civetweb/civetweb/tree/master/examples/embedded_cpp)) application.
The C++ wrapper only offers a subset of the full C API, thus the C example is more complete than the C++ example. These examples were not designed with security in mind, but to show how the API can be used in principle. For more information, see the [documentation](https://github.com/civetweb/civetweb/tree/master/docs). Some examples can also be found in the [test](https://github.com/civetweb/civetweb/tree/master/test) folder (but they are less documented and adapted to the needs of the test framework).

In addition, there is one example how to configure a HTTPS server, to comply with modern security standards ([https](https://github.com/civetweb/civetweb/tree/master/examples/https)). It does not hold any source, but only a configuration file and some documentation how to use it.

The [multidomain](https://github.com/civetweb/civetweb/tree/master/examples/multidomain) example demonstrates how to host multiple domains with different HTTPS certificates. It uses the standalone server (civetweb.c + main.c) and existing certificates.

The [ws_client](https://github.com/civetweb/civetweb/tree/master/examples/ws_client) example shows how to use the websocket client interface to communicate with an (external) websocket server. It uses the "echo demo" of [websocket.org](http://websocket.org/echo.html), and only works if this server is reachable.

Some no longer maintained examples can be found in the ["obsolete"](https://github.com/civetweb/civetweb/tree/master/examples/_obsolete) folder. It is not guaranteed that they work in the current version - they are kept for reference, but might be removed in the future.

All examples are subject to the MIT license (unless noted otherwise) - they come without warranty of any kind.

