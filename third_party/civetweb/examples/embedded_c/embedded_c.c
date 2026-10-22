/*
 * Copyright (c) 2013-2017 the CivetWeb developers
 * Copyright (c) 2013 No Face Press, LLC
 * License http://opensource.org/licenses/mit-license.php MIT License
 */

#ifdef NO_SSL
#define TEST_WITHOUT_SSL
#endif

/* Simple example program on how to use CivetWeb embedded into a C program. */
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "civetweb.h"


#define DOCUMENT_ROOT "."
#ifdef TEST_WITHOUT_SSL

#ifdef USE_IPV6
#define PORT "[::]:8888,8884"
#else
#define PORT "8888,8884"
#endif

#else

#ifdef USE_IPV6
#define PORT "[::]:8888r,[::]:8843s,8884"
#else
#define PORT "8888r,8843s,8884"
#endif

#endif


#define EXAMPLE_URI "/example"
#define EXIT_URI "/exit"
int exitNow = 0;


int
ExampleHandler(struct mg_connection *conn, void *cbdata)
{
	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	mg_printf(conn, "<html><body>");
	mg_printf(conn, "<h2>This is an example text from a C handler</h2>");
	mg_printf(
	    conn,
	    "<p>To see a page from the A handler <a href=\"A\">click A</a></p>");
	mg_printf(conn,
	          "<p>To see a page from the A handler <a href=\"A/A\">click "
	          "A/A</a></p>");
	mg_printf(conn,
	          "<p>To see a page from the A/B handler <a "
	          "href=\"A/B\">click A/B</a></p>");
	mg_printf(conn,
	          "<p>To see a page from the B handler (0) <a "
	          "href=\"B\">click B</a></p>");
	mg_printf(conn,
	          "<p>To see a page from the B handler (1) <a "
	          "href=\"B/A\">click B/A</a></p>");
	mg_printf(conn,
	          "<p>To see a page from the B handler (2) <a "
	          "href=\"B/B\">click B/B</a></p>");
	mg_printf(conn,
	          "<p>To see a page from the *.foo handler <a "
	          "href=\"xy.foo\">click xy.foo</a></p>");
	mg_printf(conn,
	          "<p>To see a page from the close handler <a "
	          "href=\"close\">click close</a></p>");
	mg_printf(conn,
	          "<p>To see a page from the FileHandler handler <a "
	          "href=\"form\">click form</a> (the starting point of the "
	          "<b>form</b> test)</p>");
	mg_printf(conn,
	          "<p>To see a page from the CookieHandler handler <a "
	          "href=\"cookie\">click cookie</a></p>");
	mg_printf(conn,
	          "<p>To see a page from the PostResponser handler <a "
	          "href=\"postresponse\">click post response</a></p>");
	mg_printf(conn,
	          "<p>To see an example for parsing files on the fly <a "
	          "href=\"on_the_fly_form\">click form</a> (form for "
	          "uploading files)</p>");

#ifdef USE_WEBSOCKET
	mg_printf(conn,
	          "<p>To test the websocket handler <a href=\"/websocket\">click "
	          "websocket</a></p>");
#endif

	mg_printf(conn,
	          "<p>To test the authentication handler <a href=\"/auth\">click "
	          "auth</a></p>");

	mg_printf(conn, "<p>To exit <a href=\"%s\">click exit</a></p>", EXIT_URI);
	mg_printf(conn, "</body></html>\n");
	return 1;
}


int
ExitHandler(struct mg_connection *conn, void *cbdata)
{
	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: "
	          "text/plain\r\nConnection: close\r\n\r\n");
	mg_printf(conn, "Server will shut down.\n");
	mg_printf(conn, "Bye!\n");
	exitNow = 1;
	return 1;
}


int
AHandler(struct mg_connection *conn, void *cbdata)
{
	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	mg_printf(conn, "<html><body>");
	mg_printf(conn, "<h2>This is the A handler!!!</h2>");
	mg_printf(conn, "</body></html>\n");
	return 1;
}


int
ABHandler(struct mg_connection *conn, void *cbdata)
{
	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	mg_printf(conn, "<html><body>");
	mg_printf(conn, "<h2>This is the AB handler!!!</h2>");
	mg_printf(conn, "</body></html>\n");
	return 1;
}


int
BXHandler(struct mg_connection *conn, void *cbdata)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);

	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	mg_printf(conn, "<html><body>");
	mg_printf(conn,
	          "<h2>This is the BX handler with argument %s.</h2>",
	          cbdata);
	mg_printf(conn, "<p>The actual uri is %s</p>", req_info->local_uri);
	mg_printf(conn, "</body></html>\n");
	return 1;
}


