/* Copyright (c) 2013-2018 the Civetweb developers
 * Copyright (c) 2013 No Face Press, LLC
 * License http://opensource.org/licenses/mit-license.php MIT License
 */

// Simple example program on how to use Embedded C++ interface.

#include "CivetServer.h"
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define DOCUMENT_ROOT "."
#define PORT "8081"
#define EXAMPLE_URI "/example"
#define EXIT_URI "/exit"


/* Exit flag for main loop */
volatile bool exitNow = false;


class ExampleHandler : public CivetHandler
{
  public:
	bool
	handleGet(CivetServer *server, struct mg_connection *conn)
	{
		mg_printf(conn,
		          "HTTP/1.1 200 OK\r\nContent-Type: "
		          "text/html\r\nConnection: close\r\n\r\n");
		mg_printf(conn, "<html><body>\r\n");
		mg_printf(conn,
		          "<h2>This is an example text from a C++ handler</h2>\r\n");
		mg_printf(conn,
		          "<p>To see a page from the A handler <a "
		          "href=\"a\">click here</a></p>\r\n");
		mg_printf(conn,
                  "<form action=\"a\" method=\"get\">"
                  "To see a page from the A handler with a parameter "
                  "<input type=\"submit\" value=\"click here\" "
                  "name=\"param\" \\> (GET)</form>\r\n");
        mg_printf(conn,
                  "<form action=\"a\" method=\"post\">"
                  "To see a page from the A handler with a parameter "
                  "<input type=\"submit\" value=\"click here\" "
                  "name=\"param\" \\> (POST)</form>\r\n");
		mg_printf(conn,
		          "<p>To see a page from the A/B handler <a "
		          "href=\"a/b\">click here</a></p>\r\n");
		mg_printf(conn,
		          "<p>To see a page from the *.foo handler <a "
		          "href=\"xy.foo\">click here</a></p>\r\n");
		mg_printf(conn,
		          "<p>To see a page from the WebSocket handler <a "
		          "href=\"ws\">click here</a></p>\r\n");
		mg_printf(conn,
		          "<p>To exit <a href=\"%s\">click here</a></p>\r\n",
		          EXIT_URI);
		mg_printf(conn, "</body></html>\r\n");
		return true;
	}
};

class ExitHandler : public CivetHandler
{
  public:
	bool
	handleGet(CivetServer *server, struct mg_connection *conn)
	{
		mg_printf(conn,
		          "HTTP/1.1 200 OK\r\nContent-Type: "
		          "text/plain\r\nConnection: close\r\n\r\n");
		mg_printf(conn, "Bye!\n");
		exitNow = true;
		return true;
	}
};

class AHandler : public CivetHandler
{
  private:
	bool
	handleAll(const char *method,
	          CivetServer *server,
	          struct mg_connection *conn)
	{
		std::string s = "";
		mg_printf(conn,
		          "HTTP/1.1 200 OK\r\nContent-Type: "
		          "text/html\r\nConnection: close\r\n\r\n");
		mg_printf(conn, "<html><body>");
		mg_printf(conn, "<h2>This is the A handler for \"%s\" !</h2>", method);
		if (CivetServer::getParam(conn, "param", s)) {
			mg_printf(conn, "<p>param set to %s</p>", s.c_str());
		} else {
			mg_printf(conn, "<p>param not set</p>");
		}
		mg_printf(conn, "</body></html>\n");
		return true;
	}

  public:
	bool
	handleGet(CivetServer *server, struct mg_connection *conn)
	{
		return handleAll("GET", server, conn);
	}
	bool
	handlePost(CivetServer *server, struct mg_connection *conn)
	{
		return handleAll("POST", server, conn);
	}
};

class ABHandler : public CivetHandler
{
  public:
	bool
	handleGet(CivetServer *server, struct mg_connection *conn)
	{
		mg_printf(conn,
		          "HTTP/1.1 200 OK\r\nContent-Type: "
		          "text/html\r\nConnection: close\r\n\r\n");
		mg_printf(conn, "<html><body>");
		mg_printf(conn, "<h2>This is the AB handler!!!</h2>");
		mg_printf(conn, "</body></html>\n");
		return true;
	}
};

