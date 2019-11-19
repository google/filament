Release Notes v1.12
===
### Objectives: *TO BE DEFINED*

Changes
-------

- Support for flawed CGI interpreters returning only <LF> instead of <CR><LF>
- Add NO_FILESYSTEM flag for (embedded) system without any file system
- Several fixes for server side Lua scripts
- Disable SSL renegotiation for new OpenSSL version
- Allow to force TLSv1.3 (disable TLSv1.2)
- Prefer pre-compressed *.gz file, if it already exists
- Fix some #include statements for various compilers / OS / SDK versions
- Support for Linux Standard Base (LSB)
- Fixes to mg_get_*_info() API functions
- Fix some bugs/deficiencies in examples and tests
- Fix some static source code analysis warnings
- Add Conan package build
- Fix include for Lua pages in "Kepler Syntax"
- Replace some uses of deprecated Linux and OpenSSL API functions
- Improved documentation and examples
- Fixes for timeout handling
- Fixes for the request queue (rare loss of requests)
- Client side SNI
- Update version number


Release Notes v1.11
===
### Objectives: *Support multiple domains and certificates, support websocket ping-pong, on-the-fly compression, additional API functions*

Changes
-------

- Add API function to send file body for C and Lua
- Fix several warnings from different compilers and static code analyzers
- Drop Symbian support from the code
- Improve examples
- Timeout for CGI scripts
- Fix for requests using IPv6 addresses as hostname
- Shared data for Lua scripts and Lua server pages
- Add API function for 30x redirect
- Script for Linux bash auto-completion
- Add HTTP JSON C callback example
- Add helper function for HTTP 200 OK response
- Allow Kepler Syntax for Lua Server pages
- Update duktape to 2.2.0 and Lua to 5.3.4
- Optional support for on-the-fly compression (if zlib is available and USE_ZLIB is set)
- Add method to replace mg\_cry and log\_access by own implementation
- Fixes for IPv6 support
- Add server support for websocket ping pong protocol
- Fix misspellings in source code and documentation
- Add error msg to http_error callback
- Move unit test to a new directory
- Remove remote\_ip request\_info member (it has been legacy since several versions)
- Use gmtime_r instead of gmtime, if available
- Add some functions to C++ wrapper
- Support multiple domains with different certificate files (TLS server name identification, SNI)
- Provide client peer certificate (X509) in mg\_client\_cert structure
- Add new callback (get\_external\_ssl\_ctx) to provide pre-initialized TLS context
- Improve unit tests
- Fix ssl init for HTTPS clients
- Update version number


Release Notes v1.10
===
### Objectives: *OpenSSL 1.1 support, add server statistics and diagnostic data*

Changes
-------