int
FooHandler(struct mg_connection *conn, void *cbdata)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);

	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	mg_printf(conn, "<html><body>");
	mg_printf(conn, "<h2>This is the Foo handler!!!</h2>");
	mg_printf(conn,
	          "<p>The request was:<br><pre>%s %s HTTP/%s</pre></p>",
	          req_info->request_method,
	          req_info->local_uri,
	          req_info->http_version);
	mg_printf(conn, "</body></html>\n");
	return 1;
}


int
CloseHandler(struct mg_connection *conn, void *cbdata)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);

	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	mg_printf(conn, "<html><body>");
	mg_printf(conn,
	          "<h2>This handler will close the connection in a second</h2>");
#ifdef _WIN32
	Sleep(1000);
#else
	sleep(1);
#endif
	mg_printf(conn, "bye");
	printf("CloseHandler: close connection\n");
	mg_close_connection(conn);
	printf("CloseHandler: wait 10 sec\n");
#ifdef _WIN32
	Sleep(10000);
#else
	sleep(10);
#endif
	printf("CloseHandler: return from function\n");
	return 1;
}


#if !defined(NO_FILESYSTEMS)
int
FileHandler(struct mg_connection *conn, void *cbdata)
{
	/* In this handler, we ignore the req_info and send the file "fileName". */
	const char *fileName = (const char *)cbdata;

	mg_send_file(conn, fileName);
	return 1;
}
#endif /* NO_FILESYSTEMS */


#define MD5_STATIC static
#include "../src/md5.inl"

/* Stringify binary data. Output buffer must be twice as big as input,
 * because each byte takes 2 bytes in string representation */
static void
bin2str(char *to, const unsigned char *p, size_t len)
{
	static const char *hex = "0123456789abcdef";

	for (; len--; p++) {
		*to++ = hex[p[0] >> 4];
		*to++ = hex[p[0] & 0x0f];
	}
	*to = '\0';
}


int
field_found(const char *key,
            const char *filename,
            char *path,
            size_t pathlen,
            void *user_data)
{
	struct mg_connection *conn = (struct mg_connection *)user_data;

	mg_printf(conn, "\r\n\r\n%s:\r\n", key);

	if (filename && *filename) {
#ifdef _WIN32
		_snprintf(path, pathlen, "D:\\tmp\\%s", filename);
#else
		snprintf(path, pathlen, "/tmp/%s", filename);
#endif
		return MG_FORM_FIELD_STORAGE_GET;
	}
	return MG_FORM_FIELD_STORAGE_GET;
}


int
field_get(const char *key, const char *value, size_t valuelen, void *user_data)
{
	struct mg_connection *conn = (struct mg_connection *)user_data;

	if ((key != NULL) && (key[0] == '\0')) {
		/* Incorrect form data detected */
		return MG_FORM_FIELD_HANDLE_ABORT;
	}
	if ((valuelen > 0) && (value == NULL)) {
		/* Unreachable, since this call will not be generated by civetweb. */
		return MG_FORM_FIELD_HANDLE_ABORT;
	}

	if (key) {
		mg_printf(conn, "key = %s\n", key);
	}
	mg_printf(conn, "valuelen = %u\n", valuelen);

	if (valuelen > 0) {
		/* mg_write(conn, value, valuelen); */

		md5_byte_t hash[16];
		md5_state_t ctx;
		char outputbuf[33];

		md5_init(&ctx);
		md5_append(&ctx, (const md5_byte_t *)value, valuelen);
		md5_finish(&ctx, hash);
		bin2str(outputbuf, hash, sizeof(hash));
		mg_printf(conn, "value md5 hash = %s\n", outputbuf);
	}

#if 0 /* for debugging */
	if (!strcmp(key, "File")) {
		FILE *f = fopen("test.txt", "wb");
		if (f) {
			fwrite(value, 1, valuelen, f);
			fclose(f);
		}
	}
#endif

	return 0;
}


int
field_stored(const char *path, long long file_size, void *user_data)
{
	struct mg_connection *conn = (struct mg_connection *)user_data;

	mg_printf(conn,
	          "stored as %s (%lu bytes)\r\n\r\n",
	          path,
	          (unsigned long)file_size);

	return 0;
}


int
FormHandler(struct mg_connection *conn, void *cbdata)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);
	int ret;
	struct mg_form_data_handler fdh = {field_found, field_get, field_stored, 0};

	/* It would be possible to check the request info here before calling
	 * mg_handle_form_request. */
	(void)req_info;

	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: "
	          "text/plain\r\nConnection: close\r\n\r\n");
	fdh.user_data = (void *)conn;

	/* Call the form handler */
	mg_printf(conn, "Form data:");
	ret = mg_handle_form_request(conn, &fdh);
	mg_printf(conn, "\r\n%i fields found", ret);

	return 1;
}


