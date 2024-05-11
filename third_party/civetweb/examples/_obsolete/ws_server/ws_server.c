// Copyright (c) 2004-2012 Sergey Lyubka
// This file is a part of civetweb project, http://github.com/bel2125/civetweb
//
// v 0.1 Contributed by William Greathouse    9-Sep-2013

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "civetweb.h"

// simple structure for keeping track of websocket connection
struct ws_connection {
    struct mg_connection    *conn;
    int     update;
    int     closing;
};

// time base and structure periodic updates to client for demo
#define BASETIME 100000 /* 0.1 seconds */
struct progress {
    int     limit;
    int     increment;
    int     period;
    int     value;
};

// up to 16 independent client connections
#define CONNECTIONS 16
static struct ws_connection ws_conn[CONNECTIONS];


// ws_server_thread()
// Simple demo server thread. Sends periodic updates to connected clients
static void *ws_server_thread(void *parm)
{
    int wsd = (long)parm;
    struct mg_connection    *conn = ws_conn[wsd].conn;
    int timer = 0;
    char tstr[32];
    int i;
    struct progress meter[] = {
        /* first meter 0 to 1000, by 5 every 0.1 second */
        { 1000, 5, 1, 0 },
        /* second meter 0 to 500, by 10 every 0.5 second */
        { 500, 10, 5, 0 },
        /* third meter 0 to 100, by 10 every 1.0 second */
        { 100, 10, 10, 0},
        /* end of list */
        { 0, 0, 0, 0}
    };

    fprintf(stderr, "ws_server_thread %d\n", wsd);

    /* Send initial meter updates */
    for (i=0; meter[i].period != 0; i++) {
        if (meter[i].value >= meter[i].limit)
            meter[i].value = 0;
        if (meter[i].value >= meter[i].limit)
            meter[i].value = meter[i].limit;
        sprintf(tstr, "meter%d:%d,%d", i+1,
                meter[i].value, meter[i].limit);
        mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, tstr, strlen(tstr));
    }

    /* While the connection is open, send periodic updates */
    while(!ws_conn[wsd].closing) {
        usleep(100000); /* 0.1 second */
        timer++;

        /* Send meter updates */
        if (ws_conn[wsd].update) {
            for (i=0; meter[i].period != 0; i++) {
                if (timer%meter[i].period == 0) {
                    if (meter[i].value >= meter[i].limit)
                        meter[i].value = 0;
                    else
                        meter[i].value += meter[i].increment;
                    if (meter[i].value >= meter[i].limit)
                        meter[i].value = meter[i].limit;
                    // if we are closing, server should not send new data
                    if (!ws_conn[wsd].closing) {
                        sprintf(tstr, "meter%d:%d,%d", i+1,
                                meter[i].value, meter[i].limit);
                        mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, tstr, strlen(tstr));
                    }
                }
            }
        }

        /* Send periodic PING to assure websocket remains connected, except if we are closing */
        if (timer%100 == 0 && !ws_conn[wsd].closing)
            mg_websocket_write(conn, WEBSOCKET_OPCODE_PING, NULL, 0);
    }

    fprintf(stderr, "ws_server_thread %d exiting\n", wsd);

    // reset connection information to allow reuse by new client
    ws_conn[wsd].conn = NULL;
    ws_conn[wsd].update = 0;
    ws_conn[wsd].closing = 2;

    return NULL;
}

// websocket_connect_handler()
// On new client connection, find next available server connection and store
// new connection information. If no more server connections are available
// tell civetweb to not accept the client request.
static int websocket_connect_handler(const struct mg_connection *conn)
{
    int i;

    fprintf(stderr, "connect handler\n");

    for(i=0; i < CONNECTIONS; ++i) {
        if (ws_conn[i].conn == NULL) {
            fprintf(stderr, "...prep for server %d\n", i);
            ws_conn[i].conn = (struct mg_connection *)conn;
            ws_conn[i].closing = 0;
            ws_conn[i].update = 0;
            break;
        }
    }
    if (i >= CONNECTIONS) {
        fprintf(stderr, "Refused connection: Max connections exceeded\n");
        return 1;
    }

    return 0;
}

