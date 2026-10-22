
#ifndef WEBSOCKCALLBACKS_H_INCLUDED
#define WEBSOCKCALLBACKS_H_INCLUDED

#include "civetweb.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tWebSockInfo {
	int webSockState;
	unsigned long initId;
	struct mg_connection *conn;
} tWebSockInfo;

#define MAX_NUM_OF_WEBSOCKS (256)
typedef struct tWebSockContext {
	int runLoop;
	void *thread_id;
	tWebSockInfo *socketList[MAX_NUM_OF_WEBSOCKS];
} tWebSockContext;


void websock_init_lib(const struct mg_context *ctx);
void websock_exit_lib(const struct mg_context *ctx);

void
websock_send_broadcast(struct mg_context *ctx, const char *data, int data_len);

void websocket_ready_handler(struct mg_connection *conn, void *_ignored);
int websocket_data_handler(struct mg_connection *conn,
                           int flags,
                           char *data,
                           size_t data_len,
                           void *_ignored);
void connection_close_handler(const struct mg_connection *conn, void *_ignored);


#ifdef __cplusplus
}
#endif

#endif