int
FileUploadForm(struct mg_connection *conn, void *cbdata)
{
	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");

	mg_printf(conn, "<!DOCTYPE html>\n");
	mg_printf(conn, "<html>\n<head>\n");
	mg_printf(conn, "<meta charset=\"UTF-8\">\n");
	mg_printf(conn, "<title>File upload</title>\n");
	mg_printf(conn, "</head>\n<body>\n");
	mg_printf(conn,
	          "<form action=\"%s\" method=\"POST\" "
	          "enctype=\"multipart/form-data\">\n",
	          (const char *)cbdata);
	mg_printf(conn, "<input type=\"file\" name=\"filesin\" multiple>\n");
	mg_printf(conn, "<input type=\"submit\" value=\"Submit\">\n");
	mg_printf(conn, "</form>\n</body>\n</html>\n");
	return 1;
}


struct tfile_checksum {
	char name[128];
	unsigned long long length;
	md5_state_t chksum;
};

#define MAX_FILES (10)

struct tfiles_checksums {
	int index;
	struct tfile_checksum file[MAX_FILES];
};


int
field_disp_read_on_the_fly(const char *key,
                           const char *filename,
                           char *path,
                           size_t pathlen,
                           void *user_data)
{
	struct tfiles_checksums *context = (struct tfiles_checksums *)user_data;

	(void)key;
	(void)path;
	(void)pathlen;

	if (context->index < MAX_FILES) {
		context->index++;
		strncpy(context->file[context->index - 1].name, filename, 128);
		context->file[context->index - 1].name[127] = 0;
		context->file[context->index - 1].length = 0;
		md5_init(&(context->file[context->index - 1].chksum));
		return MG_FORM_FIELD_STORAGE_GET;
	}
	return MG_FORM_FIELD_STORAGE_ABORT;
}


int
field_get_checksum(const char *key,
                   const char *value,
                   size_t valuelen,
                   void *user_data)
{
	struct tfiles_checksums *context = (struct tfiles_checksums *)user_data;
	(void)key;

	context->file[context->index - 1].length += valuelen;
	md5_append(&(context->file[context->index - 1].chksum),
	           (const md5_byte_t *)value,
	           valuelen);

	return 0;
}


int
CheckSumHandler(struct mg_connection *conn, void *cbdata)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);
	int i, j, ret;
	struct tfiles_checksums chksums;
	md5_byte_t digest[16];
	struct mg_form_data_handler fdh = {field_disp_read_on_the_fly,
	                                   field_get_checksum,
	                                   0,
	                                   (void *)&chksums};

	/* It would be possible to check the request info here before calling
	 * mg_handle_form_request. */
	(void)req_info;

	memset(&chksums, 0, sizeof(chksums));

	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\n"
	          "Content-Type: text/plain\r\n"
	          "Connection: close\r\n\r\n");

	/* Call the form handler */
	mg_printf(conn, "File checksums:");
	ret = mg_handle_form_request(conn, &fdh);
	for (i = 0; i < chksums.index; i++) {
		md5_finish(&(chksums.file[i].chksum), digest);
		/* Visual Studio 2010+ support llu */
		mg_printf(conn,
		          "\r\n%s %llu ",
		          chksums.file[i].name,
		          chksums.file[i].length);
		for (j = 0; j < 16; j++) {
			mg_printf(conn, "%02x", (unsigned int)digest[j]);
		}
	}
	mg_printf(conn, "\r\n%i files\r\n", ret);

	return 1;
}


int
CookieHandler(struct mg_connection *conn, void *cbdata)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);
	const char *cookie = mg_get_header(conn, "Cookie");
	char first_str[64], count_str[64];
	int count;

	(void)mg_get_cookie(cookie, "first", first_str, sizeof(first_str));
	(void)mg_get_cookie(cookie, "count", count_str, sizeof(count_str));

	mg_printf(conn, "HTTP/1.1 200 OK\r\nConnection: close\r\n");
	if (first_str[0] == 0) {
		time_t t = time(0);
		struct tm *ptm = localtime(&t);
		mg_printf(conn,
		          "Set-Cookie: first=%04i-%02i-%02iT%02i:%02i:%02i\r\n",
		          ptm->tm_year + 1900,
		          ptm->tm_mon + 1,
		          ptm->tm_mday,
		          ptm->tm_hour,
		          ptm->tm_min,
		          ptm->tm_sec);
	}
	count = (count_str[0] == 0) ? 0 : atoi(count_str);
	mg_printf(conn, "Set-Cookie: count=%i\r\n", count + 1);
	mg_printf(conn, "Content-Type: text/html\r\n\r\n");

	mg_printf(conn, "<html><body>");
	mg_printf(conn, "<h2>This is the CookieHandler.</h2>");
	mg_printf(conn, "<p>The actual uri is %s</p>", req_info->local_uri);

	if (first_str[0] == 0) {
		mg_printf(conn, "<p>This is the first time, you opened this page</p>");
	} else {
		mg_printf(conn, "<p>You opened this page %i times before.</p>", count);
		mg_printf(conn, "<p>You first opened this page on %s.</p>", first_str);
	}

	mg_printf(conn, "</body></html>\n");
	return 1;
}


