#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "civetweb.h"

/* Get an OS independent definition for sleep() */
#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep((x)*1000)
#else
#include <unistd.h>
#endif


/* User defined client data structure */
struct tclient_data {

	time_t started;
	time_t closed;
	struct tmsg_list_elem *msgs;
};

struct tmsg_list_elem {
	time_t timestamp;
	void *data;
	size_t len;
	struct tmsg_list_elem *next;
};


/* Helper function to get a printable name for websocket opcodes */
static const char *
msgtypename(int flags)
{
	unsigned f = (unsigned)flags & 0xFu;
	switch (f) {
	case MG_WEBSOCKET_OPCODE_CONTINUATION:
		return "continuation";
	case MG_WEBSOCKET_OPCODE_TEXT:
		return "text";
	case MG_WEBSOCKET_OPCODE_BINARY:
		return "binary";
	case MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE:
		return "connection close";
	case MG_WEBSOCKET_OPCODE_PING:
		return "PING";
	case MG_WEBSOCKET_OPCODE_PONG:
		return "PONG";
	}
	return "unknown";
}


/* Callback for handling data received from the server */
static int
websocket_client_data_handler(struct mg_connection *conn,
                              int flags,
                              char *data,
                              size_t data_len,
                              void *user_data)
{
	struct tclient_data *pclient_data = (struct tclient_data *)user_data;
	time_t now = time(NULL);

	/* We may get some different message types (websocket opcodes).
	 * We will handle these messages differently. */
	int is_text = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_TEXT);
	int is_bin = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_BINARY);
	int is_ping = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_PING);
	int is_pong = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_PONG);
	int is_close = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE);

	/* Log output: We got some data */
	printf("%10.0f - Client received %lu bytes of %s data from server%s",
	       difftime(now, pclient_data->started),
	       (long unsigned)data_len,
	       msgtypename(flags),
	       (is_text ? ": " : ".\n"));

	/* Check if we got a websocket PING request */
	if (is_ping) {
		/* PING requests are to check if the connection is broken.
		 * They should be replied with a PONG with the same data.
		 */
		mg_websocket_client_write(conn,
		                          MG_WEBSOCKET_OPCODE_PONG,
		                          data,
		                          data_len);
		return 1;
	}

	/* Check if we got a websocket PONG message */
	if (is_pong) {
		/* A PONG message may be a response to our PING, but
		 * it is also allowed to send unsolicited PONG messages
		 * send by the server to check some lower level TCP
		 * connections. Just ignore all kinds of PONGs. */
		return 1;
	}

	/* It we got a websocket TEXT message, handle it ... */
	if (is_text) {
		struct tmsg_list_elem *p;
		struct tmsg_list_elem **where = &(pclient_data->msgs);

		/* ... by printing it to the log ... */
		fwrite(data, 1, data_len, stdout);
		printf("\n");

		/* ... and storing it (OOM ignored for simplicity). */
		p = (struct tmsg_list_elem *)malloc(sizeof(struct tmsg_list_elem));
		p->timestamp = now;
		p->data = malloc(data_len);
		memcpy(p->data, data, data_len);
		p->len = data_len;
		p->next = NULL;
		while (*where != NULL) {
			where = &((*where)->next);
		}
		*where = p;
	}

	/* Another option would be BINARY data. */
	if (is_bin) {
		/* In this example, we just ignore binary data.
		 * According to some blogs, discriminating TEXT and
		 * BINARY may be some remains from earlier drafts
		 * of the WebSocket protocol.
		 * Anyway, a real application will usually use
		 * either TEXT or BINARY. */
	}

	/* It could be a CLOSE message as well. */
	if (is_close) {
		printf("%10.0f - Goodbye\n", difftime(now, pclient_data->started));
		return 0;
	}

	/* Return 1 to keep the connection open */
	return 1;
}


/* Callback for handling a close message received from the server */
static void
websocket_client_close_handler(const struct mg_connection *conn,
                               void *user_data)
{
	struct tclient_data *pclient_data = (struct tclient_data *)user_data;

	pclient_data->closed = time(NULL);
	printf("%10.0f - Client: Close handler\n",
	       difftime(pclient_data->closed, pclient_data->started));
}


