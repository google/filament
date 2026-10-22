HTTPS Server configuration example
====

This directory contains an example [`civetweb.conf`](civetweb.conf) configuration file for a secure HTTPS server.  You can run a HTTPS server without most of the options there - only `ssl_certificate` and one port (e.g., `443s`) in `listening_ports` is required.  The default settings will work, but not comply with up to date security standards.  It is somewhat debatable what "up to date security" means - you can use the following web sites to run tests:

- https://securityheaders.io
- https://www.htbridge.com/ssl
- https://www.htbridge.com/websec
- https://www.ssllabs.com/ssltest/analyze.html / https://www.qualys.com/forms/freescan/
- probably there are some more ... let me know!

Instructions to run the test and to adapt the configuration can be found [`civetweb.conf`](civetweb.conf).  You can test this configuration directly with the standalone server, or you can take the settings and add it into your embedding code.

Note: I do not take any warranty or liability for this configuration, or for the content of any linked web site.