class FooHandler : public CivetHandler
{
  public:
	bool
	handleGet(CivetServer *server, struct mg_connection *conn)
	{
		/* Handler may access the request info using mg_get_request_info */
		const struct mg_request_info *req_info = mg_get_request_info(conn);

		mg_printf(conn,
		          "HTTP/1.1 200 OK\r\nContent-Type: "
		          "text/html\r\nConnection: close\r\n\r\n");

		mg_printf(conn, "<html><body>\n");
		mg_printf(conn, "<h2>This is the Foo GET handler!!!</h2>\n");
		mg_printf(conn,
		          "<p>The request was:<br><pre>%s %s HTTP/%s</pre></p>\n",
		          req_info->request_method,
		          req_info->request_uri,
		          req_info->http_version);
		mg_printf(conn, "</body></html>\n");

		return true;
	}
	bool
	handlePost(CivetServer *server, struct mg_connection *conn)
	{
		/* Handler may access the request info using mg_get_request_info */
		const struct mg_request_info *req_info = mg_get_request_info(conn);
		long long rlen, wlen;
		long long nlen = 0;
		long long tlen = req_info->content_length;
		char buf[1024];

		mg_printf(conn,
		          "HTTP/1.1 200 OK\r\nContent-Type: "
		          "text/html\r\nConnection: close\r\n\r\n");

		mg_printf(conn, "<html><body>\n");
		mg_printf(conn, "<h2>This is the Foo POST handler!!!</h2>\n");
		mg_printf(conn,
		          "<p>The request was:<br><pre>%s %s HTTP/%s</pre></p>\n",
		          req_info->request_method,
		          req_info->request_uri,
		          req_info->http_version);
		mg_printf(conn, "<p>Content Length: %li</p>\n", (long)tlen);
		mg_printf(conn, "<pre>\n");

		while (nlen < tlen) {
			rlen = tlen - nlen;
			if (rlen > sizeof(buf)) {
				rlen = sizeof(buf);
			}
			rlen = mg_read(conn, buf, (size_t)rlen);
			if (rlen <= 0) {
				break;
			}
			wlen = mg_write(conn, buf, (size_t)rlen);
			if (wlen != rlen) {
				break;
			}
			nlen += wlen;
		}

		mg_printf(conn, "\n</pre>\n");
		mg_printf(conn, "</body></html>\n");

		return true;
	}

    #define fopen_recursive fopen

    bool
        handlePut(CivetServer *server, struct mg_connection *conn)
    {
        /* Handler may access the request info using mg_get_request_info */
        const struct mg_request_info *req_info = mg_get_request_info(conn);
        long long rlen, wlen;
        long long nlen = 0;
        long long tlen = req_info->content_length;
        FILE * f;
        char buf[1024];
        int fail = 0;

#ifdef _WIN32
        _snprintf(buf, sizeof(buf), "D:\\somewhere\\%s\\%s", req_info->remote_user, req_info->local_uri);
        buf[sizeof(buf)-1] = 0;
        if (strlen(buf)>255) {
            /* Windows will not work with path > 260 (MAX_PATH), unless we use
             * the unicode API. However, this is just an example code: A real
             * code will probably never store anything to D:\\somewhere and
             * must be adapted to the specific needs anyhow. */
            fail = 1;
            f = NULL;
        } else {
            f = fopen_recursive(buf, "wb");
        }
#else
        snprintf(buf, sizeof(buf), "~/somewhere/%s/%s", req_info->remote_user, req_info->local_uri);
        buf[sizeof(buf)-1] = 0;
        if (strlen(buf)>1020) {
            /* The string is too long and probably truncated. Make sure an
             * UTF-8 string is never truncated between the UTF-8 code bytes.
             * This example code must be adapted to the specific needs. */
            fail = 1;
            f = NULL;
        } else {
            f = fopen_recursive(buf, "w");
        }
#endif

        if (!f) {
            fail = 1;
        } else {
            while (nlen < tlen) {
                rlen = tlen - nlen;
                if (rlen > sizeof(buf)) {
                    rlen = sizeof(buf);
                }
                rlen = mg_read(conn, buf, (size_t)rlen);
                if (rlen <= 0) {
                    fail = 1;
                    break;
                }
                wlen = fwrite(buf, 1, (size_t)rlen, f);
                if (wlen != rlen) {
                    fail = 1;
                    break;
                }
                nlen += wlen;
            }
            fclose(f);
        }

        if (fail) {
            mg_printf(conn,
                "HTTP/1.1 409 Conflict\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n\r\n");
        } else {
            mg_printf(conn,
                "HTTP/1.1 201 Created\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n\r\n");
        }

        return true;
    }
};

