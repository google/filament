/* Copyright (c) 2014 the Civetweb developers
 * Copyright (c) 2004-2012 Sergey Lyubka
 * This file is a part of civetweb project, http://github.com/bel2125/civetweb
 */

/* This example is deprecated and no longer maintained.
 * All relevant parts have been merged into the embedded_c example. */


#ifdef _WIN32
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <windows.h>
#include <io.h>
#define strtoll strtol
typedef __int64 int64_t;
#else
#include <inttypes.h>
#include <unistd.h>
#endif /* !_WIN32 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

#include "civetweb.h"


/* callback: used to generate all content */
static int begin_request_handler(struct mg_connection *conn)
{
    const char * tempPath = ".";
#ifdef _WIN32
    const char * env = getenv("TEMP");
    if (!env) env = getenv("TMP");
    if (env) tempPath = env;
#else
    tempPath = "/tmp";
#endif

    if (!strcmp(mg_get_request_info(conn)->uri, "/handle_post_request")) {

        mg_printf(conn, "%s", "HTTP/1.0 200 OK\r\n\r\n");
        mg_upload(conn, tempPath);
    } else {
        /* Show HTML form. */
        /* See http://www.w3.org/TR/html401/interact/forms.html#h-17.13.4.1 */
        static const char *html_form =
            "<html><body>Upload example."
            ""
            /* enctype="multipart/form-data" */
            "<form method=\"POST\" action=\"/handle_post_request\" "
            "  enctype=\"multipart/form-data\">"
            "<input type=\"file\" name=\"file\" /> <br/>"
            "<input type=\"file\" name=\"file2\" /> <br/>"
            "<input type=\"submit\" value=\"Upload\" />"
            "</form>"
            ""
            "</body></html>";

        mg_printf(conn, "HTTP/1.0 200 OK\r\n"
                  "Content-Length: %d\r\n"
                  "Content-Type: text/html\r\n\r\n%s",
                  (int) strlen(html_form), html_form);
    }

    /* Mark request as processed */
    return 1;
}


/* callback: called after uploading a file is completed */
static void upload_handler(struct mg_connection *conn, const char *path)
{
    mg_printf(conn, "Saved [%s]", path);
}


/* Main program: Set callbacks and start the server.  */
int main(void)
{
    /* Test server will use this port */
    const char * PORT = "8080";

    /* Startup options for the server */
    struct mg_context *ctx;
    const char *options[] = {
        "listening_ports", PORT,
        NULL};
    struct mg_callbacks callbacks;

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.begin_request = begin_request_handler;
    callbacks.upload = upload_handler;

    /* Display a welcome message */
    printf("File upload demo.\n");
    printf("Open http://localhost:%s/ in your browser.\n\n", PORT);

    /* Start the server */
    ctx = mg_start(&callbacks, NULL, options);

    /* Wait until thr user hits "enter", then stop the server */
    getchar();
    mg_stop(ctx);

    return 0;
}