int
PostResponser(struct mg_connection *conn, void *cbdata)
{
	long long r_total = 0;
	int r, s;

	char buf[2048];

	const struct mg_request_info *ri = mg_get_request_info(conn);

	if (0 != strcmp(ri->request_method, "POST")) {
		/* Not a POST request */
		char buf[1024];
		int ret = mg_get_request_link(conn, buf, sizeof(buf));

		mg_printf(conn,
		          "HTTP/1.1 405 Method Not Allowed\r\nConnection: close\r\n");
		mg_printf(conn, "Content-Type: text/plain\r\n\r\n");
		mg_printf(conn,
		          "%s method not allowed in the POST handler\n",
		          ri->request_method);
		if (ret >= 0) {
			mg_printf(conn,
			          "use a web tool to send a POST request to %s\n",
			          buf);
		}
		return 1;
	}

	if (ri->content_length >= 0) {
		/* We know the content length in advance */
	} else {
		/* We must read until we find the end (chunked encoding
		 * or connection close), indicated my mg_read returning 0 */
	}

	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\nConnection: "
	          "close\r\nTransfer-Encoding: chunked\r\n");
	mg_printf(conn, "Content-Type: text/plain\r\n\r\n");

	r = mg_read(conn, buf, sizeof(buf));
	while (r > 0) {
		r_total += r;
		s = mg_send_chunk(conn, buf, r);
		if (r != s) {
			/* Send error */
			break;
		}
		r = mg_read(conn, buf, sizeof(buf));
	}
	mg_printf(conn, "0\r\n");

	return 1;
}


#if !defined(NO_FILESYSTEMS)
int
AuthStartHandler(struct mg_connection *conn, void *cbdata)
{
	static unsigned long long firstload = 0;
	const char *passfile = "password_example_file.txt";
	const char *realm = "password_example";
	const char *user = "user";
	char passwd[64];

	if (firstload == 0) {

		/* Set a random password (4 digit number - bad idea from a security
		 * point of view, but this is an API demo, not a security tutorial),
		 * and store it in some directory within the document root (extremely
		 * bad idea, but this is still not a security tutorial).
		 * The reason we create a new password every time the server starts
		 * is just for demonstration - we don't want the browser to store the
		 * password, so when we repeat the test we start with a new password.
		 */
		firstload = (unsigned long long)time(NULL);
		sprintf(passwd, "%04u", (unsigned int)(firstload % 10000));
		mg_modify_passwords_file(passfile, realm, user, passwd);

		/* Just tell the user the new password generated for this test. */
		mg_printf(conn,
		          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
		          "close\r\n\r\n");

		mg_printf(conn, "<!DOCTYPE html>\n");
		mg_printf(conn, "<html>\n<head>\n");
		mg_printf(conn, "<meta charset=\"UTF-8\">\n");
		mg_printf(conn, "<title>Auth handlerexample</title>\n");
		mg_printf(conn, "</head>\n");

		mg_printf(conn, "<body>\n");
		mg_printf(conn,
		          "<p>The first time you visit this page, it's free!</p>\n");
		mg_printf(conn,
		          "<p>Next time, use username \"%s\" and password \"%s\"</p>\n",
		          user,
		          passwd);
		mg_printf(conn, "</body>\n</html>\n");

		return 1;
	}

	if (mg_check_digest_access_authentication(conn, realm, passfile) <= 0) {
		/* No valid authorization */
		mg_send_digest_access_authentication_request(conn, realm);
		return 1;
	}

	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");

	mg_printf(conn, "<!DOCTYPE html>\n");
	mg_printf(conn, "<html>\n<head>\n");
	mg_printf(conn, "<meta charset=\"UTF-8\">\n");
	mg_printf(conn, "<title>Auth handlerexample</title>\n");
	mg_printf(conn, "</head>\n");

	mg_printf(conn, "<body>\n");
	mg_printf(conn, "<p>This is the password protected contents</p>\n");
	mg_printf(conn, "</body>\n</html>\n");

	return 1;
}
#endif /* NO_FILESYSTEMS */