- Add missing `mg_` or `MG_` to symbols in civetweb.h. Symbols without will be removed a future version.
- Add HTTPS server configuration example
- Lua Pages: mg.include should support absolute, relative and virtual path types
- Add API function for HTTP digest authentication
- Improved interface documentation
- Support parameters for Lua background scripts
- Use new connection queue implementation (previously ALTERNATIVE\_QUEUE) as default
- Add USE\_SERVER\_STATS define, so the server collects statistics data
- Convert system\_info text output and all other diagnostic strings to JSON format
- Add experimental function to query the connection status (may be dropped again)
- Add document on proposed future interface changes (for comments)
- Officially drop Symbian support
- Ignore leading blank lines in multipart messages (for Android upload service)
- Rewrite some functions, in particular request parsing
- CORS preflight directly in the server, with additional config options
- Solve some warnings from different static source code analysis tools
- Collect server status data
- Allow hostname in listening\_ports
- Make maximum request size configurable
- Allow multiple Sec-Websocket-Protocol
- Add configuration option to send additional headers
- Add configuration option for Strict-Transport-Security
- Mark "file in memory" feature is a candidate for deletion
- Improve examples
- Fix timeout error when sending larger files
- Add mg\_send\_chunk interface function
- Allow to separate server private key and certificate chain in two different files
- Support for multipart requests without quotes (for some C# clients)
- Initialize SSL in mg\_init\_library, so https client functions can be used when no server is running
- Allow "REPORT" HTTP method for REST calls to scripts
- Allow to compile civetweb.c with a C++ compiler
- Lua: Remove internal length limits of encode/decode functions
- Allow sub-resources of index script files
- Add config parameter allow\_index\_script\_resource the aforementioned feature
- Remove deprecated "uri" member of the request from the interface
- Improve documentation
- Make auth domain check optional (configuration)
- Update unit test framework to check 0.11.0 (C89/C90 compilers still need a patched version)
- Limit depth of mg.include for Lua server pages
- Additional unit tests
- OpenSSL 1.1 support
- Update version number


Release Notes v1.9.1
===
### Objectives: *Bug fix*

Changes
-------

- Add "open website" button for pre-built Windows binaries
- Fix for connections closed prematurely
- Update to a new check unit test framework and remove patches required for previous version
- Update version number


Release Notes v1.9
===
### Objectives: *Read SSI client certificate information, improve windows usability, use non-blocking sockets, bug fixes*

Changes
-------

- Add library init/exit functions (call is now optional, but will be required in V1.10)
- Windows: Show system information from the tray icon
- Windows: Bring overlaid windows to top from the tray icon
- Add Lua background script, running independent from server state
- Move obsolete examples into separated directory
- Change name of CMake generated C++ library to civetweb-cpp
- Add option to set linger timeout
- Update Duktape and Lua (third-party code)
- Add continuous integration tests
- Add API documentation
- Limit recursions in .htpasswd files
- Fix SCRIPT_NAME for CGI directory index files (index.php)
- Use non-blocking sockets
- stdint.h is now required and no longer optional
- Rewrite connection close handling
- Rewrite mg_fopen/mg_stat
- Enhanced tray icon menu for Windows
- Add subprotocol management for websocket connections
- Partially rewrite timeout handling
- Add option keep_alive_timeout_ms
- Improve support for absolute URIs
- Allow some additional compiler checks (higher warning level)
- Add option for case sensitive file names for Windows
- Short notation for listening_ports option when using IPv4 and IPv6 ports
- Make usage of Linux sendfile configurable
- Optimize build matrix for Travis CI
- Retry failing TLS/HTTPS read/write operations
- Read client certificate information
- Do not tolerate URIs with invalid characters
- Fix mg_get_cookie to ignore substrings
- Fix memory leak in form handling
- Fix bug in timer logic (for Lua Websockets)
- Updated version number

Release Notes v1.8
===
### Objectives: *CMake integration and continuous integration tests, Support client certificates, bug fixes*

Changes
-------

- Replace mg_upload by mg_handle_form_request
- CGI-scripts must receive EOF if all POST data is read
- Add API function to handle all kinds of HTML form data
- Do not allow short file names in Windows
- Callback when a new thread is initialized
- Support for short lived certificates
- Add NO_CACHING compile option
- Update Visual Studio project files to VS2015; rename directory VS2012 to VS
- Sec-Wesocket-Protocol must only return one protocol
- Mark some examples and tests as obsolete
- Remove no longer maintained test utils
- Add some default MIME types and the mg_send_mime_file API function.
- Client API using SSL certificates
- Send "Cache-Control" headers
- Add alternative to mg_upload
- Additional configuration options
- Fix memory leaks
- Add API function to check available features
- Add new interface to get listening ports
- Add websocket client interface and encode websocket data with a simple random number
- Support SSL client certificates
- Add configuration options for SSL client certificates
- Stand-alone server: Add command line option -I to display information about the system
- Redirect stderr of CGI process to error log
- Support absolute URI; split uri in mg_request_info to request_uri and local_uri
- Some source code refactoring, to improve maintainability
- Use recursive mutex for Linux
- Allow CGI environment to grow dynamically
- Support build for Lua 5.1 (including LuaJIT), Lua 5.2 and Lua 5.3
- Improve examples and documentation
- Build option CIVETWEB_SERVE_NO_FILES to disable serving static files
- Add Server side JavaScript support (Duktape library)
- Created a "civetweb" organization at GitHub.
- Repository moved from https://github.com/bel2125/civetweb to https://github.com/civetweb/civetweb
- Improved continuous integration
- CMake support, continuous integration with Travis CI and Appveyor
- Adapt/port unit tests to CMake/Travis/Appveyor
- Bug fixes, including issues from static code analysis
- Add status badges to the GitHub project main page
- Updated version number

Release Notes v1.7
===
### Objectives: *Examples, documentation, additional API functions, some functions rewritten, bug fixes and updates*

Changes
-------

- Format source with clang_format
- Use function 'sendfile' for Linux
- Fix for CRAMFS in Linux
- Fix for file modification times in Windows
- Use SO_EXCLUSIVEADDRUSE instead of SO_REUSEADDR for Windows
- Rewrite push/pull functions
- Allow to use Lua as shared objects (WITH_LUA_SHARED)
- Fixes for many warnings
- URI specific callbacks and different timeouts for websockets
- Add chunked transfer support
- Update LuaFileSystem
- Update Lua to 5.2.4
- Fix build for MinGW-x64, TDM-GCC and clang
- Update SQLite to 3.8.10.2
- Fix CGI variables SCRIPT_NAME and PATH_TRANSLATED
- Set TCP_USER_TIMEOUT to deal faster with broken connections
- Add a Lua form handling example
- Return more differentiated HTTP error codes
- Add log_access callback
- Rewrite and comment request handling function
- Specify in detail and document return values of callback functions
- Set names for all threads (unless NO_THREAD_NAME is defined)
- New API functions for TCP/HTTP clients
- Fix upload of huge files
- Allow multiple SSL instances within one application
- Improve API and user documentation
- Allow to choose between static and dynamic Lua library
- Improve unit test
- Use temporary file name for partially uploaded files
- Additional API functions exported to C++
- Add a websocket client example
- Add a websocket client API
- Update websocket example
- Make content length available in request_info
- New API functions: access context, callback for create/delete, access user data
- Upgraded Lua from 5.2.2 to 5.2.3 and finally 5.2.4
- Integrate LuaXML (for testing purposes)
- Fix compiler warnings
- Updated version number

Release Notes v1.6
===
### Objectives: *Enhance Lua support, configuration dialog for windows, new examples, bug fixes and updates*

Changes
-------

- Add examples of Lua pages, scripts and websockets to the test directory (bel)
- Add dialog to change htpasswd files for the Windows standalone server (bel)
- Fix compiler warnings and warnings from static code analysis (Danny Al-Gaaf, jmc-, Thomas, bel, ...)
- Add new unit tests (bel)
- Support includes in htpasswd files (bel)
- Add a basic option check for the standalone executable (bel)
- Support user defined error pages (bel)
- Method to get POST request parameters via C++ interface (bel)
- Re-Add unit tests for Linux and Windows (jmc-, bel)
- Allow to specify title and tray icon for the Windows standalone server (bel)
- Fix minor memory leaks (bel)
- Redirect all memory allocation/deallocation through mg functions which may be overwritten (bel)
- Support Cross-Origin Resource Sharing (CORS) for static files and scripts (bel)
- Win32: Replace dll.def file by export macros in civetweb.h (CSTAJ)
- Base64 encode and decode functions for Lua (bel)
- Support pre-loaded files for the Lua environment (bel)
- Server should check the nonce for http digest access authentication (bel)
- Hide read-only flag in file dialogs opened by the Edit Settings dialog for the Windows executable (bel)
- Add all functions to dll.def, that are in the header (bel)
- Added Lua extensions: send_file, get_var, get_mime_type, get_cookie, url_decode, url_encode (bel)
- mg_set_request_handler() mod to use pattern (bel, Patch from Toni Wilk)
- Solved, tested and documented SSL support for Windows (bel)
- Fixed: select for Linux needs the nfds parameter set correctly (bel)
- Add methods for returning the ports civetweb is listening on (keithel)
- Fixes for Lua Server Pages, as described within the google groups thread. (bel)
- Added support for plain Lua Scripts, and an example script. (bel)
- A completely new, and more illustrative websocket example for C. (bel)
- Websocket for Lua (bel)
- An optional websocket_root directory, including URL rewriting (bel)
- Update of SQLite3 to 3.8.1. (bel)
- Add "date" header field to replies, according to the requirements of RFC 2616 (the HTTP standard), Section 14.18 (bel)
- Fix websocket long pull (celeron55)
- Updated API documentation (Alex Kozlov)
- Fixed Posix locking functions for Windows (bel2125)
- Updated version number

Release Notes v1.5
===
### Objectives: *Bug fixes and updates, repository restoration*

Changes
-------

- Corrected bad mask flag/opcode passing to websocket callback (William Greathouse)
- Moved CEVITWEB_VERSION define into civetweb.h
- Added new simple zip deployment build for Windows.
- Removed windows install package build.
- Fixes page violation in mod_lua.inl (apkbox)
- Use C style comments to enable compiling most of civetweb with -ansi. (F-Secure Corporation)
- Allow directories with non ASCII characters in Windows in UTF-8 encoded (bel2125)
- Added Lua File System support (bel2125)
- Added mongoose history back in repository thanks to (Paul Sokolovsky)
- Fixed keep alive (bel2125)
- Updated of MIME types (bel2125)
- Updated lsqlite (bel2125)
- Fixed master thread priority (bel2125)
- Fixed IPV6 defines under Windowe (grenclave)
- Fixed potential dead lock in connection_close() (Morgan McGuire)
- Added WebSocket example using asynchronous server messages (William Greathouse)
- Fixed the getcwd() warning (William Greathouse)
- Implemented the connection_close() callback (William Greathouse)
- Fixed support URL's in civetweb.c (Daniel Oaks)
- Allow port number to be zero to use a random free port (F-Secure Corporation)
- Wait for threads to finish when stopping for a clean shutdown (F-Secure Corporation)
- More static analysis fixes against Coverity tool (F-Secure Corporation)
- Travis automated build testing support added (Daniel Oaks)
- Updated version numbers.
- Added contributor credits file.

Release Notes v1.4
===
### Objectives: *New URI handler interface, feature enhancements, C++ extensions*
The main idea behind this release is to bring about API consistency. All changes
are backward compatible and have been kept to a minimum.

Changes
-------

- Added mg_set_request_handler() which provides a URI mapping for callbacks.
   This is a new alternative to overriding callbacks.begin_request.
- Externalized mg_url_encode()
- Externalized mg_strncasecmp() for utiliy
- Added CivetServer::getParam methods
- Added CivetServer::urlDecode methods
- Added CivetServer::urlEncode methods
- Dealt with compiler warnings and some static analysis hits.
- Added mg_get_var2() to parse repeated query variables
- Externalized logging function cry() as mg_cry()
- Added CivetServer::getCookie method (Hariprasad Kamath)
- Added CivetServer::getHeader method (Hariprasad Kamath)
- Added new basic C embedding example
- Conformed source files to UNIX line endings for consistency.
- Unified the coding style to improve reability.

Release Notes v1.3
===
### Objectives: *Buildroot Integration*

Changes
-------

- Made option to put initial HTMLDIR in a different place
- Validated build without SQLITE3 large file support
- Updated documentation
- Updated Buildroot config example

Release Notes v1.2
===
### Objectives: *Installation Improvements, buildroot, cross compile support*
The objective of this release is to make installation seamless.

Changes
-------

- Create an installation guide
- Created both 32 and 64 bit windows installations
- Added install for windows distribution
- Added 64 bit build profiles for VS 2012.
- Created a buildroot patch
- Updated makefile to better support buildroot
- Made doc root and ports configurable during the make install.
- Updated Linux Install
- Updated OS X Package
- Improved install scheme with welcome web page

Known Issues
-----

- The prebuilt Window's version requires [Visual C++ Redistributable for Visual Studio 2012](http://www.microsoft.com/en-us/download/details.aspx?id=30679)

Release Notes v1.1
===
### Objectives: *Build, Documentation, License Improvements*
The objective of this release is to establish a maintable code base, ensure MIT license rights and improve usability and documentation.

Changes
-------

- Reorangized build directories to make them more intuitive
- Added new build rules for lib and slib with option to include C++ class
- Upgraded Lua from 5.2.1 to 5.2.2
- Added fallback configuration file path for Linux systems.
    + Good for having a system wide default configuration /usr/local/etc/civetweb.conf
- Added new C++ abstraction class CivetServer
- Added thread safety for and fixed websocket defects (Morgan McGuire)
- Created PKGBUILD to use Arch distribution (Daniel Oaks)
- Created new documentation on Embeddeding, Building and yaSSL (see docs/).
- Updated License file to include all licenses.
- Replaced MD5 implementation due to questionable license.
     + This requires new source file md5.inl
- Changed UNIX/OSX build to conform to common practices.
     + Supports build, install and clean rules.
     + Supports cross compiling
     + Features can be chosen in make options
- Moved Cocoa/OSX build and packaging to a separate file.
     + This actually a second build variant for OSX.
     + Removed yaSSL from the OSX build, not needed.
- Added new Visual Studio projects for Windows builds.
     + Removed Windows support from Makefiles
     + Provided additional, examples with Lua, and another with yaSSL.
- Changed Zombie Reaping policy to not ignore SIGCHLD.
     + The previous method caused trouble in applciations that spawn children.

Known Issues
-----

- Build support for VS6 and some other has been deprecated.
    + This does not impact embedded programs, just the stand-alone build.
    + The old Makefile was renamed to Makefile.deprecated.
    + This is partcially do to lack fo testing.
    + Need to find out what is actually in demand.
- Build changes may impact current users.
    + As with any change of this type, changes may impact some users.

Release Notes v1.0
===

### Objectives: *MIT License Preservation, Rebranding*
The objective of this release is to establish a version of the Mongoose software distribution that still retains the MIT license.

Changes
-------

- Renamed Mongoose to Civetweb in the code and documentation.
- Replaced copyrighted images with new images
- Created a new code repository at https://github.com/civetweb/civetweb
- Created a distribution site at https://sourceforge.net/projects/civetweb/
- Basic build testing
