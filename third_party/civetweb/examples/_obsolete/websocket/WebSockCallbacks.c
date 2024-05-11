/* This example uses deprecated interfaces: global websocket callbacks.
   They have been superseeded by URI specific callbacks.
   See examples/embedded_c for an up to date example.
   */

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "WebSockCallbacks.h"

#ifdef _WIN32
#include <windows.h>
#define mg_sleep(x) Sleep(x)
#else
#include <unistd.h>
#include <pthread.h>
#define mg_sleep(x) usleep((x)*1000)
#endif


static void
send_to_all_websockets(struct mg_context *ctx, const char *data, int data_len)
{

	int i;
	tWebSockContext *ws_ctx = (tWebSockContext *)mg_get_user_data(ctx);

	mg_lock_context(ctx);
	for (i = 0; i < MAX_NUM_OF_WEBSOCKS; i++) {
		if (ws_ctx->socketList[i]
		    && (ws_ctx->socketList[i]->webSockState == 2)) {
			mg_websocket_write(ws_ctx->socketList[i]->conn,
			                   WEBSOCKET_OPCODE_TEXT,
			                   data,
			                   data_len);
		}
	}
	mg_unlock_context(ctx);
}


void
websocket_ready_handler(struct mg_connection *conn, void *_ignored)
{

	int i;
	const struct mg_request_info *rq = mg_get_request_info(conn);
	struct mg_context *ctx = mg_get_context(conn);
	tWebSockContext *ws_ctx = (tWebSockContext *)mg_get_user_data(ctx);
	tWebSockInfo *wsock = malloc(sizeof(tWebSockInfo));
	assert(wsock);
	wsock->webSockState = 0;
	mg_set_user_connection_data(conn, wsock);

	mg_lock_context(ctx);
	for (i = 0; i < MAX_NUM_OF_WEBSOCKS; i++) {
		if (0 == ws_ctx->socketList[i]) {
			ws_ctx->socketList[i] = wsock;
			wsock->conn = conn;
			wsock->webSockState = 1;
			break;
		}
	}
	printf("\nNew websocket attached: %s:%u\n",
	       rq->remote_addr,
	       rq->remote_port);
	mg_unlock_context(ctx);
}


static void
websocket_done(tWebSockContext *ws_ctx, tWebSockInfo *wsock)
{

	int i;

	if (wsock) {
		wsock->webSockState = 99;
		for (i = 0; i < MAX_NUM_OF_WEBSOCKS; i++) {
			if (wsock == ws_ctx->socketList[i]) {
				ws_ctx->socketList[i] = 0;
				break;
			}
		}
		printf("\nClose websocket attached: %s:%u\n",
		       mg_get_request_info(wsock->conn)->remote_addr,
		       mg_get_request_info(wsock->conn)->remote_port);
		free(wsock);
	}
}


int
websocket_data_handler(struct mg_connection *conn,
                       int flags,
                       char *data,
                       size_t data_len,
                       void *_ignored)
{

	const struct mg_request_info *rq = mg_get_request_info(conn);
	tWebSockInfo *wsock = (tWebSockInfo *)rq->conn_data;
	struct mg_context *ctx = mg_get_context(conn);
	tWebSockContext *ws_ctx = (tWebSockContext *)mg_get_user_data(ctx);
	char msg[128];

	mg_lock_context(ctx);
	if (flags == 136) {
		// close websock
		websocket_done(ws_ctx, wsock);
		mg_set_user_connection_data(conn, NULL);
		mg_unlock_context(ctx);
		return 1;
	}
	if (((data_len >= 5) && (data_len < 100) && (flags == 129))
	    || (flags == 130)) {

		// init command
		if ((wsock->webSockState == 1) && (!memcmp(data, "init ", 5))) {
			char *chk;
			unsigned long gid;
			memcpy(msg, data + 5, data_len - 5);
			msg[data_len - 5] = 0;
			gid = strtoul(msg, &chk, 10);
			wsock->initId = gid;
			if (gid > 0 && chk != NULL && *chk == 0) {
				wsock->webSockState = 2;
			}
			mg_unlock_context(ctx);
			return 1;
		}

		// chat message
		if ((wsock->webSockState == 2) && (!memcmp(data, "msg ", 4))) {
			send_to_all_websockets(ctx, data, data_len);
			mg_unlock_context(ctx);
			return 1;
		}
	}

	// keep alive
	if ((data_len == 4) && !memcmp(data, "ping", 4)) {
		mg_unlock_context(ctx);
		return 1;
	}

	mg_unlock_context(ctx);
	return 0;
}


void
connection_close_handler(const struct mg_connection *conn, void *_ignored)
{

	const struct mg_request_info *rq = mg_get_request_info(conn);
	tWebSockInfo *wsock = (tWebSockInfo *)rq->conn_data;
	struct mg_context *ctx = mg_get_context(conn);
	tWebSockContext *ws_ctx = (tWebSockContext *)mg_get_user_data(ctx);

	mg_lock_context(ctx);
	websocket_done(ws_ctx, wsock);
	mg_set_user_connection_data(conn, NULL);
	mg_unlock_context(ctx);
}


static void *
eventMain(void *arg)
{

	char msg[256];
	struct mg_context *ctx = (struct mg_context *)arg;
	tWebSockContext *ws_ctx = (tWebSockContext *)mg_get_user_data(ctx);

	ws_ctx->runLoop = 1;
	while (ws_ctx->runLoop) {
		time_t t = time(0);
		struct tm *timestr = localtime(&t);
		strftime(msg, sizeof(msg), "title %c", timestr);
		send_to_all_websockets(ctx, msg, strlen(msg));

		mg_sleep(1000);
	}

	return NULL;
}


void
websock_send_broadcast(struct mg_context *ctx, const char *data, int data_len)
{

	char buffer[260];

	if (data_len <= 256) {
		strcpy(buffer, "msg ");
		memcpy(buffer + 4, data, data_len);

		send_to_all_websockets(ctx, buffer, data_len + 4);
	}
}


void
websock_init_lib(const struct mg_context *ctx)
{

	tWebSockContext *ws_ctx = (tWebSockContext *)mg_get_user_data(ctx);
	memset(ws_ctx, 0, sizeof(*ws_ctx));
	/* todo: use mg_start_thread_id instead of mg_start_thread */
	mg_start_thread(eventMain, (void *)ctx);
}


void
websock_exit_lib(const struct mg_context *ctx)
{

	tWebSockContext *ws_ctx = (tWebSockContext *)mg_get_user_data(ctx);
	ws_ctx->runLoop = 0;
	/* todo: wait for the thread instead of a timeout */
	mg_sleep(2000);
}