int
WebSocketStartHandler(struct mg_connection *conn, void *cbdata)
{
	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");

	mg_printf(conn, "<!DOCTYPE html>\n");
	mg_printf(conn, "<html>\n<head>\n");
	mg_printf(conn, "<meta charset=\"UTF-8\">\n");
	mg_printf(conn, "<title>Embedded websocket example</title>\n");

#ifdef USE_WEBSOCKET
	/* mg_printf(conn, "<script type=\"text/javascript\"><![CDATA[\n"); ...
	 * xhtml style */
	mg_printf(conn, "<script>\n");
	mg_printf(
	    conn,
	    "function load() {\n"
	    "  var wsproto = (location.protocol === 'https:') ? 'wss:' : 'ws:';\n"
	    "  connection = new WebSocket(wsproto + '//' + window.location.host + "
	    "'/websocket');\n"
	    "  websock_text_field = "
	    "document.getElementById('websock_text_field');\n"
	    "  connection.onmessage = function (e) {\n"
	    "    websock_text_field.innerHTML=e.data;\n"
	    "  }\n"
	    "  connection.onerror = function (error) {\n"
	    "    alert('WebSocket error');\n"
	    "    connection.close();\n"
	    "  }\n"
	    "}\n");
	/* mg_printf(conn, "]]></script>\n"); ... xhtml style */
	mg_printf(conn, "</script>\n");
	mg_printf(conn, "</head>\n<body onload=\"load()\">\n");
	mg_printf(
	    conn,
	    "<div id='websock_text_field'>No websocket connection yet</div>\n");
#else
	mg_printf(conn, "</head>\n<body>\n");
	mg_printf(conn, "Example not compiled with USE_WEBSOCKET\n");
#endif
	mg_printf(conn, "</body>\n</html>\n");

	return 1;
}


#ifdef USE_WEBSOCKET

/* MAX_WS_CLIENTS defines how many clients can connect to a websocket at the
 * same time. The value 5 is very small and used here only for demonstration;
 * it can be easily tested to connect more than MAX_WS_CLIENTS clients.
 * A real server should use a much higher number, or better use a dynamic list
 * of currently connected websocket clients. */
#define MAX_WS_CLIENTS (5)

struct t_ws_client {
	struct mg_connection *conn;
	int state;
} static ws_clients[MAX_WS_CLIENTS];


#define ASSERT(x)                                                              \
	{                                                                          \
		if (!(x)) {                                                            \
			fprintf(stderr,                                                    \
			        "Assertion failed in line %u\n",                           \
			        (unsigned)__LINE__);                                       \
		}                                                                      \
	}


int
WebSocketConnectHandler(const struct mg_connection *conn, void *cbdata)
{
	struct mg_context *ctx = mg_get_context(conn);
	int reject = 1;
	int i;

	mg_lock_context(ctx);
	for (i = 0; i < MAX_WS_CLIENTS; i++) {
		if (ws_clients[i].conn == NULL) {
			ws_clients[i].conn = (struct mg_connection *)conn;
			ws_clients[i].state = 1;
			mg_set_user_connection_data(ws_clients[i].conn,
			                            (void *)(ws_clients + i));
			reject = 0;
			break;
		}
	}
	mg_unlock_context(ctx);

	fprintf(stdout,
	        "Websocket client %s\r\n\r\n",
	        (reject ? "rejected" : "accepted"));
	return reject;
}


void
WebSocketReadyHandler(struct mg_connection *conn, void *cbdata)
{
	const char *text = "Hello from the websocket ready handler";
	struct t_ws_client *client = mg_get_user_connection_data(conn);

	mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, text, strlen(text));
	fprintf(stdout, "Greeting message sent to websocket client\r\n\r\n");
	ASSERT(client->conn == conn);
	ASSERT(client->state == 1);

	client->state = 2;
}


int
WebsocketDataHandler(struct mg_connection *conn,
                     int bits,
                     char *data,
                     size_t len,
                     void *cbdata)
{
	struct t_ws_client *client = mg_get_user_connection_data(conn);
	ASSERT(client->conn == conn);
	ASSERT(client->state >= 1);

	fprintf(stdout, "Websocket got %lu bytes of ", (unsigned long)len);
	switch (((unsigned char)bits) & 0x0F) {
	case MG_WEBSOCKET_OPCODE_CONTINUATION:
		fprintf(stdout, "continuation");
		break;
	case MG_WEBSOCKET_OPCODE_TEXT:
		fprintf(stdout, "text");
		break;
	case MG_WEBSOCKET_OPCODE_BINARY:
		fprintf(stdout, "binary");
		break;
	case MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE:
		fprintf(stdout, "close");
		break;
	case MG_WEBSOCKET_OPCODE_PING:
		fprintf(stdout, "ping");
		break;
	case MG_WEBSOCKET_OPCODE_PONG:
		fprintf(stdout, "pong");
		break;
	default:
		fprintf(stdout, "unknown(%1xh)", ((unsigned char)bits) & 0x0F);
		break;
	}
	fprintf(stdout, " data:\r\n");
	fwrite(data, len, 1, stdout);
	fprintf(stdout, "\r\n\r\n");

	return 1;
}


