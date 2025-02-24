This simple utility receives messages via a WebSocket and writes them out to both the command line
and a file called `wslog-TIMESTAMP.txt` in the working directory.

It can be used to receive logs from CTS in a way that's resistant to test crashes and totally
independent of which runtime is being used (e.g. standalone, WPT, Node).
It's used in particular to capture timing results for predefining "chunking" of the CTS for WPT.

To set up, use `npm ci`.
To launch, use `npm start`.
