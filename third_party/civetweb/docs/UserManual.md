Overview
=====

CivetWeb is a small and easy to use web server.
It may be embedded into C/C++ host applications or used as a stand-alone
server. See `Embedding.md` for information on embedding CivetWeb into
host applications.

The stand-alone server is self-contained, and does not require any external
software to run. Some Windows users may need to install the
[Visual C++ Redistributable](http://www.microsoft.com/en-us/download/details.aspx?id=30679).


Installation
----

On Windows, UNIX and Mac, the CivetWeb stand-alone executable may be started
from the command line.
Running `CivetWeb` in a terminal, optionally followed by configuration parameters
(`CivetWeb [OPTIONS]`) or a configuration file name (`CivetWeb [config_file_name]`),
starts the web server.

For UNIX and Mac, CivetWeb does not detach from the terminal.
Pressing `Ctrl-C` keys will stop the server.

On Windows, CivetWeb iconifies itself to the system tray icon when started.
Right-clicking on the icon pops up a menu, where it is possible to stop
CivetWeb, configure it, or install it as Windows service.

When started without options, the server exposes the local directory at
[http](http://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol) port 8080.
Thus, the easiest way to share a folder on Windows is to copy `CivetWeb.exe`
to this folder, double-click the exe, and launch a browser at
[http://localhost:8080](http://localhost:8080). Note that 'localhost' should
be changed to a machine's name if a folder is accessed from another computer.

When started, CivetWeb first searches for the configuration file.
If a configuration file is specified explicitly in the command line, i.e.
`CivetWeb path_to_config_file`, then the specified configuration file is used.
Otherwise, CivetWeb will search for the file `CivetWeb.conf` in the same directory that
the executable is located, and use it. This configuration file is optional.

The configuration file is a sequence of lines, each line containing one
command line argument name and the corresponding value.
Empty lines, and lines beginning with `#`, are ignored.
Here is the example of a `CivetWeb.conf` file:

    document_root c:\www
    listening_ports 80,443s
    ssl_certificate c:\CivetWeb\ssl_cert.pem

When a configuration file is used, additional command line arguments may
override the configuration file settings.
All command line arguments must start with `-`.

For example: The above `CivetWeb.conf` file is used, and CivetWeb started as
`CivetWeb -document_root D:\web`. Then the `D:\web` directory will be served
as document root, because command line options take priority over the
configuration file. The configuration options section below provides a good
overview of CivetWeb features.

Note that configuration options on the command line must start with `-`,
but their names are the same as in the config file. All option names are
listed in the next section. Thus, the following two setups are equivalent:

    # Using command line arguments
    $ CivetWeb -listening_ports 1234 -document_root /var/www

    # Using config file
    $ cat CivetWeb.conf
    listening_ports 1234
    document_root /var/www
    $ CivetWeb

CivetWeb can also be used to modify `.htpasswd` passwords files:

    CivetWeb -A <htpasswd_file> <realm> <user> <passwd>


Configuration Options
----

Below is a list of configuration options understood by CivetWeb.
Every option is followed by it's default value. If a default value is not
present, then the default is empty.

## Pattern matching in configuration options

CivetWeb uses shell-like glob patterns for several configuration options,
e.g., CGI, SSI and Lua script files are recognized by the file name pattern. 
Pattern match starts at the beginning of the string, so essentially
patterns are prefix patterns. Syntax is as follows:

     **      Matches everything
     *       Matches everything but slash character, '/'
     ?       Matches any character
     $       Matches the end of the string
     |       Matches if pattern on the left side or the right side matches.

All other characters in the pattern match themselves. Examples:

    **.cgi$      Any string that ends with .cgi
    /foo         Any string that begins with /foo
    **a$|**b$    Any string that ends with a or b


## Options from `civetweb.c`

The following options are supported in `civetweb.c`. They can be used for
the stand-alone executable as well as for applications embedding CivetWeb.
The stand-alone executable supports some additional options: see *Options from `main.c`*.
The options are explained in alphabetic order - for a quick start, check
*document\_root*, *listening\_ports*, *error\_log\_file* and (for HTTPS) *ssl\_certificate*.

### access\_control\_allow\_headers `*`
Access-Control-Allow-Headers header field, used for cross-origin resource
sharing (CORS) pre-flight requests.
See the [Wikipedia page on CORS](http://en.wikipedia.org/wiki/Cross-origin_resource_sharing).

If set to an empty string, pre-flights will not allow additional headers.
If set to "*", the pre-flight will allow whatever headers have been requested.
If set to a comma separated list of valid HTTP headers, the pre-flight will return
exactly this list as allowed headers.
If set in any other way, the result is unspecified.

### access\_control\_allow\_methods `*`
Access-Control-Allow-Methods header field, used for cross-origin resource
sharing (CORS) pre-flight requests.
See the [Wikipedia page on CORS](http://en.wikipedia.org/wiki/Cross-origin_resource_sharing).

If set to an empty string, pre-flights will not be supported directly by the server,
but scripts may still support pre-flights by handling the OPTIONS method properly.
If set to "*", the pre-flight will allow whatever method has been requested.
If set to a comma separated list of valid HTTP methods, the pre-flight will return
exactly this list as allowed method.
If set in any other way, the result is unspecified.

### access\_control\_allow\_origin `*`
Access-Control-Allow-Origin header field, used for cross-origin resource
sharing (CORS).
See the [Wikipedia page on CORS](http://en.wikipedia.org/wiki/Cross-origin_resource_sharing).

### access\_control\_list
An Access Control List (ACL) allows restrictions to be put on the list of IP
addresses which have access to the web server. In the case of the CivetWeb
web server, the ACL is a comma separated list of IP subnets, where each
subnet is pre-pended by either a `-` or a `+` sign. A plus sign means allow,
where a minus sign means deny. If a subnet mask is omitted, such as `-1.2.3.4`,
this means to deny only that single IP address.

Subnet masks may vary from 0 to 32, inclusive. The default setting is to allow
all accesses. On each request the full list is traversed, and
the last match wins. Examples:

    -0.0.0.0/0,+192.168/16    deny all accesses, only allow 192.168/16 subnet

To learn more about subnet masks, see the
[Wikipedia page on Subnetwork](http://en.wikipedia.org/wiki/Subnetwork).

### access\_log\_file
Path to a file for access logs. Either full path, or relative to the current
working directory. If absent (default), then accesses are not logged.

### additional\_header
Send additional HTTP response header line for every request.
The full header line including key and value must be specified, excluding the carriage return line feed.

Example (used as command line option): 
`-additional_header "X-Frame-Options: SAMEORIGIN"`

This option can be specified multiple times. All specified header lines will be sent.

### allow\_index\_script\_resource `no`
Index scripts (like `index.cgi` or `index.lua`) may have script handled resources.

It this feature is activated, that /some/path/file.ext might be handled by:
  1. /some/path/file.ext (with PATH\_INFO='/', if ext = cgi)
  2. /some/path/index.lua with mg.request\_info.path\_info='/file.ext'
  3. /some/path/index.cgi with PATH\_INFO='/file.ext'
  4. /some/path/index.php with PATH\_INFO='/file.ext'
  5. /some/index.lua with mg.request\_info.path\_info=='/path/file.ext'
  6. /some/index.cgi with PATH\_INFO='/path/file.ext'
  7. /some/index.php with PATH\_INFO='/path/file.ext'
  8. /index.lua with mg.request\_info.path\_info=='/some/path/file.ext'
  9. /index.cgi with PATH\_INFO='/some/path/file.ext'
  10. /index.php with PATH\_INFO='/some/path/file.ext'

Note: This example is valid, if the default configuration values for
`index_files`, `cgi_pattern` and `lua_script_pattern` are used, 
and the server is built with CGI and Lua support enabled.

If this feature is not activated, only the first file (/some/path/file.cgi) will be accepted.

Note: This parameter affects only index scripts. A path like /here/script.cgi/handle/this.ext
will call /here/script.cgi with PATH\_INFO='/handle/this.ext', no matter if this option is set to `yes` or `no`. 

This feature can be used to completely hide the script extension from the URL.

### allow\_sendfile\_call `yes`
This option can be used to enable or disable the use of the Linux `sendfile` system call. 
It is only available for Linux systems and only affecting HTTP (not HTTPS) connections 
if `throttle` is not enabled. 
While using the `sendfile` call will lead to a performance boost for HTTP connections, 
this call may be broken for some file systems and some operating system versions.

### authentication\_domain `mydomain.com`
Authorization realm used for HTTP digest authentication. This domain is
used in the encoding of the `.htpasswd` authorization files as well.
Changing the domain retroactively will render the existing passwords useless.

### case\_sensitive `no`
This option can be uset to enable case URLs for Windows servers. 
It is only available for Windows systems.  Windows file systems are not case sensitive, 
but they still store the file name including case.
If this option is set to `yes`, the comparison for URIs and Windows file names will be case sensitive.

### cgi\_environment
Extra environment variables to be passed to the CGI script in
addition to standard ones. The list must be comma-separated list
of name=value pairs, like this: `VARIABLE1=VALUE1,VARIABLE2=VALUE2`.

### cgi\_interpreter
Path to an executable to use as CGI interpreter for __all__ CGI scripts
regardless of the script file extension. If this option is not set (which is
the default), CivetWeb looks at first line of a CGI script,
[shebang line](http://en.wikipedia.org/wiki/Shebang_(Unix\)), for an
interpreter (not only on Linux and Mac but also for Windows).

For example, if both PHP and Perl CGIs are used, then
`#!/path/to/php-cgi.exe` and `#!/path/to/perl.exe` must be first lines of the
respective CGI scripts. Note that paths should be either full file paths,
or file paths relative to the current working directory of the CivetWeb
server. If CivetWeb is started by mouse double-click on Windows, the current
working directory is the directory where the CivetWeb executable is located.

If all CGIs use the same interpreter, for example they are all PHP, it is
more efficient to set `cgi_interpreter` to the path to `php-cgi.exe`.
The shebang line in the CGI scripts can be omitted in this case.
Note that PHP scripts must use `php-cgi.exe` as executable, not `php.exe`.

### cgi\_pattern `**.cgi$|**.pl$|**.php$`
All files that match `cgi_pattern` are treated as CGI files. The default pattern
allows CGI files be anywhere. To restrict CGIs to a certain directory,
use `/path/to/cgi-bin/**.cgi` as the pattern. Note that the full file path is
matched against the pattern, not the URI.

### cgi\_timeout\_ms
Maximum allowed runtime for CGI scripts.  CGI processes are terminated by
the server after this time.  The default is "no timeout", so scripts may
run or block for undefined time.

### decode\_url `yes`
URL encoded request strings are decoded in the server, unless it is disabled
by setting this option to `no`.

### document\_root `.`
A directory to serve. By default, the current working directory is served.
The current directory is commonly referenced as dot (`.`).
It is recommended to use an absolute path for document\_root, in order to 
avoid accidentally serving the wrong directory.

### enable\_auth\_domain\_check `yes`
When using absolute URLs, verify the host is identical to the authentication\_domain.
If enabled, requests to absolute URLs will only be processed 
if they are directed to the domain. If disabled, absolute URLs to any host
will be accepted.

### enable\_directory\_listing `yes`
Enable directory listing, either `yes` or `no`.

### enable\_keep\_alive `no`
Enable connection keep alive, either `yes` or `no`.

Allows clients to reuse TCP connection for subsequent HTTP requests, 
which improves performance.
For this to work when using request handlers it is important to add the
correct Content-Length HTTP header for each request. If this is forgotten the
client will time out.

Note: If you set keep\_alive to `yes`, you should set keep\_alive\_timeout\_ms
to some value > 0 (e.g. 500). If you set keep\_alive to `no`, you should set
keep\_alive\_timeout\_ms to 0. Currently, this is done as a default value,
but this configuration is redundant. In a future version, the keep\_alive 
configuration option might be removed and automatically set to `yes` if 
a timeout > 0 is set.

### enable\_websocket\_ping\_pong `no`
If this configuration value is set to `yes`, the server will send a
websocket PING message to a websocket client, once the timeout set by
websocket\_timeout\_ms expires. Clients (Web browsers) supporting this
feature will reply with a PONG message.

If this configuration value is set to `no`, the websocket server will
close the connection, once the timeout expires.

Note: This configuration value only exists, if the server has been built 
with websocket support enabled.

### error\_log\_file
Path to a file for error logs. Either full path, or relative to the current
working directory. If absent (default), then errors are not logged.

### error\_pages
This option may be used to specify a directory for user defined error pages.
To specify a directory, make sure the name ends with a backslash (Windows) 
or slash (Linux, MacOS, ...).
The error pages may be specified for an individual http status code (e.g.,
404 - page requested by the client not found), a group of http status codes
(e.g., 4xx - all client errors) or all errors. The corresponding error pages
must be called error404.ext, error4xx.ext or error.ext, whereas the file
extension may be one of the extensions specified for the index_files option.
See the [Wikipedia page on HTTP status codes](http://en.wikipedia.org/wiki/HTTP_status_code).

### extra\_mime\_types
Extra mime types, in the form `extension1=type1,exten-sion2=type2,...`.
See the [Wikipedia page on Internet media types](http://en.wikipedia.org/wiki/Internet_media_type).
Extension must include a leading dot. Example:
`.cpp=plain/text,.java=plain/text`

### global\_auth\_file
Path to a global passwords file, either full path or relative to the current
working directory. If set, per-directory `.htpasswd` files are ignored,
and all requests are authorized against that file.

The file has to include the realm set through `authentication_domain` and the
password in digest format:

    user:realm:digest
    test:test.com:ce0220efc2dd2fad6185e1f1af5a4327

Password files may be generated using `CivetWeb -A` as explained above, or
online tools e.g. [this generator](http://www.askapache.com/online-tools/htpasswd-generator).

### hide\_files\_patterns
A pattern for the files to hide. Files that match the pattern will not
show up in directory listing and return `404 Not Found` if requested. Pattern
must be for a file name only, not including directory names. Example:

    CivetWeb -hide_files_patterns secret.txt|**.hide

Note: hide\_file\_patterns uses the pattern described above. If you want to
hide all files with a certain extension, make sure to use **.extension
(not just *.extension).

### index\_files `index.xhtml,index.html,index.htm,index.cgi,index.shtml,index.php`
Comma-separated list of files to be treated as directory index files.
If more than one matching file is present in a directory, the one listed to the left
is used as a directory index.

In case built-in Lua support has been enabled, `index.lp,index.lsp,index.lua`
are additional default index files, ordered before `index.cgi`.

### keep\_alive\_timeout\_ms `500` or `0`
Idle timeout between two requests in one keep-alive connection.
If keep alive is enabled, multiple requests using the same connection 
are possible. This reduces the overhead for opening and closing connections
when loading several resources from one server, but it also blocks one port
and one thread at the server during the lifetime of this connection.
Unfortunately, browsers do not close the keep-alive connection after loading
all resources required to show a website.
The server closes a keep-alive connection, if there is no additional request
from the client during this timeout.

Note: if enable\_keep\_alive is set to `no` the value of 
keep\_alive\_timeout\_ms should be set to `0`, if enable\_keep\_alive is set 
to `yes`, the value of keep\_alive\_timeout\_ms must be >0.
Currently keep\_alive\_timeout\_ms is ignored if enable\_keep\_alive is no,
but future versions may drop the enable\_keep\_alive configuration value and
automatically use keep-alive if keep\_alive\_timeout\_ms is not 0.

### linger\_timeout\_ms
Set TCP socket linger timeout before closing sockets (SO\_LINGER option).
The configured value is a timeout in milliseconds. Setting the value to 0
will yield in abortive close (if the socket is closed from the server side).
Setting the value to -1 will turn off linger.
If the value is not set (or set to -2), CivetWeb will not set the linger
option at all.

Note: For consistency with other timeout configurations, the value is 
configured in milliseconds. However, the TCP socket layer usually only
offers a timeout in seconds, so the value should be an integer multiple
of 1000.

### lua\_background\_script
Experimental feature, and subject to change.
Run a Lua script in the background, independent from any connection.
The script is started before network access to the server is available.
It can be used to prepare the document root (e.g., update files, compress
files, ...), check for external resources, remove old log files, etc.

The Lua state remains open until the server is stopped.
In the future, some callback functions will be available to notify the
script on changes of the server state. See example lua script :
[background.lua](https://github.com/civetweb/civetweb/blob/master/test/background.lua).

Additional functions available in background script :
sleep, root path, script name, is terminated

### lua\_background\_script\_params `param1=1,param2=2`
Can add dynamic parameters to background script.
Parameters mapped to global 'mg' table 'params' field.

### lua\_preload\_file
This configuration option can be used to specify a Lua script file, which
is executed before the actual web page script (Lua script, Lua server page
or Lua websocket). It can be used to modify the Lua environment of all web
page scripts, e.g., by loading additional libraries or defining functions
required by all scripts.
It may be used to achieve backward compatibility by defining obsolete
functions as well.

### lua\_script\_pattern `"**.lua$`
A pattern for files that are interpreted as Lua scripts by the server.
In contrast to Lua server pages, Lua scripts use plain Lua syntax.
An example can be found in the test directory.

### lua\_server\_page\_pattern `**.lp$|**.lsp$`
Files matching this pattern are treated as Lua server pages.
In contrast to Lua scripts, the content of a Lua server pages is delivered
directly to the client. Lua script parts are delimited from the standard
content by including them between <? and ?> tags.
An example can be found in the test directory.

### lua\_websocket\_pattern `"**.lua$`
A pattern for websocket script files that are interpreted as Lua scripts by the server.

### listening\_ports `8080`
Comma-separated list of ports to listen on. If the port is SSL, a
letter `s` must be appended, for example, `80,443s` will open
port 80 and port 443, and connections on port 443 will be SSL-ed.
For non-SSL ports, it is allowed to append letter `r`, meaning 'redirect'.
Redirect ports will redirect all their traffic to the first configured
SSL port. For example, if `listening_ports` is `80r,443s`, then all
HTTP traffic coming at port 80 will be redirected to HTTPS port 443.

It is possible to specify an IP address to bind to. In this case,
an IP address and a colon must be pre-pended to the port number.
For example, to bind to a loopback interface on port 80 and to
all interfaces on HTTPS port 443, use `127.0.0.1:80,443s`.

If the server is built with IPv6 support, `[::]:8080` can be used to
listen to IPv6 connections to port 8080. IPv6 addresses of network
interfaces can be specified as well,
e.g. `[::1]:8080` for the IPv6 loopback interface.

[::]:80 will bind to port 80 IPv6 only. In order to use port 80 for
all interfaces, both IPv4 and IPv6, use either the configuration
`80,[::]:80` (create one socket for IPv4 and one for IPv6 only),
or `+80` (create one socket for both, IPv4 and IPv6). 
The `+` notation to use IPv4 and IPv6 will only work if no network
interface is specified. Depending on your operating system version
and IPv6 network environment, some configurations might not work
as expected, so you have to test to find the configuration most 
suitable for your needs. In case `+80` does not work for your
environment, you need to use `80,[::]:80`.

It is possible to use network interface addresses (e.g., `192.0.2.3:80`,
`[2001:0db8::1234]:80`). To get a list of available network interface
addresses, use `ipconfig` (in a `cmd` window in Windows) or `ifconfig` 
(in a Linux shell).
Alternatively, you could use the hostname for an interface. Check the 
hosts file of your operating system for a proper hostname 
(for Windows, usually found in C:\Windows\System32\drivers\etc\, 
for most Linux distributions: /etc/hosts). E.g., to bind the IPv6 
local host, you could use `ip6-localhost:80`. This translates to 
`[::1]:80`. Beside the hosts file, there are several other name
resolution services. Using your hostname might bind you to the
localhost or an external interface. You could also try `hostname.local`,
if the proper network services are installed (Zeroconf, mDNS, Bonjour, 
Avahi). When using a hostname, you need to test in your particular network
environment - in some cases, you might need to resort to a fixed IP address.

### max\_request\_size `16384`
Size limit for HTTP request headers and header data returned from CGI scripts, in Bytes.
A buffer of the configured size is pre allocated for every worker thread.
max\_request\_size limits the HTTP header, including query string and cookies,
but it does not affect the HTTP body length.
The server has to read the entire header from a client or from a CGI script,
before it is able to process it. In case the header is longer than max\_request\_size, 
the request is considered as invalid or as DoS attack.
The configuration value is approximate, the real limit might be a few bytes off.
The minimum is 1024 (1 kB).

### num\_threads `50`
Number of worker threads. CivetWeb handles each incoming connection in a
separate thread. Therefore, the value of this option is effectively the number
of concurrent HTTP connections CivetWeb can handle.

### protect\_uri
Comma separated list of URI=PATH pairs, specifying that given
URIs must be protected with password files specified by PATH.
All Paths must be full file paths.

### put\_delete\_auth\_file
Passwords file for PUT and DELETE requests. Without a password file, it will not
be possible to PUT new files to the server or DELETE existing ones. PUT and
DELETE requests might still be handled by Lua scripts and CGI paged.

### request\_timeout\_ms `30000`
Timeout for network read and network write operations, in milliseconds.
If a client intends to keep long-running connection, either increase this
value or (better) use keep-alive messages.

### run\_as\_user
Switch to given user credentials after startup. Usually, this option is
required when CivetWeb needs to bind on privileged ports on UNIX. To do
that, CivetWeb needs to be started as root. From a security point of view,
running as root is not advisable, therefore this option can be used to drop
privileges. Example:

    civetweb -listening_ports 80 -run_as_user webserver

### ssi\_pattern `**.shtml$|**.shtm$`
All files that match `ssi_pattern` are treated as Server Side Includes (SSI).

SSI is a simple interpreted server-side scripting language which is most
commonly used to include the contents of another file in a web page.
It can be useful when it is desirable to include a common piece
of code throughout a website, for example, headers and footers.

In order for a webpage to recognize an SSI-enabled HTML file, the filename
should end with a special extension, by default the extension should be
either `.shtml` or `.shtm`. These extensions may be changed using the
`ssi_pattern` option.

Unknown SSI directives are silently ignored by CivetWeb. Currently, two SSI
directives are supported, `<!--#include ...>` and
`<!--#exec "command">`. Note that the `<!--#include ...>` directive supports
three path specifications:

    <!--#include virtual="path">  Path is relative to web server root
    <!--#include abspath="path">  Path is absolute or relative to
                                  web server working dir
    <!--#include file="path">,    Path is relative to current document
    <!--#include "path">

The `include` directive may be used to include the contents of a file or the
result of running a CGI script. The `exec` directive is used to execute a
command on a server, and show the output that would have been printed to
stdout (the terminal window) otherwise. Example:

    <!--#exec "ls -l" -->

For more information on Server Side Includes, take a look at the Wikipedia:
[Server Side Includes](http://en.wikipedia.org/wiki/Server_Side_Includes)

### ssl\_ca\_path
Name of a directory containing trusted CA certificates. Each file in the
directory must contain only a single CA certificate. The files must be named
by the subject name’s hash and an extension of “.0”. If there is more than one
certificate with the same subject name they should have extensions ".0", ".1",
".2" and so on respectively.

### ssl\_ca\_file
Path to a .pem file containing trusted certificates. The file may contain
more than one certificate.

### ssl\_certificate
Path to the SSL certificate file. This option is only required when at least
one of the `listening\_ports` is SSL. The file must be in PEM format,
and it must have both, private key and certificate, see for example
[ssl_cert.pem](https://github.com/civetweb/civetweb/blob/master/resources/ssl_cert.pem)
A description how to create a certificate can be found in doc/OpenSSL.md

### ssl\_certificate\_chain
Path to an SSL certificate chain file. As a default, the ssl\_certificate file is used.

### ssl\_cipher\_list
List of ciphers to present to the client. Entries should be separated by
colons, commas or spaces.

    ALL           All available ciphers
    ALL:!eNULL    All ciphers excluding NULL ciphers
    AES128:!MD5   AES 128 with digests other than MD5

See [this entry](https://www.openssl.org/docs/manmaster/apps/ciphers.html) in
OpenSSL documentation for full list of options and additional examples.

### ssl\_default\_verify\_paths `yes`
Loads default trusted certificates locations set at openssl compile time.

### ssl\_protocol\_version `0`
Sets the minimal accepted version of SSL/TLS protocol according to the table:

Protocols | Value
------------ | -------------
SSL2+SSL3+TLS1.0+TLS1.1+TLS1.2  | 0
SSL3+TLS1.0+TLS1.1+TLS1.2  | 1
TLS1.0+TLS1.1+TLS1.2 | 2
TLS1.1+TLS1.2 | 3
TLS1.2 | 4

More recent versions of OpenSSL include support for TLS version 1.3. 
To use TLS1.3 only, set ssl\_protocol\_version to 5.

### ssl\_short\_trust `no`
Enables the use of short lived certificates. This will allow for the certificates
and keys specified in `ssl_certificate`, `ssl_ca_file` and `ssl_ca_path` to be
exchanged and reloaded while the server is running.

In an automated environment it is advised to first write the new pem file to
a different filename and then to rename it to the configured pem file name to
increase performance while swapping the certificate.

Disk IO performance can be improved when keeping the certificates and keys stored
on a tmpfs (linux) on a system with very high throughput.

### ssl\_verify\_depth `9`
Sets maximum depth of certificate chain. If client's certificate chain is longer
than the depth set here connection is refused.

### ssl\_verify\_peer `no`
Enable client's certificate verification by the server.

### static\_file\_max\_age `3600`
Set the maximum time (in seconds) a cache may store a static files.

This option will set the `Cache-Control: max-age` value for static files.
Dynamically generated content, i.e., content created by a script or callback,
must send cache control headers by themselves.

A value >0 corresponds to a maximum allowed caching time in seconds.
This value should not exceed one year (RFC 2616, Section 14.21).
A value of 0 will send "do not cache" headers for all static files.
For values <0 and values >31622400, the behaviour is undefined.

### strict\_transport\_security\_max\_age

Set the `Strict-Transport-Security` header, and set the `max-age` value.
This instructs web browsers to interact with the server only using HTTPS,
never by HTTP. If set, it will be sent for every request handled directly
by the server, except scripts (CGI, Lua, ..) and callbacks. They must 
send HTTP headers on their own.

The time is specified in seconds. If this configuration is not set, 
or set to -1, no `Strict-Transport-Security` header will be sent.
For values <-1 and values >31622400, the behaviour is undefined.

### tcp\_nodelay `0`
Enable TCP_NODELAY socket option on client connections.

If set the socket option will disable Nagle's algorithm on the connection
which means that packets will be sent as soon as possible instead of waiting
for a full buffer or timeout to occur.

    0    Keep the default: Nagel's algorithm enabled
    1    Disable Nagel's algorithm for all sockets

### throttle
Limit download speed for clients.  `throttle` is a comma-separated
list of key=value pairs, where key could be:

    *                   limit speed for all connections
    x.x.x.x/mask        limit speed for specified subnet
    uri_prefix_pattern  limit speed for given URIs

The value is a floating-point number of bytes per second, optionally
followed by a `k` or `m` character, meaning kilobytes and
megabytes respectively. A limit of 0 means unlimited rate. The
last matching rule wins. Examples:

    *=1k,10.0.0.0/8=0   limit all accesses to 1 kilobyte per second,
                        but give connections the from 10.0.0.0/8 subnet
                        unlimited speed

    /downloads/=5k      limit accesses to all URIs in `/downloads/` to
                        5 kilobytes per second. All other accesses are unlimited

### url\_rewrite\_patterns
Comma-separated list of URL rewrites in the form of
`uri_pattern=file_or_directory_path`. When CivetWeb receives any request,
it constructs the file name to show by combining `document_root` and the URI.
However, if the rewrite option is used and `uri_pattern` matches the
requested URI, then `document_root` is ignored. Instead,
`file_or_directory_path` is used, which should be a full path name or
a path relative to the web server's current working directory. Note that
`uri_pattern`, as all CivetWeb patterns, is a prefix pattern.

This makes it possible to serve many directories outside from `document_root`,
redirect all requests to scripts, and do other tricky things. For example,
to redirect all accesses to `.doc` files to a special script, do:

    CivetWeb -url_rewrite_patterns **.doc$=/path/to/cgi-bin/handle_doc.cgi

Or, to imitate support for user home directories, do:

    CivetWeb -url_rewrite_patterns /~joe/=/home/joe/,/~bill=/home/bill/

### websocket\_root
In case CivetWeb is built with Lua and websocket support, Lua scripts may
be used for websockets as well. Since websockets use a different URL scheme
(ws, wss) than other http pages (http, https), the Lua scripts used for
websockets may also be served from a different directory. By default,
the document\_root is used as websocket\_root as well.

### websocket\_timeout\_ms
Timeout for network read and network write operations for websockets, WS(S),
in milliseconds. If this value is not set, the value of request\_timeout\_ms
is used for HTTP(S) as well as for WS(S). In case websocket\_timeout\_ms is
set, HTTP(S) and WS(S) can use different timeouts.

Note: This configuration value only exists, if the server has been built 
with websocket support enabled.


## Options from `main.c`

The following options are supported in `main.c`, the additional source file for
the stand-alone executable. These options are not supported by other applications
embedding `civetweb.c`, unless they are added explicitly.

### title
Use the configured string as a server name.  For Windows, this will be shown as
the window title.

### icon
For Windows, show this icon file in the systray, replacing the CivetWeb standard
icon.  This option has no effect for Linux.

### website
For Windows, use this website as a link in the systray, replacing the default
link for CivetWeb.


### add\_domain
Option to load an additional configuration file, specifying an additional domain
to host.  To add multiple additional domains, use the add\_domain option 
multiple times with one configuration file for each domain.  
A domain configuration file may have the same options as the main server, with
some exceptions.  The options are passed to the `mg_start_domain` API function.


Scripting
----

# Lua Scripts and Lua Server Pages
Pre-built Windows and Mac CivetWeb binaries have built-in Lua scripting
support as well as support for Lua Server Pages.

Lua scripts (default extension: *.lua) use plain Lua syntax.
The body of the script file is not sent directly to the client,
the Lua script must send header and content of the web page by calling
the function mg.write(text).

Lua Server Pages (default extensions: *.lsp, *.lp) are html pages containing
script elements similar to PHP, using the Lua programming language instead of
PHP. Lua script elements must be enclosed in `<?  ?>` blocks, and can appear
anywhere on the page. Furthermore, Lua Server Pages offer the opportunity to
insert the content of a variable by enclosing the Lua variable name in
`<?=  ?>` blocks, similar to PHP.
For example, to print the current weekday name and the URI of the current
page, one can write:

    <p>
      <span>Today is:</span>
      <? mg.write(os.date("%A")) ?>
    </p>
    <p>
      URI is <?=mg.request_info.uri?>
    </p>

From version 1.11, CivetWeb supports "Kepler Syntax" in addition to the
traditional Lua pages syntax of CivetWeb. Kepler Syntax uses `<?lua ?>`
or `<% %>` blocks for script elements (corresponding to `<? ?>` above)
and `<?lua= ?>` or `<%= %>` for variable content (corresponding to `<?= ?>`).

    <ul>
       <% for key, value in pairs(mg.request_info) do %>
       <li> <%= key %>: <%= value %> </li>
       <% end %>
    </ul>
    
Currently the extended "Kepler Syntax" is available only for HTML (see 
note on HTTP headers below).

Lua is known for it's speed and small size. The default Lua version for
CivetWeb is Lua 5.2.4. The documentation for it can be found in the
[Lua 5.2 reference manual](http://www.lua.org/manual/5.2/). However,
CivetWeb can be built with Lua 5.1, 5.2, 5.3, 5.4 (currently pre-release)
and LuaJIT.

Note that this example uses function `mg.write()`, which sends data to the
web client. Using `mg.write()` is the way to generate web content from inside
Lua code. In addition to `mg.write()`, all standard Lua library functions
are accessible from the Lua code (please check the reference manual for
details). Lua functions working on files (e.g., `io.open`) use a path
relative to the working path of the CivetWeb process. The web server content
is located in the path `mg.document_root`.
Information on the request is available in the `mg.request_info`
object, like the request method, all HTTP headers, etcetera.

[page2.lua](https://github.com/civetweb/civetweb/blob/master/test/page2.lua)
is an example for a plain Lua script.

[page2.lp](https://github.com/civetweb/civetweb/blob/master/test/page2.lp)
is an example for a Lua Server Page.

[page4kepler.lp](https://github.com/civetweb/civetweb/blob/master/test/page4kepler.lp)
is a Lua Server Page showing "Kepler Syntax" in addition to traditional CivetWeb
Lua Server Pages syntax.

These examples show the content of the `mg.request_info` object as the page
content. Please refer to `struct mg_request_info` definition in
[CivetWeb.h](https://github.com/civetweb/civetweb/blob/master/include/civetweb.h)
to see additional information on the elements of the `mg.request_info` object.

CivetWeb also provides access to the [SQlite3 database](http://www.sqlite.org/)
through the [LuaSQLite3 interface](http://lua.sqlite.org/index.cgi/doc/tip/doc/lsqlite3.wiki)
in Lua. Examples are given in
[page.lua](https://github.com/civetweb/civetweb/blob/master/test/page.lua) and
[page.lp](https://github.com/civetweb/civetweb/blob/master/test/page.lp).


CivetWeb exports the following functions to Lua:

mg (table):

    mg.read()                   -- reads a chunk from POST data, returns it as a string
    mg.write(str)               -- writes string to the client
    mg.include(filename, [pathtype]) -- include another Lua Page file (Lua Pages only)
                                -- pathtype can be "abs", "rel"/"file" or "virt[ual]"
                                -- like defined for SSI #include
    mg.redirect(uri)            -- internal redirect to a given URI
    mg.onerror(msg)             -- error handler, can be overridden
    mg.version                  -- a string that holds CivetWeb version
    mg.document_root            -- a string that holds the document root directory
    mg.auth_domain              -- a string that holds the HTTP authentication domain
    mg.get_var(str, varname)    -- extract variable from (query) string
    mg.get_cookie(str, cookie)  -- extract cookie from a string
    mg.get_mime_type(filename)  -- get MIME type of a file
    mg.get_info(infotype)       -- get server status information
    mg.send_file(filename)      -- send a file, including all required HTTP headers
    mg.send_file_body(filename) -- send a file, excluding HTTP headers
    mg.url_encode(str)          -- URL encode a string
    mg.url_decode(str, [form])  -- URL decode a string. If form=true, replace + by space.
    mg.base64_encode(str)       -- BASE64 encode a string
    mg.base64_decode(str)       -- BASE64 decode a string
    mg.md5(str)                 -- return the MD5 hash of a string
    mg.keep_alive(bool)         -- allow/forbid to use http keep-alive for this request
    mg.request_info             -- a table with the following request information
         .remote_addr           -- IP address of the client as string
         .remote_port           -- remote port number
         .server_port           -- server port number
         .request_method        -- HTTP method (e.g.: GET, POST)
         .http_version          -- HTTP protocol version (e.g.: 1.1)
         .uri                   -- resource name
         .query_string          -- query string if present, nil otherwise
         .script_name           -- name of the Lua script
         .https                 -- true if accessed by https://, false otherwise
         .remote_user           -- user name if authenticated, nil otherwise

connect (function):

    -- Connect to the remote TCP server. This function is an implementation
    -- of simple socket interface. It returns a socket object with three
    -- methods: send, recv, close, which are synchronous (blocking).
    -- connect() throws an exception on connection error.
    -- use_ssl is not implemented.
    connect(host, port, use_ssl)

    -- Example of using connect() interface:
    local host = 'www.example.com'  -- IP address or domain name
    local ok, sock = pcall(connect, host, 80, 0)
    if ok then
      sock:send('GET / HTTP/1.0\r\n' ..
                'Host: ' .. host .. '\r\n\r\n')
      local reply = sock:recv()
      sock:close()
      -- reply now contains the web page http://www.example.com/
    end


All filename arguments are either absolute or relative to the CivetWeb working
directory (not the document root or the Lua script/page file).
    
To serve a Lua Page, CivetWeb creates a Lua context. That context is used for
all Lua blocks within the page. That means, all Lua blocks on the same page
share the same context. If one block defines a variable, for example, that
variable is visible in all blocks that follow.


**Important note on HTTP headers:**

Lua scripts MUST send HTTP headers themselves, e.g.:

    mg.write('HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n')

Lua Server Pages CAN send HTTP reply headers, like this:

    HTTP/1.0 200 OK
    Content-Type: text/html
        
    <html><body>
      ... the rest of the web page ...

or using Lua code:

    <? mg.write('HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n') ?>
    <html><body>
      ... the rest of the web page ...

or Lua Server Pages generating HTML content MAY skip the HTTP header lines.
In this case, CivetWeb automatically creates a "200 OK"/"Content-Type: text/html"
reply header. In this case, the document should start with "<!DOCTYPE html>" 
or "<html".

Currently the extended "Kepler Syntax" is available only for text/html pages
not sending their own HTTP headers. Thus, "Kepler Syntax" can only be used for
HTML pages, while traditional CivetWeb syntax can be used to send a content-type
header and generate any kind of file.


## Websockets for Lua
CivetWeb offers support for websockets in Lua as well. In contrast to plain
Lua scripts and Lua server pages, Lua websocket scripts are shared by all clients.

Lua websocket scripts must define a few functions:
    open(arg)    -- callback to accept or reject a connection
    ready(arg)   -- called after a connection has been established
    data(arg)    -- called when the server receives data from the client
    close(arg)   -- called when a websocket connection is closed
All function are called with one argument of type table with at least one field
"client" to identify the client. When "open" is called, the argument table additionally
contains the "request_info" table as defined above. For the "data" handler, an
additional field "data" is available. The functions "open", "ready" and "data"
must return true in order to keep the connection open.

Lua websocket pages do support single shot (timeout) and interval timers.

An example is shown in
[websocket.lua](https://github.com/civetweb/civetweb/blob/master/test/websocket.lua).


# Using CGI

Unlike some other web servers, CivetWeb does not require CGI scripts to be located
in a special directory. CGI scripts files are recognized by the file name pattern
and can be anywhere.

When using CGI, make sure your CGI file names match the `cgi\_pattern` parameter
configured for the server.
Furthermore, you must either configure a `cgi\_interpreter` to be used for all
CGI scripts, or all scripts must start with `#!` followed by the CGI
interpreter executable, e.g.: `#!/path/to/perl.exe` or `#!/bin/sh`.

See `cgi\_pattern` and `cgi\_interpreter` for more details.

It is possible to disable CGI completely by building the server with
the `NO\_CGI` define. Setting this define is required for operating 
systems not supporting `fork/exec` or `CreateProcess` (since CGI is
based on creating child processes, it will not be available on such
operating systems for principle reasons).

Every CGI request will spawn a new child process. Data sent from the
HTTP client to the server is passed to stdin of the child process,
while data written to stdout by the child process is sent back to the
HTTP client.

In case a CGI script cannot handle a particular request, it might
write a short error message to stderr instead of writing to stdout.
This error message is added to the server error log.

A script should not write to stderr after writing a reply header
to stdout. In case CGI libraries are writing to stderr (e.g., for
logging/debugging), the CGI script should redirect stderr to a 
user defined log file at the beginning of the script.


FAQ
----

# Common Problems
- PHP doesn't work - getting empty page, or 'File not found' error. The
  reason for that is wrong paths to the interpreter. Remember that with PHP,
  the correct interpreter is `php-cgi.exe` (`php-cgi` on UNIX).
  Solution: specify the full path to the PHP interpreter, e.g.:
    `CivetWeb -cgi_interpreter /full/path/to/php-cgi`

- `php-cgi` is unavailable, for example on Mac OS X. As long as the `php` binary is installed, you can run CGI programs in command line mode (see the example below). Note that in this mode, `$_GET` and friends will be unavailable, and you'll have to parse the query string manually using [parse_str](http://php.net/manual/en/function.parse-str.php) and the `QUERY_STRING` environmental variable.

        #!/usr/bin/php
        <?php
        echo "Content-Type: text/html\r\n\r\n";
        echo "Hello World!\n";
        ?>

- CivetWeb fails to start. If CivetWeb exits immediately when started, this
  usually indicates a syntax error in the configuration file
  (named `civetweb.conf` by default) or the command-line arguments.
  Syntax checking is omitted from CivetWeb to keep its size low. However,
  the Manual should be of help. Note: the syntax changes from time to time,
  so updating the config file might be necessary after executable update.
  Try to use the *error\_log\_file* option for details.

- Embedding with OpenSSL on Windows might fail because of calling convention.
  To force CivetWeb to use `__stdcall` convention, add `/Gz` compilation
  flag in Visual Studio compiler.