void
WebSocketCloseHandler(const struct mg_connection *conn, void *cbdata)
{
	struct mg_context *ctx = mg_get_context(conn);
	struct t_ws_client *client = mg_get_user_connection_data(conn);
	ASSERT(client->conn == conn);
	ASSERT(client->state >= 1);

	mg_lock_context(ctx);
	client->state = 0;
	client->conn = NULL;
	mg_unlock_context(ctx);

	fprintf(stdout,
	        "Client dropped from the set of webserver connections\r\n\r\n");
}


void
InformWebsockets(struct mg_context *ctx)
{
	static unsigned long cnt = 0;
	char text[32];
	size_t textlen;
	int i;

	sprintf(text, "%lu", ++cnt);
	textlen = strlen(text);

	mg_lock_context(ctx);
	for (i = 0; i < MAX_WS_CLIENTS; i++) {
		if (ws_clients[i].state == 2) {
			mg_websocket_write(ws_clients[i].conn,
			                   MG_WEBSOCKET_OPCODE_TEXT,
			                   text,
			                   textlen);
		}
	}
	mg_unlock_context(ctx);
}
#endif


#ifdef USE_SSL_DH
#include "openssl/dh.h"
#include "openssl/ec.h"
#include "openssl/ecdsa.h"
#include "openssl/evp.h"
#include "openssl/ssl.h"

DH *
get_dh2236()
{
	static unsigned char dh2236_p[] = {
	    0x0E, 0x97, 0x6E, 0x6A, 0x88, 0x84, 0xD2, 0xD7, 0x55, 0x6A, 0x17, 0xB7,
	    0x81, 0x9A, 0x98, 0xBC, 0x7E, 0xD1, 0x6A, 0x44, 0xB1, 0x18, 0xE6, 0x25,
	    0x3A, 0x62, 0x35, 0xF0, 0x41, 0x91, 0xE2, 0x16, 0x43, 0x9D, 0x8F, 0x7D,
	    0x5D, 0xDA, 0x85, 0x47, 0x25, 0xC4, 0xBA, 0x68, 0x0A, 0x87, 0xDC, 0x2C,
	    0x33, 0xF9, 0x75, 0x65, 0x17, 0xCB, 0x8B, 0x80, 0xFE, 0xE0, 0xA8, 0xAF,
	    0xC7, 0x9E, 0x82, 0xBE, 0x6F, 0x1F, 0x00, 0x04, 0xBD, 0x69, 0x50, 0x8D,
	    0x9C, 0x3C, 0x41, 0x69, 0x21, 0x4E, 0x86, 0xC8, 0x2B, 0xCC, 0x07, 0x4D,
	    0xCF, 0xE4, 0xA2, 0x90, 0x8F, 0x66, 0xA9, 0xEF, 0xF7, 0xFC, 0x6F, 0x5F,
	    0x06, 0x22, 0x00, 0xCB, 0xCB, 0xC3, 0x98, 0x3F, 0x06, 0xB9, 0xEC, 0x48,
	    0x3B, 0x70, 0x6E, 0x94, 0xE9, 0x16, 0xE1, 0xB7, 0x63, 0x2E, 0xAB, 0xB2,
	    0xF3, 0x84, 0xB5, 0x3D, 0xD7, 0x74, 0xF1, 0x6A, 0xD1, 0xEF, 0xE8, 0x04,
	    0x18, 0x76, 0xD2, 0xD6, 0xB0, 0xB7, 0x71, 0xB6, 0x12, 0x8F, 0xD1, 0x33,
	    0xAB, 0x49, 0xAB, 0x09, 0x97, 0x35, 0x9D, 0x4B, 0xBB, 0x54, 0x22, 0x6E,
	    0x1A, 0x33, 0x18, 0x02, 0x8A, 0xF4, 0x7C, 0x0A, 0xCE, 0x89, 0x75, 0x2D,
	    0x10, 0x68, 0x25, 0xA9, 0x6E, 0xCD, 0x97, 0x49, 0xED, 0xAE, 0xE6, 0xA7,
	    0xB0, 0x07, 0x26, 0x25, 0x60, 0x15, 0x2B, 0x65, 0x88, 0x17, 0xF2, 0x5D,
	    0x2C, 0xF6, 0x2A, 0x7A, 0x8C, 0xAD, 0xB6, 0x0A, 0xA2, 0x57, 0xB0, 0xC1,
	    0x0E, 0x5C, 0xA8, 0xA1, 0x96, 0x58, 0x9A, 0x2B, 0xD4, 0xC0, 0x8A, 0xCF,
	    0x91, 0x25, 0x94, 0xB4, 0x14, 0xA7, 0xE4, 0xE2, 0x1B, 0x64, 0x5F, 0xD2,
	    0xCA, 0x70, 0x46, 0xD0, 0x2C, 0x95, 0x6B, 0x9A, 0xFB, 0x83, 0xF9, 0x76,
	    0xE6, 0xD4, 0xA4, 0xA1, 0x2B, 0x2F, 0xF5, 0x1D, 0xE4, 0x06, 0xAF, 0x7D,
	    0x22, 0xF3, 0x04, 0x30, 0x2E, 0x4C, 0x64, 0x12, 0x5B, 0xB0, 0x55, 0x3E,
	    0xC0, 0x5E, 0x56, 0xCB, 0x99, 0xBC, 0xA8, 0xD9, 0x23, 0xF5, 0x57, 0x40,
	    0xF0, 0x52, 0x85, 0x9B,
	};
	static unsigned char dh2236_g[] = {
	    0x02,
	};
	DH *dh;

	if ((dh = DH_new()) == NULL)
		return (NULL);
	dh->p = BN_bin2bn(dh2236_p, sizeof(dh2236_p), NULL);
	dh->g = BN_bin2bn(dh2236_g, sizeof(dh2236_g), NULL);
	if ((dh->p == NULL) || (dh->g == NULL)) {
		DH_free(dh);
		return (NULL);
	}
	return (dh);
}
#endif


