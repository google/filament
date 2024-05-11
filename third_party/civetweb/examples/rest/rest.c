/*
 * Copyright (c) 2018 the CivetWeb developers
 * MIT License
 */

/* Simple demo of a REST callback. */
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cJSON.h"
#include "civetweb.h"


#ifdef NO_SSL
#define PORT "8089"
#define HOST_INFO "http://localhost:8089"
#else
#define PORT "8089r,8843s"
#define HOST_INFO "https://localhost:8843"
#endif

#define EXAMPLE_URI "/example"
#define EXIT_URI "/exit"

int exitNow = 0;


static int
SendJSON(struct mg_connection *conn, cJSON *json_obj)
{
	char *json_str = cJSON_PrintUnformatted(json_obj);
	size_t json_str_len = strlen(json_str);

	/* Send HTTP message header */
	mg_send_http_ok(conn, "application/json; charset=utf-8", json_str_len);

	/* Send HTTP message content */
	mg_write(conn, json_str, json_str_len);

	/* Free string allocated by cJSON_Print* */
	cJSON_free(json_str);

	return (int)json_str_len;
}


static unsigned request = 0; /* demo data: request counter */


static int
ExampleGET(struct mg_connection *conn)
{
	cJSON *obj = cJSON_CreateObject();

	if (!obj) {
		/* insufficient memory? */
		mg_send_http_error(conn, 500, "Server error");
		return 500;
	}


	cJSON_AddStringToObject(obj, "version", CIVETWEB_VERSION);
	cJSON_AddNumberToObject(obj, "request", ++request);
	SendJSON(conn, obj);
	cJSON_Delete(obj);

	return 200;
}


static int
ExampleDELETE(struct mg_connection *conn)
{
	request = 0;
	mg_send_http_error(conn,
	                   204,
	                   "%s",
	                   ""); /* Return "deleted" = "204 No Content" */

	return 204;
}


static int
ExamplePUT(struct mg_connection *conn)
{
	char buffer[1024];
	int dlen = mg_read(conn, buffer, sizeof(buffer) - 1);
	cJSON *obj, *elem;
	unsigned newvalue;

	if ((dlen < 1) || (dlen >= sizeof(buffer))) {
		mg_send_http_error(conn, 400, "%s", "No request body data");
		return 400;
	}
	buffer[dlen] = 0;

	obj = cJSON_Parse(buffer);
	if (obj == NULL) {
		mg_send_http_error(conn, 400, "%s", "Invalid request body data");
		return 400;
	}

	elem = cJSON_GetObjectItemCaseSensitive(obj, "request");

	if (!cJSON_IsNumber(elem)) {
		cJSON_Delete(obj);
		mg_send_http_error(conn,
		                   400,
		                   "%s",
		                   "No \"request\" number in body data");
		return 400;
	}

	newvalue = (unsigned)elem->valuedouble;

	if ((double)newvalue != elem->valuedouble) {
		cJSON_Delete(obj);
		mg_send_http_error(conn,
		                   400,
		                   "%s",
		                   "Invalid \"request\" number in body data");
		return 400;
	}

	request = newvalue;
	cJSON_Delete(obj);

	mg_send_http_error(conn, 201, "%s", ""); /* Return "201 Created" */

	return 201;
}


static int
ExamplePOST(struct mg_connection *conn)
{
	/* In this example, do the same for PUT and POST */
	return ExamplePUT(conn);
}


static int
ExamplePATCH(struct mg_connection *conn)
{
	/* In this example, do the same for PUT and PATCH */
	return ExamplePUT(conn);
}


static int
ExampleHandler(struct mg_connection *conn, void *cbdata)
{

	const struct mg_request_info *ri = mg_get_request_info(conn);
	(void)cbdata; /* currently unused */

	if (0 == strcmp(ri->request_method, "GET")) {
		return ExampleGET(conn);
	}
	if (0 == strcmp(ri->request_method, "PUT")) {
		return ExamplePUT(conn);
	}
	if (0 == strcmp(ri->request_method, "POST")) {
		return ExamplePOST(conn);
	}
	if (0 == strcmp(ri->request_method, "DELETE")) {
		return ExampleDELETE(conn);
	}
	if (0 == strcmp(ri->request_method, "PATCH")) {
		return ExamplePATCH(conn);
	}

	/* this is not a GET request */
	mg_send_http_error(
	    conn, 405, "Only GET, PUT, POST, DELETE and PATCH method supported");
	return 405;
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
log_message(const struct mg_connection *conn, const char *message)
{
	puts(message);
	return 1;
}


int
main(int argc, char *argv[])
{
	const char *options[] = {"listening_ports",
	                         PORT,
	                         "request_timeout_ms",
	                         "10000",
	                         "error_log_file",
	                         "error.log",
#ifndef NO_SSL
	                         "ssl_certificate",
	                         "../../resources/cert/server.pem",
	                         "ssl_protocol_version",
	                         "3",
	                         "ssl_cipher_list",
	                         "DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256",
#endif
	                         "enable_auth_domain_check",
	                         "no",
	                         0};

	struct mg_callbacks callbacks;
	struct mg_context *ctx;
	int err = 0;

/* Check if libcivetweb has been built with all required features. */
#ifndef NO_SSL
	if (!mg_check_feature(2)) {
		fprintf(stderr,
		        "Error: Embedded example built with SSL support, "
		        "but civetweb library build without.\n");
		err = 1;
	}


	mg_init_library(MG_FEATURES_SSL);

#else
	mg_init_library(0);

#endif
	if (err) {
		fprintf(stderr, "Cannot start CivetWeb - inconsistent build.\n");
		return EXIT_FAILURE;
	}


	/* Callback will print error messages to console */
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.log_message = log_message;

	/* Start CivetWeb web server */
	ctx = mg_start(&callbacks, 0, options);

	/* Check return value: */
	if (ctx == NULL) {
		fprintf(stderr, "Cannot start CivetWeb - mg_start failed.\n");
		return EXIT_FAILURE;
	}

	/* Add handler EXAMPLE_URI, to explain the example */
	mg_set_request_handler(ctx, EXAMPLE_URI, ExampleHandler, 0);
	mg_set_request_handler(ctx, EXIT_URI, ExitHandler, 0);

	/* Show sone info */
	printf("Start example: %s%s\n", HOST_INFO, EXAMPLE_URI);
	printf("Exit example:  %s%s\n", HOST_INFO, EXIT_URI);


	/* Wait until the server should be closed */
	while (!exitNow) {
#ifdef _WIN32
		Sleep(1000);
#else
		sleep(1);
#endif
	}

	/* Stop the server */
	mg_stop(ctx);

	printf("Server stopped.\n");
	printf("Bye!\n");

	return EXIT_SUCCESS;
}
