#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(WIN32) || defined(WINDOWS)
#include <fcntl.h>
#include <io.h>
#endif

int
main(int argc, char *argv[])
{
	char buf[1024];
	size_t rec_len = 0;
	const char *response_header = "Content-Type: text/plain\r\n"
	                              "Connection: close\r\n"
	                              "\r\n";
	const char *req_method = getenv("REQUEST_METHOD");
	const char *con_length = getenv("CONTENT_LENGTH");

#if defined(_WIN32) || defined(WIN32) || defined(WINDOWS)
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
#endif

	/* Write the response header with \r\n */
	fwrite(response_header, 1, strlen(response_header), stdout);

	/* Headline for generated reply: */
	printf("Got message:\n  Method: %s\n  Content-Length: %s\n  Content: ",
	       req_method,
	       con_length ? con_length : "not set");

	/* Read all data from stdin and send it to stdout */
	while ((rec_len = fread(buf, 1, sizeof(buf) - 1, stdin)) > 0) {
		fwrite(buf, 1, rec_len, stdout);
	}

	return 0;
}