#ifndef TEST_WITHOUT_SSL
int
init_ssl(void *ssl_context, void *user_data)
{
	/* Add application specific SSL initialization */
	struct ssl_ctx_st *ctx = (struct ssl_ctx_st *)ssl_context;

#ifdef USE_SSL_DH
	/* example from https://github.com/civetweb/civetweb/issues/347 */
	DH *dh = get_dh2236();
	if (!dh)
		return -1;
	if (1 != SSL_CTX_set_tmp_dh(ctx, dh))
		return -1;
	DH_free(dh);

	EC_KEY *ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	if (!ecdh)
		return -1;
	if (1 != SSL_CTX_set_tmp_ecdh(ctx, ecdh))
		return -1;
	EC_KEY_free(ecdh);

	printf("ECDH ciphers initialized\n");
#endif
	return 0;
}
#endif


int
log_message(const struct mg_connection *conn, const char *message)
{
	puts(message);
	return 1;
}


int
main(int argc, char *argv[])
{
	const char *options[] = {
#if !defined(NO_FILES)
		"document_root",
		DOCUMENT_ROOT,
#endif
		"listening_ports",
		PORT,
		"request_timeout_ms",
		"10000",
		"error_log_file",
		"error.log",
#ifdef USE_WEBSOCKET
		"websocket_timeout_ms",
		"3600000",
#endif
#ifndef TEST_WITHOUT_SSL
		"ssl_certificate",
		"../../resources/cert/server.pem",
		"ssl_protocol_version",
		"3",
		"ssl_cipher_list",
#ifdef USE_SSL_DH
		"ECDHE-RSA-AES256-GCM-SHA384:DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256",
#else
		"DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256",
#endif
#endif
		"enable_auth_domain_check",
		"no",
		0
	};
	struct mg_callbacks callbacks;
	struct mg_context *ctx;
	struct mg_server_port ports[32];
	int port_cnt, n;
	int err = 0;

	/* Check if libcivetweb has been built with all required features. */
#ifdef USE_IPV6
	if (!mg_check_feature(8)) {
		fprintf(stderr,
		        "Error: Embedded example built with IPv6 support, "
		        "but civetweb library build without.\n");
		err = 1;
	}
#endif
#ifdef USE_WEBSOCKET
	if (!mg_check_feature(16)) {
		fprintf(stderr,
		        "Error: Embedded example built with websocket support, "
		        "but civetweb library build without.\n");
		err = 1;
	}
#endif
#ifndef TEST_WITHOUT_SSL
	if (!mg_check_feature(2)) {
		fprintf(stderr,
		        "Error: Embedded example built with SSL support, "
		        "but civetweb library build without.\n");
		err = 1;
	}
#endif
	if (err) {
		fprintf(stderr, "Cannot start CivetWeb - inconsistent build.\n");
		return EXIT_FAILURE;
	}

	/* Start CivetWeb web server */
	memset(&callbacks, 0, sizeof(callbacks));
#ifndef TEST_WITHOUT_SSL
	callbacks.init_ssl = init_ssl;
#endif
	callbacks.log_message = log_message;
	ctx = mg_start(&callbacks, 0, options);

	/* Check return value: */
	if (ctx == NULL) {
		fprintf(stderr, "Cannot start CivetWeb - mg_start failed.\n");
		return EXIT_FAILURE;
	}

	/* Add handler EXAMPLE_URI, to explain the example */
	mg_set_request_handler(ctx, EXAMPLE_URI, ExampleHandler, 0);
	mg_set_request_handler(ctx, EXIT_URI, ExitHandler, 0);

	/* Add handler for /A* and special handler for /A/B */
	mg_set_request_handler(ctx, "/A", AHandler, 0);
	mg_set_request_handler(ctx, "/A/B", ABHandler, 0);

	/* Add handler for /B, /B/A, /B/B but not for /B* */
	mg_set_request_handler(ctx, "/B$", BXHandler, (void *)"alpha");
	mg_set_request_handler(ctx, "/B/A$", BXHandler, (void *)"beta");
	mg_set_request_handler(ctx, "/B/B$", BXHandler, (void *)"gamma");

	/* Add handler for all files with .foo extension */
	mg_set_request_handler(ctx, "**.foo$", FooHandler, 0);

	/* Add handler for /close extension */
	mg_set_request_handler(ctx, "/close", CloseHandler, 0);

#if !defined(NO_FILESYSTEMS)
	/* Add handler for /form  (serve a file outside the document root) */
	mg_set_request_handler(ctx,
	                       "/form",
	                       FileHandler,
	                       (void *)"../../test/form.html");
#endif /* NO_FILESYSTEMS */

	/* Add handler for form data */
	mg_set_request_handler(ctx,
	                       "/handle_form.embedded_c.example.callback",
	                       FormHandler,
	                       (void *)0);

	/* Add a file upload handler for parsing files on the fly */
	mg_set_request_handler(ctx,
	                       "/on_the_fly_form",
	                       FileUploadForm,
	                       (void *)"/on_the_fly_form.md5.callback");
	mg_set_request_handler(ctx,
	                       "/on_the_fly_form.md5.callback",
	                       CheckSumHandler,
	                       (void *)0);

	/* Add handler for /cookie example */
	mg_set_request_handler(ctx, "/cookie", CookieHandler, 0);

	/* Add handler for /postresponse example */
	mg_set_request_handler(ctx, "/postresponse", PostResponser, 0);

	/* Add HTTP site to open a websocket connection */
	mg_set_request_handler(ctx, "/websocket", WebSocketStartHandler, 0);

#if !defined(NO_FILESYSTEMS)
	/* Add HTTP site with auth */
	mg_set_request_handler(ctx, "/auth", AuthStartHandler, 0);
#endif /* NO_FILESYSTEMS */


#ifdef USE_WEBSOCKET
	/* WS site for the websocket connection */
	mg_set_websocket_handler(ctx,
	                         "/websocket",
	                         WebSocketConnectHandler,
	                         WebSocketReadyHandler,
	                         WebsocketDataHandler,
	                         WebSocketCloseHandler,
	                         0);
#endif

	/* List all listening ports */
	memset(ports, 0, sizeof(ports));
	port_cnt = mg_get_server_ports(ctx, 32, ports);
	printf("\n%i listening ports:\n\n", port_cnt);

	for (n = 0; n < port_cnt && n < 32; n++) {
		const char *proto = ports[n].is_ssl ? "https" : "http";
		const char *host;

		if ((ports[n].protocol & 1) == 1) {
			/* IPv4 */
			host = "127.0.0.1";
			printf("Browse files at %s://%s:%i/\n", proto, host, ports[n].port);
			printf("Run example at %s://%s:%i%s\n",
			       proto,
			       host,
			       ports[n].port,
			       EXAMPLE_URI);
			printf(
			    "Exit at %s://%s:%i%s\n", proto, host, ports[n].port, EXIT_URI);
			printf("\n");
		}

		if ((ports[n].protocol & 2) == 2) {
			/* IPv6 */
			host = "[::1]";
			printf("Browse files at %s://%s:%i/\n", proto, host, ports[n].port);
			printf("Run example at %s://%s:%i%s\n",
			       proto,
			       host,
			       ports[n].port,
			       EXAMPLE_URI);
			printf(
			    "Exit at %s://%s:%i%s\n", proto, host, ports[n].port, EXIT_URI);
			printf("\n");
		}
	}

	/* Wait until the server should be closed */
	while (!exitNow) {
#ifdef _WIN32
		Sleep(1000);
#else
		sleep(1);
#endif
#ifdef USE_WEBSOCKET
		InformWebsockets(ctx);
#endif
	}

	/* Stop the server */
	mg_stop(ctx);
	printf("Server stopped.\n");
	printf("Bye!\n");

	return EXIT_SUCCESS;
}
