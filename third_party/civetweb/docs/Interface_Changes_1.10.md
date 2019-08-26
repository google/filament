# Interface changes

## Proposed interface changes for future versions

Interface changes from 1.10 to 1.11 and/or later versions -
see also [this GitHub issue](https://github.com/civetweb/civetweb/issues/544).


### Server interface

#### S1: mg\_start / mg\_init\_library

Calling mg\_init\_library is recommended before calling mg\_start.

**Compatibility considerations:**
Initially, mg\_init\_library will be called implicitly if it has 
not been called before mg\_start.
If mg\_init\_library was not called, mg\_stop may leave memory leaks.

**Required Actions:**
Call mg\_init\_library manually to avoid a small memory leak when
closing the server.


#### S2: mg\_websocket\_write functions

Calling mg\_lock\_connection is no longer called implicitly
in mg\_websocket\_write functions. 
If you use websocket write functions them from two threads,
you must call mg\_lock\_connection explicitly, just like for any
other connection.

This is an API harmonization issue.

**Compatibility considerations:**
If a websocket connection was used in only one thread, there is
no incompatibility. If a websocket connection was used in multiple
threads, the user has to add the mg\_lock\_connection before and
the mg\_unlock\_connection after the websocket write call.

**Required Actions:**
Call mg\_lock\_connection and mg\_unlock\_connection manually
when using mg\_websocket\_write.


#### S3: open\_file member of mg\_callbacks

Memory mapped files are a relic from before `mg_set_request_handler`
was introduced in CivetWeb 1.4 (September 2013).
Is "file in memory" still a useful feature or dead code? See this
[discussion](https://groups.google.com/forum/#!topic/civetweb/h9HT4CmeYqI).
Since it is not widely used, and a burden in maintenance, the
"file in memory" should be completely removed, including removing
the open\_file member of mg\_callbacks.


**Compatibility considerations:**
Removing "file in memory" will require code using open\_file to be changed.
A possible replacement by mg\_set\_request\_handler is sketched in
[this comment](https://github.com/civetweb/civetweb/issues/440#issuecomment-290531238).

**Required Actions:**
Modify code using open\_file by using request handlers.


#### S4: Support multiple hostnames and SNI

TLS [Server Name Identification (SNI)](https://en.wikipedia.org/wiki/Server_Name_Indication)
allows to host different domains with different X.509 certificates
on the same physical server (same IP+port). In order to support this,
some configurations (like authentication\_domain, ssl\_certificate, 
document\_root and may others) need to be specified multiple times - 
once for each domain hosted 
(see [535](https://github.com/civetweb/civetweb/issues/535)).

The current configuration model does not account for SNI, so it needs
to be extended to support configuration of multiple instances.

**Compatibility considerations:**
To be defined as soon as possible solutions are evaluated.


#### S5: IPv6 support for access\_control\_list and throttle

The current configuration for access\_control\_list and throttle only
works for IPv4 addresses. If server and client support 
[IPv6](https://en.wikipedia.org/wiki/IPv6_address) as well,
there is no way to add a client to the throttle or access list.
The current configuration syntax isn't really comfortable for IPv4
either.
Combined with hosting multiple domains (and SNI), different domains
may have different block/throttle configurations as well - this has
to be considered in a new configuration as well.

**Compatibility considerations:**
To be defined as soon as possible solutions are evaluated.


### Client interface

#### C1: mg\_init\_library

Calling mg\_init\_library is required before calling any client
function. In particular, the TLS initialization must be done
before using mg\_connect\_client\_secure.

**Compatibility considerations:**
Some parts of the client interface did not work, if mg\_start
was not called before. Now it works after calling
mg\_init\_library - this is not an incompatibility.


#### C2: mg\_connect\_client (family)

mg\_connect\_client needs several new parameters (options).

Details are to be defined.

mg\_connect\_client and mg\_download should return a different kind of
mg_connection than used in server callbacks. At least, there should
be a function mg\_get\_response\_info, instead of using 
mg\_get\_request\_info, and getting the HTTP response code from the
server by looking into the uri member of struct mg\_request\_info.


### General interfaces

#### G1: `size_t` in all interface

Having `size_t` in interfaces while building for 32 and 64 bit
complicates maintenance in an unnecessary way 
(see [498](https://github.com/civetweb/civetweb/issues/498)).

Replace all data sizes by 64 bit integers.


#### G2: Pattern definition

The current definition of pattern matching is problematic
(see [499](https://github.com/civetweb/civetweb/issues/499)).

Find and implement a new definition.