// websocket_ready_handler()
// Once websocket negotiation is complete, start a server for the connection
static void websocket_ready_handler(struct mg_connection *conn)
{
    int i;

    fprintf(stderr, "ready handler\n");

    for(i=0; i < CONNECTIONS; ++i) {
        if (ws_conn[i].conn == conn) {
            fprintf(stderr, "...start server %d\n", i);
            mg_start_thread(ws_server_thread, (void *)(long)i);
            break;
        }
    }
}

// websocket_close_handler()
// When websocket is closed, tell the associated server to shut down
static void websocket_close_handler(struct mg_connection *conn)
{
    int i;

    //fprintf(stderr, "close handler\n");   /* called for every close, not just websockets */

    for(i=0; i < CONNECTIONS; ++i) {
        if (ws_conn[i].conn == conn) {
            fprintf(stderr, "...close server %d\n", i);
            ws_conn[i].closing = 1;
        }
    }
}

// Arguments:
//   flags: first byte of websocket frame, see websocket RFC,
//          http://tools.ietf.org/html/rfc6455, section 5.2
//   data, data_len: payload data. Mask, if any, is already applied.
static int websocket_data_handler(struct mg_connection *conn, int flags,
                                  char *data, size_t data_len)
{
    int i;
    int wsd;

    for(i=0; i < CONNECTIONS; ++i) {
        if (ws_conn[i].conn == conn) {
            wsd = i;
            break;
        }
    }
    if (i >= CONNECTIONS) {
        fprintf(stderr, "Received websocket data from unknown connection\n");
        return 1;
    }

    if (flags & 0x80) {
        flags &= 0x7f;
        switch (flags) {
        case WEBSOCKET_OPCODE_CONTINUATION:
            fprintf(stderr, "CONTINUATION...\n");
            break;
        case WEBSOCKET_OPCODE_TEXT:
            fprintf(stderr, "TEXT: %-.*s\n", (int)data_len, data);
            /*** interpret data as commands here ***/
            if (strncmp("update on", data, data_len)== 0) {
                /* turn on updates */
                ws_conn[wsd].update = 1;
                /* echo back */
                mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, data, data_len);
            } else if (strncmp("update off", data, data_len)== 0) {
                /* turn off updates */
                ws_conn[wsd].update = 0;
                /* echo back */
                mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, data, data_len);
            }
            break;
        case WEBSOCKET_OPCODE_BINARY:
            fprintf(stderr, "BINARY...\n");
            break;
        case WEBSOCKET_OPCODE_CONNECTION_CLOSE:
            fprintf(stderr, "CLOSE...\n");
            /* If client initiated close, respond with close message in acknowlegement */
            if (!ws_conn[wsd].closing) {
                mg_websocket_write(conn, WEBSOCKET_OPCODE_CONNECTION_CLOSE, data, data_len);
                ws_conn[wsd].closing = 1; /* we should not send additional messages when close requested/acknowledged */
            }
            return 0; /* time to close the connection */
            break;
        case WEBSOCKET_OPCODE_PING:
            /* client sent PING, respond with PONG */
            mg_websocket_write(conn, WEBSOCKET_OPCODE_PONG, data, data_len);
            break;
        case WEBSOCKET_OPCODE_PONG:
            /* received PONG to our PING, no action */
            break;
        default:
            fprintf(stderr, "Unknown flags: %02x\n", flags);
            break;
        }
    }

    return 1;   /* keep connection open */
}


int main(void)
{
    char server_name[40];
    struct mg_context *ctx;
    struct mg_callbacks callbacks;
    const char *options[] = {
        "listening_ports", "8080",
        "document_root", "docroot",
        NULL
    };

    /* get simple greeting for the web server */
    snprintf(server_name, sizeof(server_name),
             "Civetweb websocket server v. %s",
             mg_version());

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.websocket_connect = websocket_connect_handler;
    callbacks.websocket_ready = websocket_ready_handler;
    callbacks.websocket_data = websocket_data_handler;
    callbacks.connection_close = websocket_close_handler;

    ctx = mg_start(&callbacks, NULL, options);

    /* show the greeting and some basic information */
    printf("%s started on port(s) %s with web root [%s]\n",
           server_name, mg_get_option(ctx, "listening_ports"),
           mg_get_option(ctx, "document_root"));

    getchar();  // Wait until user hits "enter"
    mg_stop(ctx);

    return 0;
}