class WsStartHandler : public CivetHandler
{
  public:
	bool
	handleGet(CivetServer *server, struct mg_connection *conn)
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
	    "var i=0\n"
	    "function load() {\n"
	    "  var wsproto = (location.protocol === 'https:') ? 'wss:' : 'ws:';\n"
	    "  connection = new WebSocket(wsproto + '//' + window.location.host + "
	    "'/websocket');\n"
	    "  websock_text_field = "
	    "document.getElementById('websock_text_field');\n"
	    "  connection.onmessage = function (e) {\n"
	    "    websock_text_field.innerHTML=e.data;\n"
	    "    i=i+1;"
	    "    connection.send(i);\n"
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
};


#ifdef USE_WEBSOCKET
class WebSocketHandler : public CivetWebSocketHandler {

	virtual bool handleConnection(CivetServer *server,
	                              const struct mg_connection *conn) {
		printf("WS connected\n");
		return true;
	}

	virtual void handleReadyState(CivetServer *server,
	                              struct mg_connection *conn) {
		printf("WS ready\n");

		const char *text = "Hello from the websocket ready handler";
		mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, text, strlen(text));
	}

	virtual bool handleData(CivetServer *server,
	                        struct mg_connection *conn,
	                        int bits,
	                        char *data,
	                        size_t data_len) {
		printf("WS got %lu bytes: ", (long unsigned)data_len);
		fwrite(data, 1, data_len, stdout);
		printf("\n");

		mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, data, data_len);
		return (data_len<4);
	}

	virtual void handleClose(CivetServer *server,
	                         const struct mg_connection *conn) {
		printf("WS closed\n");
	}
};
#endif


int
main(int argc, char *argv[])
{
	const char *options[] = {
	    "document_root", DOCUMENT_ROOT, "listening_ports", PORT, 0};
    
    std::vector<std::string> cpp_options;
    for (int i=0; i<(sizeof(options)/sizeof(options[0])-1); i++) {
        cpp_options.push_back(options[i]);
    }

	// CivetServer server(options); // <-- C style start
	CivetServer server(cpp_options); // <-- C++ style start

	ExampleHandler h_ex;
	server.addHandler(EXAMPLE_URI, h_ex);

	ExitHandler h_exit;
	server.addHandler(EXIT_URI, h_exit);

	AHandler h_a;
	server.addHandler("/a", h_a);

	ABHandler h_ab;
	server.addHandler("/a/b", h_ab);

	WsStartHandler h_ws;
	server.addHandler("/ws", h_ws);

#ifdef NO_FILES
	/* This handler will handle "everything else", including
	 * requests to files. If this handler is installed,
	 * NO_FILES should be set. */
	FooHandler h_foo;
	server.addHandler("", h_foo);

	printf("See a page from the \"all\" handler at http://localhost:%s/\n", PORT);
#else
	FooHandler h_foo;
	server.addHandler("**.foo", h_foo);
	printf("Browse files at http://localhost:%s/\n", PORT);
#endif

#ifdef USE_WEBSOCKET
	WebSocketHandler h_websocket;
	server.addWebSocketHandler("/websocket", h_websocket);
	printf("Run websocket example at http://localhost:%s/ws\n", PORT);
#endif

	printf("Run example at http://localhost:%s%s\n", PORT, EXAMPLE_URI);
	printf("Exit at http://localhost:%s%s\n", PORT, EXIT_URI);

	while (!exitNow) {
#ifdef _WIN32
		Sleep(1000);
#else
		sleep(1);
#endif
	}

	printf("Bye!\n");

	return 0;
}