/* Websocket client test function */
void
run_websocket_client(const char *host,
                     int port,
                     int secure,
                     const char *path,
                     const char *greetings)
{
	char err_buf[100] = {0};
	struct mg_connection *client_conn;
	struct tclient_data *pclient_data;
	int i;

	/* Allocate some memory for callback specific data.
	 * For simplicity, we ignore OOM handling in this example. */
	pclient_data = (struct tclient_data *)malloc(sizeof(struct tclient_data));

	/* Store start time in the private structure */
	pclient_data->started = time(NULL);
	pclient_data->closed = 0;
	pclient_data->msgs = NULL;

	/* Log first action (time = 0.0) */
	printf("%10.0f - Connecting to %s:%i\n", 0.0, host, port);

	/* Connect to the given WS or WSS (WS secure) server */
	client_conn = mg_connect_websocket_client(host,
	                                          port,
	                                          secure,
	                                          err_buf,
	                                          sizeof(err_buf),
	                                          path,
	                                          NULL,
	                                          websocket_client_data_handler,
	                                          websocket_client_close_handler,
	                                          pclient_data);

	/* Check if connection is possible */
	if (client_conn == NULL) {
		printf("mg_connect_websocket_client error: %s\n", err_buf);
		return;
	}

	/* Connection established */
	printf("%10.0f - Connected\n", difftime(time(NULL), pclient_data->started));

	/* If there are greetings to send, do it now */
	if (greetings) {
		printf("%10.0f - Sending greetings\n",
		       difftime(time(NULL), pclient_data->started));

		mg_websocket_client_write(client_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          greetings,
		                          strlen(greetings));
	}

	/* Wait for some seconds */
	sleep(5);

	/* Does the server play "ping pong" ? */
	for (i = 0; i < 5; i++) {
		/* Send a PING message every 5 seconds. */
		printf("%10.0f - Sending PING\n",
		       difftime(time(NULL), pclient_data->started));
		mg_websocket_client_write(client_conn,
		                          MG_WEBSOCKET_OPCODE_PING,
		                          (const char *)&i,
		                          sizeof(int));
		sleep(5);
	}

	/* Wait a while */
	/* If we do not use "ping pong", the server will probably
	 * close the connection with a timeout earlier. */
	sleep(150);

	/* Send greetings again */
	if (greetings) {
		printf("%10.0f - Sending greetings again\n",
		       difftime(time(NULL), pclient_data->started));

		mg_websocket_client_write(client_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          greetings,
		                          strlen(greetings));
	}

	/* Wait for some seconds */
	sleep(5);

	/* Send some "song text": http://www.99-bottles-of-beer.net/ */
	{
		char txt[128];
		int b = 99; /* start with 99 bottles */

		while (b > 0) {
			/* Send "b bottles" text line. */
			sprintf(txt,
			        "%i bottle%s of beer on the wall, "
			        "%i bottle%s of beer.",
			        b,
			        ((b != 1) ? "s" : ""),
			        b,
			        ((b != 1) ? "s" : ""));
			mg_websocket_client_write(client_conn,
			                          MG_WEBSOCKET_OPCODE_TEXT,
			                          txt,
			                          strlen(txt));

			/* Take a breath. */
			sleep(1);

			/* Drink a bottle */
			b--;

			/* Send "remaining bottles" text line. */
			if (b) {
				sprintf(txt,
				        "Take one down and pass it around, "
				        "%i bottle%s of beer on the wall.",
				        b,
				        ((b != 1) ? "s" : ""));
			} else {
				strcpy(txt,
				       "Take one down and pass it around, "
				       "no more bottles of beer on the wall.");
			}
			mg_websocket_client_write(client_conn,
			                          MG_WEBSOCKET_OPCODE_TEXT,
			                          txt,
			                          strlen(txt));

			/* Take a breath. */
			sleep(2);
		}

		/* Send "no more bottles" text line. */
		strcpy(txt,
		       "No more bottles of beer on the wall, "
		       "no more bottles of beer.");
		mg_websocket_client_write(client_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          txt,
		                          strlen(txt));

		/* Take a breath. */
		sleep(1);

		/* Buy new bottles. */
		b = 99;

		/* Send "buy some more" text line. */
		sprintf(txt,
		        "Go to the store and buy some more, "
		        "%i bottle%s of beer on the wall.",
		        b,
		        ((b != 1) ? "s" : ""));
		mg_websocket_client_write(client_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          txt,
		                          strlen(txt));
	}

	/* Wait for some seconds */
	sleep(5);

	/* Somewhat boring conversation, isn't it?
	 * Tell the server we have to leave. */
	printf("%10.0f - Sending close message\n",
	       difftime(time(NULL), pclient_data->started));
	mg_websocket_client_write(client_conn,
	                          MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE,
	                          NULL,
	                          0);

	/* We don't need to wait, this is just to have the log timestamp
	 * a second later, and to not log from the handlers and from
	 * here the same time (printf to stdout is not thread-safe, but
	 * adding flock or mutex or an explicit logging function makes
	 * this example unnecessarily complex). */
	sleep(5);

	/* Connection should be closed by now. */
	printf("%10.0f - Connection state: %s\n",
	       difftime(time(NULL), pclient_data->started),
	       ((pclient_data->closed == 0) ? "open" : "closed"));

	/* Close client connection */
	mg_close_connection(client_conn);
	printf("%10.0f - End of test\n",
	       difftime(time(NULL), pclient_data->started));


	/* Print collected data */
	printf("\n\nPrint all text messages from server again:\n");
	{
		struct tmsg_list_elem **where = &(pclient_data->msgs);
		while (*where != NULL) {
			printf("%10.0f - [%5lu] ",
			       difftime((*where)->timestamp, pclient_data->started),
			       (unsigned long)(*where)->len);
			fwrite((const char *)(*where)->data, 1, (*where)->len, stdout);
			printf("\n");

			where = &((*where)->next);
		}
	}

	/* Free collected data */
	{
		struct tmsg_list_elem **where = &(pclient_data->msgs);
		void *p1 = 0;
		void *p2 = 0;
		while (*where != NULL) {
			free((*where)->data);
			free(p2);
			p2 = p1;
			p1 = *where;

			where = &((*where)->next);
		}
		free(p2);
		free(p1);
	}
	free(pclient_data);
}


/* main will initialize the CivetWeb library
 * and start the WebSocket client test function */
int
main(int argc, char *argv[])
{
	const char *greetings = "Hello World!";

	const char *host = "echo.websocket.org";
	const char *path = "/";

#if defined(NO_SSL)
	const int port = 80;
	const int secure = 0;
	mg_init_library(0);
#else
	const int port = 443;
	const int secure = 1;
	mg_init_library(MG_FEATURES_SSL);
#endif

	run_websocket_client(host, port, secure, path, greetings);
}
