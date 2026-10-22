/* Copyright (c) 2015-2018 the Civetweb developers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "public_server.h"
#include <civetweb.h>

#if defined(_WIN32)
#include <windows.h>
#define test_sleep(x) (Sleep((x)*1000))
#else
#include <unistd.h>
#define test_sleep(x) (sleep(x))
#endif

#define SLEEP_BEFORE_MG_START (1)
#define SLEEP_AFTER_MG_START (3)
#define SLEEP_BEFORE_MG_STOP (1)
#define SLEEP_AFTER_MG_STOP (5)

/* This unit test file uses the excellent Check unit testing library.
 * The API documentation is available here:
 * http://check.sourceforge.net/doc/check_html/index.html
 */

#if defined(__MINGW32__) || defined(__GNUC__)
/* Return codes of the tested functions are evaluated. Checking all other
 * functions, used only to prepare the test environment seems redundant.
 * If they fail, the test fails anyway. */
#pragma GCC diagnostic ignored "-Wunused-result"
#endif

static const char *
locate_path(const char *a_path)
{
	static char r_path[256];

#ifdef _WIN32
#ifdef LOCAL_TEST
	sprintf(r_path, "%s\\", a_path);
#else
	/* Appveyor */
	sprintf(r_path, "..\\..\\..\\%s\\", a_path);
/* TODO: the different paths
 * used in the different test
 * system is an unsolved
 * problem. */
#endif
#else
#ifdef LOCAL_TEST
	sprintf(r_path, "%s/", a_path);
#else
	/* Travis */
	sprintf(r_path,
	        "../../%s/",
	        a_path); // TODO: fix path in CI test environment
#endif
#endif

	return r_path;
}


#define locate_resources() locate_path("resources")
#define locate_test_exes() locate_path("output")


static const char *
locate_ssl_cert(void)
{
	static char cert_path[256];
	const char *res = locate_resources();
	size_t l;

	ck_assert(res != NULL);
	l = strlen(res);
	ck_assert_uint_gt(l, 0);
	ck_assert_uint_lt(l, 100); /* assume there is enough space left in our
	                              typical 255 character string buffers */

	strcpy(cert_path, res);
	strcat(cert_path, "ssl_cert.pem");
	return cert_path;
}


static int
wait_not_null(void *volatile *data)
{
	int i;
	for (i = 0; i < 100; i++) {
		mark_point();
		test_sleep(1);

		if (*data != NULL) {
			mark_point();
			return 1;
		}
	}

#if defined(__MINGW32__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunreachable-code"
#pragma GCC diagnostic ignored "-Wunreachable-code-return"
#endif

	ck_abort_msg("wait_not_null failed (%i sec)", i);

	return 0;

#if defined(__MINGW32__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
}


START_TEST(test_the_test_environment)
{
	char wd[300];
	char buf[500];
	FILE *f;
	struct stat st;
	int ret;
	const char *ssl_cert = locate_ssl_cert();

	memset(wd, 0, sizeof(wd));
	memset(buf, 0, sizeof(buf));

/* Get the current working directory */
#ifdef _WIN32
	(void)GetCurrentDirectoryA(sizeof(wd), wd);
	wd[sizeof(wd) - 1] = 0;
#else
	(void)getcwd(wd, sizeof(wd));
	wd[sizeof(wd) - 1] = 0;
#endif

/* Check the pem file */
#ifdef _WIN32
	strcpy(buf, wd);
	strcat(buf, "\\");
	strcat(buf, ssl_cert);
	f = fopen(buf, "rb");
#else
	strcpy(buf, wd);
	strcat(buf, "/");
	strcat(buf, ssl_cert);
	f = fopen(buf, "r");
#endif

	if (f) {
		fclose(f);
	} else {
		fprintf(stderr, "Certificate %s not found\n", buf);
		exit(1); /* some path is not correct --> test will not work */
	}


#ifdef _WIN32
/* Try to copy the files required for AppVeyor */
#if defined(_WIN64) || defined(__MINGW64__)
	(void)system("cmd /c copy C:\\OpenSSL-Win64\\libeay32.dll libeay32.dll");
	(void)system("cmd /c copy C:\\OpenSSL-Win64\\libssl32.dll libssl32.dll");
	(void)system("cmd /c copy C:\\OpenSSL-Win64\\ssleay32.dll ssleay32.dll");
	(void)system("cmd /c copy C:\\OpenSSL-Win64\\libeay32.dll libeay64.dll");
	(void)system("cmd /c copy C:\\OpenSSL-Win64\\libssl32.dll libssl64.dll");
	(void)system("cmd /c copy C:\\OpenSSL-Win64\\ssleay32.dll ssleay64.dll");
#else
	(void)system("cmd /c copy C:\\OpenSSL-Win32\\libeay32.dll libeay32.dll");
	(void)system("cmd /c copy C:\\OpenSSL-Win32\\libssl32.dll libssl32.dll");
	(void)system("cmd /c copy C:\\OpenSSL-Win32\\ssleay32.dll ssleay32.dll");
#endif
#endif
}
END_TEST


static void *threading_data = 0;

static void *
test_thread_func_t(void *param)
{
	ck_assert_ptr_eq(param, &threading_data);
	ck_assert_ptr_eq(threading_data, NULL);
	threading_data = &threading_data;
	return NULL;
}


START_TEST(test_threading)
{
	int ok;

	threading_data = NULL;
	mark_point();

	ok = mg_start_thread(test_thread_func_t, &threading_data);
	ck_assert_int_eq(ok, 0);

	wait_not_null(&threading_data);
	ck_assert_ptr_eq(threading_data, &threading_data);
}
END_TEST


static int
log_msg_func(const struct mg_connection *conn, const char *message)
{
	struct mg_context *ctx;
	char *ud;

	ck_assert(conn != NULL);
	ctx = mg_get_context(conn);
	ck_assert(ctx != NULL);
	ud = (char *)mg_get_user_data(ctx);

	strncpy(ud, message, 255);
	ud[255] = 0;
	mark_point();

	printf("LOG_MSG_FUNC: %s\n", message);
	mark_point();

	return 1; /* Return 1 means "already handled" */
}


static int
test_log_message(const struct mg_connection *conn, const char *message)
{
	(void)conn;

	printf("LOG_MESSAGE: %s\n", message);
	mark_point();

	return 0; /* Return 0 means "not yet handled" */
}


static struct mg_context *
test_mg_start(const struct mg_callbacks *callbacks,
              void *user_data,
              const char **configuration_options)
{
	struct mg_context *ctx;
	struct mg_callbacks cb;

	if (callbacks) {
		memcpy(&cb, callbacks, sizeof(cb));
	} else {
		memset(&cb, 0, sizeof(cb));
	}

	if (cb.log_message == NULL) {
		cb.log_message = test_log_message;
	}

	mark_point();
	test_sleep(SLEEP_BEFORE_MG_START);
	mark_point();
	ctx = mg_start(&cb, user_data, configuration_options);
	mark_point();
	if (ctx) {
		/* Give the server some time to start in the test VM */
		/* Don't need to do this if mg_start failed */
		test_sleep(SLEEP_AFTER_MG_START);
	}
	mark_point();

	return ctx;
}


static void
test_mg_stop(struct mg_context *ctx)
{
#ifdef __MACH__
	/* For unknown reasons, there are sporadic hands
	 * for OSX if mark_point is called here */
	test_sleep(SLEEP_BEFORE_MG_STOP);
	mg_stop(ctx);
	test_sleep(SLEEP_AFTER_MG_STOP);
#else
	mark_point();
	test_sleep(SLEEP_BEFORE_MG_STOP);
	mark_point();
	mg_stop(ctx);
	mark_point();
	test_sleep(SLEEP_AFTER_MG_STOP);
	mark_point();
#endif
}


static void
test_mg_start_stop_http_server_impl(int ipv6, int bound)
{
	struct mg_context *ctx;
	const char *OPTIONS[16];
	int optcnt = 0;
	const char *localhost_name = ((ipv6) ? "[::1]" : "127.0.0.1");

#if defined(MG_LEGACY_INTERFACE)
	size_t ports_cnt;
	int ports[16];
	int ssl[16];
#endif
	struct mg_callbacks callbacks;
	char errmsg[256];

	struct mg_connection *client_conn;
	char client_err[256];
	const struct mg_response_info *client_ri;
	int client_res, ret;
	struct mg_server_port portinfo[8];

	mark_point();

#if !defined(NO_FILES)
	OPTIONS[optcnt++] = "document_root";
	OPTIONS[optcnt++] = ".";
#endif
	OPTIONS[optcnt++] = "listening_ports";
	if (bound) {
		OPTIONS[optcnt++] = ((ipv6) ? "[::1]:+8080" : "127.0.0.1:8080");
	} else {
		OPTIONS[optcnt++] = ((ipv6) ? "+8080" : "8080");
		/* Test also tcp_nodelay - this option is not related
		 * to interface binding, it's just tested here in this
		 * combination to keep the number of tests smaller and
		 * the test duration shorter.
		 */
		OPTIONS[optcnt++] = "tcp_nodelay";
		OPTIONS[optcnt++] = "1";
	}

	OPTIONS[optcnt] = 0;

#if defined(MG_LEGACY_INTERFACE)
	memset(ports, 0, sizeof(ports));
	memset(ssl, 0, sizeof(ssl));
#endif
	memset(portinfo, 0, sizeof(portinfo));
	memset(&callbacks, 0, sizeof(callbacks));
	memset(errmsg, 0, sizeof(errmsg));

	callbacks.log_message = log_msg_func;

	ctx = test_mg_start(&callbacks, (void *)errmsg, OPTIONS);

	ck_assert_str_eq(errmsg, "");
	ck_assert(ctx != NULL);

#if defined(MG_LEGACY_INTERFACE)
	ports_cnt = mg_get_ports(ctx, 16, ports, ssl);
	ck_assert_uint_eq(ports_cnt, 1);
	ck_assert_int_eq(ports[0], 8080);
	ck_assert_int_eq(ssl[0], 0);
	ck_assert_int_eq(ports[1], 0);
	ck_assert_int_eq(ssl[1], 0);
#endif

	ret = mg_get_server_ports(ctx, 0, portinfo);
	ck_assert_int_lt(ret, 0);
	ck_assert_int_eq(portinfo[0].protocol, 0);
	ck_assert_int_eq(portinfo[0].port, 0);
	ck_assert_int_eq(portinfo[0].is_ssl, 0);
	ck_assert_int_eq(portinfo[0].is_redirect, 0);
	ck_assert_int_eq(portinfo[1].protocol, 0);
	ck_assert_int_eq(portinfo[1].port, 0);
	ck_assert_int_eq(portinfo[1].is_ssl, 0);
	ck_assert_int_eq(portinfo[1].is_redirect, 0);

	ret = mg_get_server_ports(ctx, 4, portinfo);
	ck_assert_int_eq(ret, 1);
	if (ipv6) {
		ck_assert_int_eq(portinfo[0].protocol, 3);
	} else {
		ck_assert_int_eq(portinfo[0].protocol, 1);
	}
	ck_assert_int_eq(portinfo[0].port, 8080);
	ck_assert_int_eq(portinfo[0].is_ssl, 0);
	ck_assert_int_eq(portinfo[0].is_redirect, 0);
	ck_assert_int_eq(portinfo[1].protocol, 0);
	ck_assert_int_eq(portinfo[1].port, 0);
	ck_assert_int_eq(portinfo[1].is_ssl, 0);
	ck_assert_int_eq(portinfo[1].is_redirect, 0);

	test_sleep(1);

	/* HTTP 1.0 GET request */
	memset(client_err, 0, sizeof(client_err));
	client_conn = mg_connect_client(
	    localhost_name, 8080, 0, client_err, sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "GET / HTTP/1.0\r\n\r\n");
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

#if defined(NO_FILES)
	ck_assert_int_eq(client_ri->status_code, 404);
#else
	ck_assert_int_eq(client_ri->status_code, 200);
	/* TODO: ck_assert_str_eq(client_ri->request_method, "HTTP/1.0"); */
	client_res = (int)mg_read(client_conn, client_err, sizeof(client_err));
	ck_assert_int_gt(client_res, 0);
	ck_assert_int_le(client_res, sizeof(client_err));
#endif
	mg_close_connection(client_conn);

	test_sleep(1);

	/* HTTP 1.1 GET request */
	memset(client_err, 0, sizeof(client_err));
	client_conn = mg_connect_client(
	    localhost_name, 8080, 0, client_err, sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "GET / HTTP/1.1\r\n");
	mg_printf(client_conn, "Host: localhost:8080\r\n");
	mg_printf(client_conn, "Connection: close\r\n\r\n");
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

#if defined(NO_FILES)
	ck_assert_int_eq(client_ri->status_code, 404);
#else
	ck_assert_int_eq(client_ri->status_code, 200);
	/* TODO: ck_assert_str_eq(client_ri->request_method, "HTTP/1.0"); */
	client_res = (int)mg_read(client_conn, client_err, sizeof(client_err));
	ck_assert_int_gt(client_res, 0);
	ck_assert_int_le(client_res, sizeof(client_err));
#endif
	mg_close_connection(client_conn);

	test_sleep(1);


	/* HTTP 1.7 GET request - this HTTP version does not exist  */
	memset(client_err, 0, sizeof(client_err));
	client_conn = mg_connect_client(
	    localhost_name, 8080, 0, client_err, sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "GET / HTTP/1.7\r\n");
	mg_printf(client_conn, "Host: localhost:8080\r\n");
	mg_printf(client_conn, "Connection: close\r\n\r\n");
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

	/* Response must be 505 HTTP Version not supported */
	ck_assert_int_eq(client_ri->status_code, 505);
	mg_close_connection(client_conn);

	test_sleep(1);


	/* HTTP request with multiline header.
	 * Multiline header are obsolete with RFC 7230 section-3.2.4
	 * and must return "400 Bad Request" */
	memset(client_err, 0, sizeof(client_err));
	client_conn = mg_connect_client(
	    localhost_name, 8080, 0, client_err, sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "GET / HTTP/1.1\r\n");
	mg_printf(client_conn, "Host: localhost:8080\r\n");
	mg_printf(client_conn, "X-Obsolete-Header: something\r\nfor nothing\r\n");
	mg_printf(client_conn, "Connection: close\r\n\r\n");
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

	/* Response must be 400 Bad Request */
	ck_assert_int_eq(client_ri->status_code, 400);
	mg_close_connection(client_conn);

	test_sleep(1);

	/* End test */
	test_mg_stop(ctx);
	mark_point();
}


START_TEST(test_mg_start_stop_http_server)
{
	mark_point();
	test_mg_start_stop_http_server_impl(0, 0);
	mark_point();
	test_mg_start_stop_http_server_impl(0, 1);
	mark_point();
}
END_TEST


START_TEST(test_mg_start_stop_http_server_ipv6)
{
	mark_point();
#if defined(USE_IPV6)
	test_mg_start_stop_http_server_impl(1, 0);
	mark_point();
	test_mg_start_stop_http_server_impl(1, 1);
#endif
	mark_point();
}
END_TEST


START_TEST(test_mg_start_stop_https_server)
{
#ifndef NO_SSL

	struct mg_context *ctx;

#if defined(MG_LEGACY_INTERFACE)
	size_t ports_cnt;
	int ports[16];
	int ssl[16];
#endif
	struct mg_callbacks callbacks;
	char errmsg[256];

	const char *OPTIONS[8]; /* initializer list here is rejected by CI test */
	int opt_idx = 0;
	const char *ssl_cert = locate_ssl_cert();

	struct mg_connection *client_conn;
	char client_err[256];
	const struct mg_response_info *client_ri;
	int client_res, ret;
	struct mg_server_port portinfo[8];

	ck_assert(ssl_cert != NULL);

	memset((void *)OPTIONS, 0, sizeof(OPTIONS));
#if !defined(NO_FILES)
	OPTIONS[opt_idx++] = "document_root";
	OPTIONS[opt_idx++] = ".";
#endif
	OPTIONS[opt_idx++] = "listening_ports";
	OPTIONS[opt_idx++] = "8080r,8443s";
	OPTIONS[opt_idx++] = "ssl_certificate";
	OPTIONS[opt_idx++] = ssl_cert;

	ck_assert_int_le(opt_idx, (int)(sizeof(OPTIONS) / sizeof(OPTIONS[0])));
	ck_assert(OPTIONS[sizeof(OPTIONS) / sizeof(OPTIONS[0]) - 1] == NULL);
	ck_assert(OPTIONS[sizeof(OPTIONS) / sizeof(OPTIONS[0]) - 2] == NULL);

#if defined(MG_LEGACY_INTERFACE)
	memset(ports, 0, sizeof(ports));
	memset(ssl, 0, sizeof(ssl));
#endif
	memset(portinfo, 0, sizeof(portinfo));
	memset(&callbacks, 0, sizeof(callbacks));
	memset(errmsg, 0, sizeof(errmsg));

	callbacks.log_message = log_msg_func;

	ctx = test_mg_start(&callbacks, (void *)errmsg, OPTIONS);

	ck_assert_str_eq(errmsg, "");
	ck_assert(ctx != NULL);

#if defined(MG_LEGACY_INTERFACE)
	ports_cnt = mg_get_ports(ctx, 16, ports, ssl);
	ck_assert_uint_eq(ports_cnt, 2);
	ck_assert_int_eq(ports[0], 8080);
	ck_assert_int_eq(ssl[0], 0);
	ck_assert_int_eq(ports[1], 8443);
	ck_assert_int_eq(ssl[1], 1);
	ck_assert_int_eq(ports[2], 0);
	ck_assert_int_eq(ssl[2], 0);
#endif

	ret = mg_get_server_ports(ctx, 0, portinfo);
	ck_assert_int_lt(ret, 0);
	ck_assert_int_eq(portinfo[0].protocol, 0);
	ck_assert_int_eq(portinfo[0].port, 0);
	ck_assert_int_eq(portinfo[0].is_ssl, 0);
	ck_assert_int_eq(portinfo[0].is_redirect, 0);
	ck_assert_int_eq(portinfo[1].protocol, 0);
	ck_assert_int_eq(portinfo[1].port, 0);
	ck_assert_int_eq(portinfo[1].is_ssl, 0);
	ck_assert_int_eq(portinfo[1].is_redirect, 0);

	ret = mg_get_server_ports(ctx, 4, portinfo);
	ck_assert_int_eq(ret, 2);
	ck_assert_int_eq(portinfo[0].protocol, 1);
	ck_assert_int_eq(portinfo[0].port, 8080);
	ck_assert_int_eq(portinfo[0].is_ssl, 0);
	ck_assert_int_eq(portinfo[0].is_redirect, 1);
	ck_assert_int_eq(portinfo[1].protocol, 1);
	ck_assert_int_eq(portinfo[1].port, 8443);
	ck_assert_int_eq(portinfo[1].is_ssl, 1);
	ck_assert_int_eq(portinfo[1].is_redirect, 0);
	ck_assert_int_eq(portinfo[2].protocol, 0);
	ck_assert_int_eq(portinfo[2].port, 0);
	ck_assert_int_eq(portinfo[2].is_ssl, 0);
	ck_assert_int_eq(portinfo[2].is_redirect, 0);

	test_sleep(1);

	memset(client_err, 0, sizeof(client_err));
	client_conn =
	    mg_connect_client("127.0.0.1", 8443, 1, client_err, sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "GET / HTTP/1.0\r\n\r\n");
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

#if defined(NO_FILES)
	ck_assert_int_eq(client_ri->status_code, 404);
#else
	ck_assert_int_eq(client_ri->status_code, 200);
	/* TODO: ck_assert_str_eq(client_ri->request_method, "HTTP/1.0"); */
	client_res = (int)mg_read(client_conn, client_err, sizeof(client_err));
	ck_assert_int_gt(client_res, 0);
	ck_assert_int_le(client_res, sizeof(client_err));
#endif
	mg_close_connection(client_conn);

	test_sleep(1);

	test_mg_stop(ctx);
	mark_point();
#endif
}
END_TEST


START_TEST(test_mg_server_and_client_tls)
{
#ifndef NO_SSL

	struct mg_context *ctx;

	int ports_cnt;
	struct mg_server_port ports[16];
	struct mg_callbacks callbacks;
	char errmsg[256];

	struct mg_connection *client_conn;
	char client_err[256];
	const struct mg_response_info *client_ri;
	int client_res;
	struct mg_client_options client_options;

	const char *OPTIONS[32]; /* initializer list here is rejected by CI test */
	int opt_idx = 0;
	char server_cert[256];
	char client_cert[256];
	const char *res_dir = locate_resources();

	ck_assert(res_dir != NULL);
	strcpy(server_cert, res_dir);
	strcpy(client_cert, res_dir);
#ifdef _WIN32
	strcat(server_cert, "cert\\server.pem");
	strcat(client_cert, "cert\\client.pem");
#else
	strcat(server_cert, "cert/server.pem");
	strcat(client_cert, "cert/client.pem");
#endif

	memset((void *)OPTIONS, 0, sizeof(OPTIONS));
#if !defined(NO_FILES)
	OPTIONS[opt_idx++] = "document_root";
	OPTIONS[opt_idx++] = ".";
#endif
	OPTIONS[opt_idx++] = "listening_ports";
	OPTIONS[opt_idx++] = "8080r,8443s";
	OPTIONS[opt_idx++] = "ssl_certificate";
	OPTIONS[opt_idx++] = server_cert;
	OPTIONS[opt_idx++] = "ssl_verify_peer";
	OPTIONS[opt_idx++] = "yes";
	OPTIONS[opt_idx++] = "ssl_ca_file";
	OPTIONS[opt_idx++] = client_cert;

	ck_assert_int_le(opt_idx, (int)(sizeof(OPTIONS) / sizeof(OPTIONS[0])));
	ck_assert(OPTIONS[sizeof(OPTIONS) / sizeof(OPTIONS[0]) - 1] == NULL);
	ck_assert(OPTIONS[sizeof(OPTIONS) / sizeof(OPTIONS[0]) - 2] == NULL);

	memset(ports, 0, sizeof(ports));
	memset(&callbacks, 0, sizeof(callbacks));
	memset(errmsg, 0, sizeof(errmsg));

	callbacks.log_message = log_msg_func;

	ctx = test_mg_start(&callbacks, (void *)errmsg, OPTIONS);

	ck_assert_str_eq(errmsg, "");
	ck_assert(ctx != NULL);

	ports_cnt = mg_get_server_ports(ctx, 16, ports);
	ck_assert_int_eq(ports_cnt, 2);
	ck_assert_int_eq(ports[0].protocol, 1);
	ck_assert_int_eq(ports[0].port, 8080);
	ck_assert_int_eq(ports[0].is_ssl, 0);
	ck_assert_int_eq(ports[0].is_redirect, 1);
	ck_assert_int_eq(ports[1].protocol, 1);
	ck_assert_int_eq(ports[1].port, 8443);
	ck_assert_int_eq(ports[1].is_ssl, 1);
	ck_assert_int_eq(ports[1].is_redirect, 0);
	ck_assert_int_eq(ports[2].protocol, 0);
	ck_assert_int_eq(ports[2].port, 0);
	ck_assert_int_eq(ports[2].is_ssl, 0);
	ck_assert_int_eq(ports[2].is_redirect, 0);

	test_sleep(1);

	memset(client_err, 0, sizeof(client_err));
	client_conn =
	    mg_connect_client("127.0.0.1", 8443, 1, client_err, sizeof(client_err));

	ck_assert_str_ne(client_err, "");
	ck_assert(client_conn == NULL);

	memset(client_err, 0, sizeof(client_err));
	memset(&client_options, 0, sizeof(client_options));
	client_options.host = "127.0.0.1";
	client_options.port = 8443;
	client_options.client_cert = client_cert;
	client_options.server_cert = server_cert;

	client_conn = mg_connect_client_secure(&client_options,
	                                       client_err,
	                                       sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "GET / HTTP/1.0\r\n\r\n");
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

#if defined(NO_FILES)
	ck_assert_int_eq(client_ri->status_code, 404);
#else
	ck_assert_int_eq(client_ri->status_code, 200);
	/* TODO: ck_assert_str_eq(client_ri->request_method, "HTTP/1.0"); */
	client_res = (int)mg_read(client_conn, client_err, sizeof(client_err));
	ck_assert_int_gt(client_res, 0);
	ck_assert_int_le(client_res, sizeof(client_err));
#endif
	mg_close_connection(client_conn);

	/* TODO: A client API using a client certificate is missing */

	test_sleep(1);

	test_mg_stop(ctx);
#endif
	mark_point();
}
END_TEST


static struct mg_context *g_ctx;

static int
request_test_handler(struct mg_connection *conn, void *cbdata)
{
	int i;
	char chunk_data[32];
	const struct mg_request_info *ri;
	struct mg_context *ctx;
	void *ud, *cud;
	void *dummy = malloc(1);

	ctx = mg_get_context(conn);
	ud = mg_get_user_data(ctx);
	ri = mg_get_request_info(conn);

	ck_assert(ri != NULL);
	ck_assert(ctx == g_ctx);
	ck_assert(ud == &g_ctx);

	ck_assert(dummy != NULL);

	mg_set_user_connection_data(conn, (void *)&dummy);
	cud = mg_get_user_connection_data(conn);
	ck_assert_ptr_eq((void *)cud, (void *)&dummy);

	mg_set_user_connection_data(conn, (void *)NULL);
	cud = mg_get_user_connection_data(conn);
	ck_assert_ptr_eq((void *)cud, (void *)NULL);

	free(dummy);

	ck_assert_ptr_eq((void *)cbdata, (void *)(ptrdiff_t)7);
	strcpy(chunk_data, "123456789A123456789B123456789C");

	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\n"
	          "Transfer-Encoding: chunked\r\n"
	          "Content-Type: text/plain\r\n\r\n");

	for (i = 1; i <= 10; i++) {
		mg_printf(conn, "%x\r\n", i);
		mg_write(conn, chunk_data, (unsigned)i);
		mg_printf(conn, "\r\n");
	}

	mg_printf(conn, "0\r\n\r\n");
	mark_point();

	return 1;
}


/* Return the same as request_test_handler using new interfaces */
static int
request_test_handler2(struct mg_connection *conn, void *cbdata)
{
	int i;
	const char *chunk_data = "123456789A123456789B123456789C";
	const struct mg_request_info *ri;
	struct mg_context *ctx;
	void *ud;

	ctx = mg_get_context(conn);
	ud = mg_get_user_data(ctx);
	ri = mg_get_request_info(conn);

	ck_assert(ri != NULL);
	ck_assert(ctx == g_ctx);
	ck_assert(ud == &g_ctx);

	mg_send_http_ok(conn, "text/plain", -1);

	for (i = 1; i <= 10; i++) {
		mg_send_chunk(conn, chunk_data, (unsigned)i);
	}

	mg_send_chunk(conn, 0, 0);
	mark_point();

	return 200;
}


#ifdef USE_WEBSOCKET
/****************************************************************************/
/* WEBSOCKET SERVER                                                         */
/****************************************************************************/
static const char *websocket_welcome_msg = "websocket welcome\n";
static const size_t websocket_welcome_msg_len =
    18 /* strlen(websocket_welcome_msg) */;
static const char *websocket_goodbye_msg = "websocket bye\n";
static const size_t websocket_goodbye_msg_len =
    14 /* strlen(websocket_goodbye_msg) */;


#if defined(DEBUG)
static void
WS_TEST_TRACE(const char *f, ...)
{
	va_list l;
	va_start(l, f);
	vprintf(f, l);
	va_end(l);
}
#else
#define WS_TEST_TRACE(...)
#endif


static int
websock_server_connect(const struct mg_connection *conn, void *udata)
{
	(void)conn;

	ck_assert_ptr_eq((void *)udata, (void *)(ptrdiff_t)7531);
	WS_TEST_TRACE("Server: Websocket connected\n");
	mark_point();

	return 0; /* return 0 to accept every connection */
}


static void
websock_server_ready(struct mg_connection *conn, void *udata)
{
	ck_assert_ptr_eq((void *)udata, (void *)(ptrdiff_t)7531);
	ck_assert_ptr_ne((void *)conn, (void *)NULL);
	WS_TEST_TRACE("Server: Websocket ready\n");

	/* Send websocket welcome message */
	mg_lock_connection(conn);
	mg_websocket_write(conn,
	                   MG_WEBSOCKET_OPCODE_TEXT,
	                   websocket_welcome_msg,
	                   websocket_welcome_msg_len);
	mg_unlock_connection(conn);

	WS_TEST_TRACE("Server: Websocket ready X\n");
	mark_point();
}


#define long_ws_buf_len_16 (500)
#define long_ws_buf_len_64 (70000)
static char long_ws_buf[long_ws_buf_len_64];


static int
websock_server_data(struct mg_connection *conn,
                    int bits,
                    char *data,
                    size_t data_len,
                    void *udata)
{
	(void)bits;

	ck_assert_ptr_eq((void *)udata, (void *)(ptrdiff_t)7531);
	WS_TEST_TRACE("Server: Got %u bytes from the client\n", (unsigned)data_len);

	if (data_len == 3 && !memcmp(data, "bye", 3)) {
		/* Send websocket goodbye message */
		mg_lock_connection(conn);
		mg_websocket_write(conn,
		                   MG_WEBSOCKET_OPCODE_TEXT,
		                   websocket_goodbye_msg,
		                   websocket_goodbye_msg_len);
		mg_unlock_connection(conn);
	} else if (data_len == 5 && !memcmp(data, "data1", 5)) {
		mg_lock_connection(conn);
		mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, "ok1", 3);
		mg_unlock_connection(conn);
	} else if (data_len == 5 && !memcmp(data, "data2", 5)) {
		mg_lock_connection(conn);
		mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, "ok 2", 4);
		mg_unlock_connection(conn);
	} else if (data_len == 5 && !memcmp(data, "data3", 5)) {
		mg_lock_connection(conn);
		mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, "ok - 3", 6);
		mg_unlock_connection(conn);
	} else if (data_len == long_ws_buf_len_16) {
		ck_assert(memcmp(data, long_ws_buf, long_ws_buf_len_16) == 0);
		mg_lock_connection(conn);
		mg_websocket_write(conn,
		                   MG_WEBSOCKET_OPCODE_BINARY,
		                   long_ws_buf,
		                   long_ws_buf_len_16);
		mg_unlock_connection(conn);
	} else if (data_len == long_ws_buf_len_64) {
		ck_assert(memcmp(data, long_ws_buf, long_ws_buf_len_64) == 0);
		mg_lock_connection(conn);
		mg_websocket_write(conn,
		                   MG_WEBSOCKET_OPCODE_BINARY,
		                   long_ws_buf,
		                   long_ws_buf_len_64);
		mg_unlock_connection(conn);
	} else {

#if defined(__MINGW32__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunreachable-code"
#endif
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code"
#endif

		ck_abort_msg("Got unexpected message from websocket client");


		return 0;

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#if defined(__MINGW32__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
	}
	mark_point();

	return 1; /* return 1 to keep the connetion open */
}


static void
websock_server_close(const struct mg_connection *conn, void *udata)
{
#ifndef __MACH__
	ck_assert_ptr_eq((void *)udata, (void *)(ptrdiff_t)7531);
	WS_TEST_TRACE("Server: Close connection\n");

	/* Can not send a websocket goodbye message here -
	 * the connection is already closed */

	mark_point();
#endif

	(void)conn;
	(void)udata;
}


/****************************************************************************/
/* WEBSOCKET CLIENT                                                         */
/****************************************************************************/
struct tclient_data {
	void *data;
	size_t len;
	int closed;
	int clientId;
};


static int
websocket_client_data_handler(struct mg_connection *conn,
                              int flags,
                              char *data,
                              size_t data_len,
                              void *user_data)
{
	struct mg_context *ctx = mg_get_context(conn);
	struct tclient_data *pclient_data =
	    (struct tclient_data *)mg_get_user_data(ctx);

	ck_assert_ptr_eq(user_data, (void *)pclient_data);

	ck_assert(pclient_data != NULL);
	ck_assert_int_gt(flags, 128);
	ck_assert_int_lt(flags, 128 + 16);
	ck_assert((flags == (int)(128 | MG_WEBSOCKET_OPCODE_BINARY))
	          || (flags == (int)(128 | MG_WEBSOCKET_OPCODE_TEXT)));

	if (flags == (int)(128 | MG_WEBSOCKET_OPCODE_TEXT)) {
		WS_TEST_TRACE(
		    "Client %i received %lu bytes text data from server: %.*s\n",
		    pclient_data->clientId,
		    (unsigned long)data_len,
		    (int)data_len,
		    data);
	} else {
		WS_TEST_TRACE("Client %i received %lu bytes binary data from\n",
		              pclient_data->clientId,
		              (unsigned long)data_len);
	}

	pclient_data->data = malloc(data_len);
	ck_assert(pclient_data->data != NULL);
	memcpy(pclient_data->data, data, data_len);
	pclient_data->len = data_len;

	mark_point();

	return 1;
}


static void
websocket_client_close_handler(const struct mg_connection *conn,
                               void *user_data)
{
	struct mg_context *ctx = mg_get_context(conn);
	struct tclient_data *pclient_data =
	    (struct tclient_data *)mg_get_user_data(ctx);

#ifndef __MACH__
	ck_assert_ptr_eq(user_data, (void *)pclient_data);

	ck_assert(pclient_data != NULL);

	WS_TEST_TRACE("Client %i: Close handler\n", pclient_data->clientId);
	pclient_data->closed++;

	mark_point();
#else

	(void)user_data;
	pclient_data->closed++;

#endif /* __MACH__ */
}

#endif /* USE_WEBSOCKET */


START_TEST(test_request_handlers)
{
	char ebuf[1024];
	struct mg_context *ctx;
	struct mg_connection *client_conn;
	const struct mg_response_info *client_ri;
	char uri[64];
	char buf[1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 8];
	const char *expected =
	    "112123123412345123456123456712345678123456789123456789A";
	int i;
	const char *request = "GET /U7 HTTP/1.0\r\n\r\n";
#if defined(USE_IPV6) && defined(NO_SSL)
	const char *HTTP_PORT = "8084,[::]:8086";
	short ipv4_port = 8084;
	short ipv6_port = 8086;
#elif !defined(USE_IPV6) && defined(NO_SSL)
	const char *HTTP_PORT = "8084";
	short ipv4_port = 8084;
#elif defined(USE_IPV6) && !defined(NO_SSL)
	const char *HTTP_PORT = "8084,[::]:8086,8194r,[::]:8196r,8094s,[::]:8096s";
	short ipv4_port = 8084;
	short ipv4s_port = 8094;
	short ipv4r_port = 8194;
	short ipv6_port = 8086;
	short ipv6s_port = 8096;
	short ipv6r_port = 8196;
#elif !defined(USE_IPV6) && !defined(NO_SSL)
	const char *HTTP_PORT = "8084,8194r,8094s";
	short ipv4_port = 8084;
	short ipv4s_port = 8094;
	short ipv4r_port = 8194;
#endif

	const char *OPTIONS[16];
	const char *opt;
	FILE *f;
	const char *plain_file_content;
	const char *cgi_script_content;
	const char *expected_cgi_result;
	int opt_idx = 0;
	struct stat st;

	const char encoded_file_content[] = "\x1f\x8b\x08\x08\xf8"
	                                    "\x9d\xcb\x55\x00\x00"
	                                    "test_gz.txt"
	                                    "\x00\x01\x11\x00\xee\xff"
	                                    "zipped text file"
	                                    "\x0a\x34\x5f\xcc\x49"
	                                    "\x11\x00\x00\x00";
	size_t encoded_file_content_len = sizeof(encoded_file_content);


#if !defined(NO_SSL)
	const char *ssl_cert = locate_ssl_cert();
#endif

#if defined(USE_WEBSOCKET)
	struct tclient_data ws_client1_data = {NULL, 0, 0, 1};
	struct tclient_data ws_client2_data = {NULL, 0, 0, 2};
	struct tclient_data ws_client3_data = {NULL, 0, 0, 3};
	struct tclient_data ws_client4_data = {NULL, 0, 0, 4};
	struct mg_connection *ws_client1_conn = NULL;
	struct mg_connection *ws_client2_conn = NULL;
	struct mg_connection *ws_client3_conn = NULL;
	struct mg_connection *ws_client4_conn = NULL;
#endif

	char cmd_buf[1024];
	char *cgi_env_opt;

	mark_point();

	memset((void *)OPTIONS, 0, sizeof(OPTIONS));
	OPTIONS[opt_idx++] = "listening_ports";
	OPTIONS[opt_idx++] = HTTP_PORT;
	OPTIONS[opt_idx++] = "authentication_domain";
	OPTIONS[opt_idx++] = "test.domain";
#if !defined(NO_FILES)
	OPTIONS[opt_idx++] = "document_root";
	OPTIONS[opt_idx++] = ".";
#endif
#ifndef NO_SSL
	ck_assert(ssl_cert != NULL);
	OPTIONS[opt_idx++] = "ssl_certificate";
	OPTIONS[opt_idx++] = ssl_cert;
#endif
	OPTIONS[opt_idx++] = "cgi_environment";
	cgi_env_opt = (char *)calloc(1, 4096 /* CGI_ENVIRONMENT_SIZE */);
	ck_assert(cgi_env_opt != NULL);
	cgi_env_opt[0] = 'x';
	cgi_env_opt[1] = '=';
	memset(cgi_env_opt + 2, 'y', 4090); /* Add large env field, so the server
	                                     * must reallocate buffers. */
	OPTIONS[opt_idx++] = cgi_env_opt;

	OPTIONS[opt_idx++] = "num_threads";
	OPTIONS[opt_idx++] = "2";


	ck_assert_int_le(opt_idx, (int)(sizeof(OPTIONS) / sizeof(OPTIONS[0])));
	ck_assert(OPTIONS[sizeof(OPTIONS) / sizeof(OPTIONS[0]) - 1] == NULL);
	ck_assert(OPTIONS[sizeof(OPTIONS) / sizeof(OPTIONS[0]) - 2] == NULL);

	ctx = test_mg_start(NULL, &g_ctx, OPTIONS);

	ck_assert(ctx != NULL);
	g_ctx = ctx;

	opt = mg_get_option(ctx, "listening_ports");
	ck_assert_str_eq(opt, HTTP_PORT);

	opt = mg_get_option(ctx, "cgi_environment");
	ck_assert_ptr_ne(opt, cgi_env_opt);
	ck_assert_int_eq((int)opt[0], (int)cgi_env_opt[0]);
	ck_assert_int_eq((int)opt[1], (int)cgi_env_opt[1]);
	ck_assert_int_eq((int)opt[2], (int)cgi_env_opt[2]);
	ck_assert_int_eq((int)opt[3], (int)cgi_env_opt[3]);
	/* full length string compare will reach limit in the implementation
	 * of the check unit test framework */
	{
		size_t len_check_1 = strlen(opt);
		size_t len_check_2 = strlen(cgi_env_opt);
		ck_assert_uint_eq(len_check_1, len_check_2);
	}

	/* We don't need the original anymore, the server has a private copy */
	free(cgi_env_opt);

	opt = mg_get_option(ctx, "throttle");
	ck_assert_str_eq(opt, "");

	opt = mg_get_option(ctx, "unknown_option_name");
	ck_assert(opt == NULL);

	for (i = 0; i < 1000; i++) {
		sprintf(uri, "/U%u", i);
		mg_set_request_handler(ctx, uri, request_test_handler, NULL);
	}
	for (i = 500; i < 800; i++) {
		sprintf(uri, "/U%u", i);
		mg_set_request_handler(ctx, uri, NULL, (void *)(ptrdiff_t)1);
	}
	for (i = 600; i >= 0; i--) {
		sprintf(uri, "/U%u", i);
		mg_set_request_handler(ctx, uri, NULL, (void *)(ptrdiff_t)2);
	}
	for (i = 750; i <= 1000; i++) {
		sprintf(uri, "/U%u", i);
		mg_set_request_handler(ctx, uri, NULL, (void *)(ptrdiff_t)3);
	}
	for (i = 5; i < 9; i++) {
		sprintf(uri, "/U%u", i);
		mg_set_request_handler(ctx,
		                       uri,
		                       request_test_handler,
		                       (void *)(ptrdiff_t)i);
	}

	mg_set_request_handler(ctx, "/handler2", request_test_handler2, NULL);

#ifdef USE_WEBSOCKET
	mg_set_websocket_handler(ctx,
	                         "/websocket",
	                         websock_server_connect,
	                         websock_server_ready,
	                         websock_server_data,
	                         websock_server_close,
	                         (void *)(ptrdiff_t)7531);
#endif

	/* Try to load non existing file */
	client_conn = mg_download("localhost",
	                          ipv4_port,
	                          0,
	                          ebuf,
	                          sizeof(ebuf),
	                          "%s",
	                          "GET /file/not/found HTTP/1.0\r\n\r\n");
	ck_assert(client_conn != NULL);
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 404);
	mg_close_connection(client_conn);

	/* Get data from callback */
	client_conn = mg_download(
	    "localhost", ipv4_port, 0, ebuf, sizeof(ebuf), "%s", request);
	ck_assert(client_conn != NULL);
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	i = mg_read(client_conn, buf, sizeof(buf));
	ck_assert_int_eq(i, (int)strlen(expected));
	buf[i] = 0;
	ck_assert_str_eq(buf, expected);
	mg_close_connection(client_conn);

	/* Get data from callback using http://127.0.0.1 */
	client_conn = mg_download(
	    "127.0.0.1", ipv4_port, 0, ebuf, sizeof(ebuf), "%s", request);
	ck_assert(client_conn != NULL);
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	i = mg_read(client_conn, buf, sizeof(buf));
	if ((i >= 0) && ((size_t)i < sizeof(buf))) {
		buf[i] = 0;
	} else {
		ck_abort_msg(
		    "ERROR: test_request_handlers: read returned %i (>=0, <%i)",
		    (int)i,
		    (int)sizeof(buf));
	}
	ck_assert((int)i < (int)sizeof(buf));
	ck_assert(i > 0);
	ck_assert_int_eq(i, (int)strlen(expected));
	buf[i] = 0;
	ck_assert_str_eq(buf, expected);
	mg_close_connection(client_conn);

#if defined(USE_IPV6)
	/* Get data from callback using http://[::1] */
	client_conn =
	    mg_download("[::1]", ipv6_port, 0, ebuf, sizeof(ebuf), "%s", request);
	ck_assert(client_conn != NULL);
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	i = mg_read(client_conn, buf, sizeof(buf));
	ck_assert_int_eq(i, (int)strlen(expected));
	buf[i] = 0;
	ck_assert_str_eq(buf, expected);
	mg_close_connection(client_conn);
#endif

#if !defined(NO_SSL)
	/* Get data from callback using https://127.0.0.1 */
	client_conn = mg_download(
	    "127.0.0.1", ipv4s_port, 1, ebuf, sizeof(ebuf), "%s", request);
	ck_assert(client_conn != NULL);
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	i = mg_read(client_conn, buf, sizeof(buf));
	ck_assert_int_eq(i, (int)strlen(expected));
	buf[i] = 0;
	ck_assert_str_eq(buf, expected);
	mg_close_connection(client_conn);

	/* Get redirect from callback using http://127.0.0.1 */
	client_conn = mg_download(
	    "127.0.0.1", ipv4r_port, 0, ebuf, sizeof(ebuf), "%s", request);
	ck_assert(client_conn != NULL);
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert((client_ri->status_code == 301) || (client_ri->status_code == 302)
	          || (client_ri->status_code == 303)
	          || (client_ri->status_code == 307)
	          || (client_ri->status_code == 308)); /* is a redirect code */
	/*
	// A redirect may have a body, or not
	i = mg_read(client_conn, buf, sizeof(buf));
	ck_assert_int_eq(i, -1);
	*/
	mg_close_connection(client_conn);
#endif

#if defined(USE_IPV6) && !defined(NO_SSL)
	/* Get data from callback using https://[::1] */
	client_conn =
	    mg_download("[::1]", ipv6s_port, 1, ebuf, sizeof(ebuf), "%s", request);
	ck_assert(client_conn != NULL);
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	i = mg_read(client_conn, buf, sizeof(buf));
	ck_assert_int_eq(i, (int)strlen(expected));
	buf[i] = 0;
	ck_assert_str_eq(buf, expected);
	mg_close_connection(client_conn);

	/* Get redirect from callback using http://127.0.0.1 */
	client_conn =
	    mg_download("[::1]", ipv6r_port, 0, ebuf, sizeof(ebuf), "%s", request);
	ck_assert(client_conn != NULL);
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert((client_ri->status_code == 301) || (client_ri->status_code == 302)
	          || (client_ri->status_code == 303)
	          || (client_ri->status_code == 307)
	          || (client_ri->status_code == 308)); /* is a redirect code */
	/*
	// A redirect may have a body, or not
	i = mg_read(client_conn, buf, sizeof(buf));
	ck_assert_int_eq(i, -1);
	*/
	mg_close_connection(client_conn);
#endif

/* It seems to be impossible to find out what the actual working
 * directory of the CI test environment is. Before breaking another
 * dozen of builds by trying blindly with different paths, just
 * create the file here */
#ifdef _WIN32
	f = fopen("test.txt", "wb");
#else
	f = fopen("test.txt", "w");
#endif
	ck_assert(f != NULL);
	plain_file_content = "simple text file\n";
	i = (int)strlen(plain_file_content);
	ck_assert_int_eq(i, 17);

	fwrite(plain_file_content, i, 1, f);
	fclose(f);

#ifdef _WIN32
	f = fopen("test_gz.txt.gz", "wb");
#else
	f = fopen("test_gz.txt.gz", "w");
#endif
	ck_assert(f != NULL);
	ck_assert_uint_ge(encoded_file_content_len, 52);
	encoded_file_content_len = 52;
	fwrite(encoded_file_content, 1, encoded_file_content_len, f);
	fclose(f);

#ifdef _WIN32
	f = fopen("test.cgi", "wb");
	cgi_script_content = "#!test.cgi.cmd\r\n";
	fwrite(cgi_script_content, strlen(cgi_script_content), 1, f);
	fclose(f);
	f = fopen("test.cgi.cmd", "w");
	cgi_script_content = "@echo off\r\n"
	                     "echo Connection: close\r\n"
	                     "echo Content-Type: text/plain\r\n"
	                     "echo.\r\n"
	                     "echo CGI test\r\n"
	                     "\r\n";
	fwrite(cgi_script_content, strlen(cgi_script_content), 1, f);
	fclose(f);
#else
	f = fopen("test.cgi", "w");
	cgi_script_content = "#!/bin/sh\n\n"
	                     "printf \"Connection: close\\r\\n\"\n"
	                     "printf \"Content-Type: text/plain\\r\\n\"\n"
	                     "printf \"\\r\\n\"\n"
	                     "printf \"CGI test\\r\\n\"\n"
	                     "\n";
	(void)fwrite(cgi_script_content, strlen(cgi_script_content), 1, f);
	(void)fclose(f);
	(void)system("chmod a+x test.cgi");
#endif
	expected_cgi_result = "CGI test";

	/* Get static data */
	client_conn = mg_download("localhost",
	                          ipv4_port,
	                          0,
	                          ebuf,
	                          sizeof(ebuf),
	                          "%s",
	                          "GET /test.txt HTTP/1.0\r\n\r\n");
	ck_assert(client_conn != NULL);
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);

#if defined(NO_FILES)
	ck_assert_int_eq(client_ri->status_code, 404);
#else
	ck_assert_int_eq(client_ri->status_code, 200);
	i = mg_read(client_conn, buf, sizeof(buf));
	ck_assert_int_eq(i, 17);
	if ((i >= 0) && (i < (int)sizeof(buf))) {
		buf[i] = 0;
	}
	ck_assert_str_eq(buf, plain_file_content);
#endif
	mg_close_connection(client_conn);


	/* Check if CGI test executable exists */
	memset(&st, 0, sizeof(st));

#if defined(_WIN32)
	/* TODO: not yet available */
	sprintf(ebuf, "%scgi_test.cgi", locate_test_exes());
#else
	sprintf(ebuf, "%scgi_test.cgi", locate_test_exes());

	if (stat(ebuf, &st) != 0) {
		fprintf(stderr, "\nFile %s not found\n", ebuf);
		fprintf(stderr,
		        "This file needs to be compiled manually before "
		        "starting the test\n");
		fprintf(stderr,
		        "e.g. by gcc test/cgi_test.c -o output/cgi_test.cgi\n\n");

		/* Abort test with diagnostic message */
		ck_abort_msg("Mandatory file %s must be built before starting the test",
		             ebuf);
	}
#endif


/* Test with CGI test executable */
#if defined(_WIN32)
	sprintf(cmd_buf, "copy %s\\cgi_test.cgi cgi_test.exe", locate_test_exes());
#else
	sprintf(cmd_buf, "cp %s/cgi_test.cgi cgi_test.cgi", locate_test_exes());
#endif
	(void)system(cmd_buf);

#if !defined(NO_CGI) && !defined(NO_FILES) && !defined(_WIN32)
	/* TODO: add test for windows, check with POST */
	client_conn = mg_download(
	    "localhost",
	    ipv4_port,
	    0,
	    ebuf,
	    sizeof(ebuf),
	    "%s",
	    "POST /cgi_test.cgi/x/y.z HTTP/1.0\r\nContent-Length: 3\r\n\r\nABC");
	ck_assert(client_conn != NULL);
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	mg_close_connection(client_conn);
#endif

	/* Get zipped static data - will not work if Accept-Encoding is not set */
	client_conn = mg_download("localhost",
	                          ipv4_port,
	                          0,
	                          ebuf,
	                          sizeof(ebuf),
	                          "%s",
	                          "GET /test_gz.txt HTTP/1.0\r\n\r\n");
	ck_assert(client_conn != NULL);
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 404);
	mg_close_connection(client_conn);

	/* Get zipped static data - with Accept-Encoding */
	client_conn = mg_download(
	    "localhost",
	    ipv4_port,
	    0,
	    ebuf,
	    sizeof(ebuf),
	    "%s",
	    "GET /test_gz.txt HTTP/1.0\r\nAccept-Encoding: gzip\r\n\r\n");
	ck_assert(client_conn != NULL);
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);

#if defined(NO_FILES)
	ck_assert_int_eq(client_ri->status_code, 404);
#else
	ck_assert_int_eq(client_ri->status_code, 200);
	i = mg_read(client_conn, buf, sizeof(buf));
	ck_assert_int_eq(i, 52);
	if ((i >= 0) && (i < (int)sizeof(buf))) {
		buf[i] = 0;
	}
	ck_assert_int_eq(client_ri->content_length, 52);
	ck_assert_str_eq(buf, encoded_file_content);
#endif
	mg_close_connection(client_conn);

/* Get CGI generated data */
#if !defined(NO_CGI)
	client_conn = mg_download("localhost",
	                          ipv4_port,
	                          0,
	                          ebuf,
	                          sizeof(ebuf),
	                          "%s",
	                          "GET /test.cgi HTTP/1.0\r\n\r\n");
	ck_assert(client_conn != NULL);
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);

#if defined(NO_FILES)
	ck_assert_int_eq(client_ri->status_code, 404);

	(void)expected_cgi_result;
	(void)cgi_script_content;
#else
	i = mg_read(client_conn, buf, sizeof(buf));
	if ((i >= 0) && (i < (int)sizeof(buf))) {
		while ((i > 0) && ((buf[i - 1] == '\r') || (buf[i - 1] == '\n'))) {
			i--;
		}
		buf[i] = 0;
	}
	/* ck_assert_int_eq(i, (int)strlen(expected_cgi_result)); */
	ck_assert_str_eq(buf, expected_cgi_result);
	ck_assert_int_eq(client_ri->status_code, 200);
	mg_close_connection(client_conn);
#endif

#else
	(void)expected_cgi_result;
	(void)cgi_script_content;
#endif

	/* Get directory listing */
	client_conn = mg_download("localhost",
	                          ipv4_port,
	                          0,
	                          ebuf,
	                          sizeof(ebuf),
	                          "%s",
	                          "GET / HTTP/1.0\r\n\r\n");
	ck_assert(client_conn != NULL);
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
#if defined(NO_FILES)
	ck_assert_int_eq(client_ri->status_code, 404);
#else
	ck_assert_int_eq(client_ri->status_code, 200);
	i = mg_read(client_conn, buf, sizeof(buf));
	ck_assert(i > 6);
	buf[6] = 0;
	ck_assert_str_eq(buf, "<html>");
#endif
	mg_close_connection(client_conn);

	/* POST to static file (will not work) */
	client_conn = mg_download("localhost",
	                          ipv4_port,
	                          0,
	                          ebuf,
	                          sizeof(ebuf),
	                          "%s",
	                          "POST /test.txt HTTP/1.0\r\n\r\n");
	ck_assert(client_conn != NULL);
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
#if defined(NO_FILES)
	ck_assert_int_eq(client_ri->status_code, 404);
#else
	ck_assert_int_eq(client_ri->status_code, 405);
	i = mg_read(client_conn, buf, sizeof(buf));
	ck_assert(i >= 29);
	buf[29] = 0;
	ck_assert_str_eq(buf, "Error 405: Method Not Allowed");
#endif
	mg_close_connection(client_conn);

	/* PUT to static file (will not work) */
	client_conn = mg_download("localhost",
	                          ipv4_port,
	                          0,
	                          ebuf,
	                          sizeof(ebuf),
	                          "%s",
	                          "PUT /test.txt HTTP/1.0\r\n\r\n");
	ck_assert(client_conn != NULL);
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
#if defined(NO_FILES)
	ck_assert_int_eq(client_ri->status_code, 405); /* method not allowed */
#else
	ck_assert_int_eq(client_ri->status_code, 401); /* not authorized */
#endif
	mg_close_connection(client_conn);


	/* Get data from callback using mg_connect_client instead of mg_download */
	memset(ebuf, 0, sizeof(ebuf));
	client_conn =
	    mg_connect_client("127.0.0.1", ipv4_port, 0, ebuf, sizeof(ebuf));

	ck_assert_str_eq(ebuf, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "%s", request);

	i = mg_get_response(client_conn, ebuf, sizeof(ebuf), 10000);
	ck_assert_int_ge(i, 0);
	ck_assert_str_eq(ebuf, "");

	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	i = mg_read(client_conn, buf, sizeof(buf));
	ck_assert_int_eq(i, (int)strlen(expected));
	buf[i] = 0;
	ck_assert_str_eq(buf, expected);
	mg_close_connection(client_conn);

	/* Get data from callback using mg_connect_client and absolute URI */
	memset(ebuf, 0, sizeof(ebuf));
	client_conn =
	    mg_connect_client("localhost", ipv4_port, 0, ebuf, sizeof(ebuf));

	ck_assert_str_eq(ebuf, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn,
	          "GET http://test.domain:%d/U7 HTTP/1.0\r\n\r\n",
	          ipv4_port);

	i = mg_get_response(client_conn, ebuf, sizeof(ebuf), 10000);
	ck_assert_int_ge(i, 0);
	ck_assert_str_eq(ebuf, "");

	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	i = mg_read(client_conn, buf, sizeof(buf));
	ck_assert_int_eq(i, (int)strlen(expected));
	buf[i] = 0;
	ck_assert_str_eq(buf, expected);
	mg_close_connection(client_conn);

	/* Get data from callback using mg_connect_client and absolute URI with a
	 * sub-domain */
	memset(ebuf, 0, sizeof(ebuf));
	client_conn =
	    mg_connect_client("localhost", ipv4_port, 0, ebuf, sizeof(ebuf));

	ck_assert_str_eq(ebuf, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn,
	          "GET http://subdomain.test.domain:%d/U7 HTTP/1.0\r\n\r\n",
	          ipv4_port);

	i = mg_get_response(client_conn, ebuf, sizeof(ebuf), 10000);
	ck_assert_int_ge(i, 0);
	ck_assert_str_eq(ebuf, "");

	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	i = mg_read(client_conn, buf, sizeof(buf));
	ck_assert_int_eq(i, (int)strlen(expected));
	buf[i] = 0;
	ck_assert_str_eq(buf, expected);
	mg_close_connection(client_conn);

	/* Get data from handler2 */
	client_conn =
	    mg_connect_client("localhost", ipv4_port, 0, ebuf, sizeof(ebuf));

	ck_assert_str_eq(ebuf, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn,
	          "GET /handler2 HTTP/1.1\r\n"
	          "Host: localhost\r\n"
	          "\r\n",
	          ipv4_port);

	i = mg_get_response(client_conn, ebuf, sizeof(ebuf), 10000);
	ck_assert_int_ge(i, 0);
	ck_assert_str_eq(ebuf, "");

	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	i = mg_read(client_conn, buf, sizeof(buf));
	ck_assert_int_eq(i, (int)strlen(expected));
	buf[i] = 0;
	ck_assert_str_eq(buf, expected);
	mg_close_connection(client_conn);

	/* Get data non existing handler (will return 404) */
	client_conn =
	    mg_connect_client("localhost", ipv4_port, 0, ebuf, sizeof(ebuf));

	ck_assert_str_eq(ebuf, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn,
	          "GET /unknown_url HTTP/1.1\r\n"
	          "Host: localhost\r\n"
	          "\r\n",
	          ipv4_port);

	i = mg_get_response(client_conn, ebuf, sizeof(ebuf), 10000);
	ck_assert_int_ge(i, 0);
	ck_assert_str_eq(ebuf, "");

	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 404);
	mg_close_connection(client_conn);

	/* Get data from handler2, but only read a part of it */
	client_conn =
	    mg_connect_client("localhost", ipv4_port, 0, ebuf, sizeof(ebuf));

	ck_assert_str_eq(ebuf, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn,
	          "GET /handler2 HTTP/1.1\r\n"
	          "Host: localhost\r\n"
	          "\r\n",
	          ipv4_port);

	i = mg_get_response(client_conn, ebuf, sizeof(ebuf), 10000);
	ck_assert_int_ge(i, 0);
	ck_assert_str_eq(ebuf, "");

	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	i = mg_read(client_conn, buf, 7);
	ck_assert_int_eq(i, 7);
	ck_assert(0 == memcmp(buf, expected, 7));
	mg_close_connection(client_conn);


/* Websocket test */
#ifdef USE_WEBSOCKET
	/* Then connect a first client */
	ws_client1_conn =
	    mg_connect_websocket_client("localhost",
	                                ipv4_port,
	                                0,
	                                ebuf,
	                                sizeof(ebuf),
	                                "/websocket",
	                                NULL,
	                                websocket_client_data_handler,
	                                websocket_client_close_handler,
	                                &ws_client1_data);

	ck_assert(ws_client1_conn != NULL);

	wait_not_null(
	    &(ws_client1_data.data)); /* Wait for the websocket welcome message */
	ck_assert_int_eq(ws_client1_data.closed, 0);
	ck_assert_int_eq(ws_client2_data.closed, 0);
	ck_assert_int_eq(ws_client3_data.closed, 0);
	ck_assert(ws_client2_data.data == NULL);
	ck_assert_uint_eq(ws_client2_data.len, 0);
	ck_assert(ws_client1_data.data != NULL);
	ck_assert_uint_eq(ws_client1_data.len, websocket_welcome_msg_len);
	ck_assert(!memcmp(ws_client1_data.data,
	                  websocket_welcome_msg,
	                  websocket_welcome_msg_len));
	free(ws_client1_data.data);
	ws_client1_data.data = NULL;
	ws_client1_data.len = 0;

	mg_websocket_client_write(ws_client1_conn,
	                          MG_WEBSOCKET_OPCODE_TEXT,
	                          "data1",
	                          5);

	wait_not_null(&(
	    ws_client1_data.data)); /* Wait for the websocket acknowledge message */
	ck_assert_int_eq(ws_client1_data.closed, 0);
	ck_assert_int_eq(ws_client2_data.closed, 0);
	ck_assert(ws_client2_data.data == NULL);
	ck_assert_uint_eq(ws_client2_data.len, 0);
	ck_assert(ws_client1_data.data != NULL);
	ck_assert_uint_eq(ws_client1_data.len, 3);
	ck_assert(!memcmp(ws_client1_data.data, "ok1", 3));
	free(ws_client1_data.data);
	ws_client1_data.data = NULL;
	ws_client1_data.len = 0;

/* Now connect a second client */
#ifdef USE_IPV6
	ws_client2_conn =
	    mg_connect_websocket_client("[::1]",
	                                ipv6_port,
	                                0,
	                                ebuf,
	                                sizeof(ebuf),
	                                "/websocket",
	                                NULL,
	                                websocket_client_data_handler,
	                                websocket_client_close_handler,
	                                &ws_client2_data);
#else
	ws_client2_conn =
	    mg_connect_websocket_client("127.0.0.1",
	                                ipv4_port,
	                                0,
	                                ebuf,
	                                sizeof(ebuf),
	                                "/websocket",
	                                NULL,
	                                websocket_client_data_handler,
	                                websocket_client_close_handler,
	                                &ws_client2_data);
#endif
	ck_assert(ws_client2_conn != NULL);

	wait_not_null(
	    &(ws_client2_data.data)); /* Wait for the websocket welcome message */
	ck_assert(ws_client1_data.closed == 0);
	ck_assert(ws_client2_data.closed == 0);
	ck_assert(ws_client1_data.data == NULL);
	ck_assert(ws_client1_data.len == 0);
	ck_assert(ws_client2_data.data != NULL);
	ck_assert(ws_client2_data.len == websocket_welcome_msg_len);
	ck_assert(!memcmp(ws_client2_data.data,
	                  websocket_welcome_msg,
	                  websocket_welcome_msg_len));
	free(ws_client2_data.data);
	ws_client2_data.data = NULL;
	ws_client2_data.len = 0;

	mg_websocket_client_write(ws_client1_conn,
	                          MG_WEBSOCKET_OPCODE_TEXT,
	                          "data2",
	                          5);

	wait_not_null(&(
	    ws_client1_data.data)); /* Wait for the websocket acknowledge message */

	ck_assert(ws_client1_data.closed == 0);
	ck_assert(ws_client2_data.closed == 0);
	ck_assert(ws_client2_data.data == NULL);
	ck_assert(ws_client2_data.len == 0);
	ck_assert(ws_client1_data.data != NULL);
	ck_assert(ws_client1_data.len == 4);
	ck_assert(!memcmp(ws_client1_data.data, "ok 2", 4));
	free(ws_client1_data.data);
	ws_client1_data.data = NULL;
	ws_client1_data.len = 0;

	mg_websocket_client_write(ws_client1_conn,
	                          MG_WEBSOCKET_OPCODE_TEXT,
	                          "bye",
	                          3);

	wait_not_null(
	    &(ws_client1_data.data)); /* Wait for the websocket goodbye message */

	ck_assert(ws_client1_data.closed == 0);
	ck_assert(ws_client2_data.closed == 0);
	ck_assert(ws_client2_data.data == NULL);
	ck_assert(ws_client2_data.len == 0);
	ck_assert(ws_client1_data.data != NULL);
	ck_assert(ws_client1_data.len == websocket_goodbye_msg_len);
	ck_assert(!memcmp(ws_client1_data.data,
	                  websocket_goodbye_msg,
	                  websocket_goodbye_msg_len));
	free(ws_client1_data.data);
	ws_client1_data.data = NULL;
	ws_client1_data.len = 0;

	ck_assert(ws_client1_data.closed == 0); /* Not closed */

	mg_close_connection(ws_client1_conn);

	test_sleep(3); /* Won't get any message */

	ck_assert(ws_client1_data.closed == 1); /* Closed */

	ck_assert(ws_client2_data.closed == 0);
	ck_assert(ws_client1_data.data == NULL);
	ck_assert(ws_client1_data.len == 0);
	ck_assert(ws_client2_data.data == NULL);
	ck_assert(ws_client2_data.len == 0);

	mg_websocket_client_write(ws_client2_conn,
	                          MG_WEBSOCKET_OPCODE_TEXT,
	                          "bye",
	                          3);

	wait_not_null(
	    &(ws_client2_data.data)); /* Wait for the websocket goodbye message */

	ck_assert(ws_client1_data.closed == 1);
	ck_assert(ws_client2_data.closed == 0);
	ck_assert(ws_client1_data.data == NULL);
	ck_assert(ws_client1_data.len == 0);
	ck_assert(ws_client2_data.data != NULL);
	ck_assert(ws_client2_data.len == websocket_goodbye_msg_len);
	ck_assert(!memcmp(ws_client2_data.data,
	                  websocket_goodbye_msg,
	                  websocket_goodbye_msg_len));
	free(ws_client2_data.data);
	ws_client2_data.data = NULL;
	ws_client2_data.len = 0;

	mg_close_connection(ws_client2_conn);

	test_sleep(3); /* Won't get any message */

	ck_assert(ws_client1_data.closed == 1);
	ck_assert(ws_client2_data.closed == 1);
	ck_assert(ws_client1_data.data == NULL);
	ck_assert(ws_client1_data.len == 0);
	ck_assert(ws_client2_data.data == NULL);
	ck_assert(ws_client2_data.len == 0);

	/* Connect client 3 */
	ws_client3_conn =
	    mg_connect_websocket_client("localhost",
#if defined(NO_SSL)
	                                ipv4_port,
	                                0,
#else
	                                ipv4s_port,
	                                1,
#endif
	                                ebuf,
	                                sizeof(ebuf),
	                                "/websocket",
	                                NULL,
	                                websocket_client_data_handler,
	                                websocket_client_close_handler,
	                                &ws_client3_data);

	ck_assert(ws_client3_conn != NULL);

	wait_not_null(
	    &(ws_client3_data.data)); /* Wait for the websocket welcome message */
	ck_assert(ws_client1_data.closed == 1);
	ck_assert(ws_client2_data.closed == 1);
	ck_assert(ws_client3_data.closed == 0);
	ck_assert(ws_client1_data.data == NULL);
	ck_assert(ws_client1_data.len == 0);
	ck_assert(ws_client2_data.data == NULL);
	ck_assert(ws_client2_data.len == 0);
	ck_assert(ws_client3_data.data != NULL);
	ck_assert(ws_client3_data.len == websocket_welcome_msg_len);
	ck_assert(!memcmp(ws_client3_data.data,
	                  websocket_welcome_msg,
	                  websocket_welcome_msg_len));
	free(ws_client3_data.data);
	ws_client3_data.data = NULL;
	ws_client3_data.len = 0;

	/* Write long data (16 bit size header) */
	mg_websocket_client_write(ws_client3_conn,
	                          MG_WEBSOCKET_OPCODE_BINARY,
	                          long_ws_buf,
	                          long_ws_buf_len_16);

	/* Wait for the response */
	wait_not_null(&(ws_client3_data.data));

	ck_assert_int_eq((int)ws_client3_data.len, (int)long_ws_buf_len_16);
	ck_assert(!memcmp(ws_client3_data.data, long_ws_buf, long_ws_buf_len_16));
	free(ws_client3_data.data);
	ws_client3_data.data = NULL;
	ws_client3_data.len = 0;

	/* Write long data (64 bit size header) */
	mg_websocket_client_write(ws_client3_conn,
	                          MG_WEBSOCKET_OPCODE_BINARY,
	                          long_ws_buf,
	                          long_ws_buf_len_64);

	/* Wait for the response */
	wait_not_null(&(ws_client3_data.data));

	ck_assert_int_eq((int)ws_client3_data.len, (int)long_ws_buf_len_64);
	ck_assert(!memcmp(ws_client3_data.data, long_ws_buf, long_ws_buf_len_64));
	free(ws_client3_data.data);
	ws_client3_data.data = NULL;
	ws_client3_data.len = 0;

	/* Disconnect client 3 */
	ck_assert(ws_client3_data.closed == 0);
	mg_close_connection(ws_client3_conn);
	ck_assert(ws_client3_data.closed == 1);

	/* Connect client 4 */
	ws_client4_conn =
	    mg_connect_websocket_client("localhost",
#if defined(NO_SSL)
	                                ipv4_port,
	                                0,
#else
	                                ipv4s_port,
	                                1,
#endif
	                                ebuf,
	                                sizeof(ebuf),
	                                "/websocket",
	                                NULL,
	                                websocket_client_data_handler,
	                                websocket_client_close_handler,
	                                &ws_client4_data);

	ck_assert(ws_client4_conn != NULL);

	wait_not_null(
	    &(ws_client4_data.data)); /* Wait for the websocket welcome message */
	ck_assert(ws_client1_data.closed == 1);
	ck_assert(ws_client2_data.closed == 1);
	ck_assert(ws_client3_data.closed == 1);
	ck_assert(ws_client4_data.closed == 0);
	ck_assert(ws_client4_data.data != NULL);
	ck_assert(ws_client4_data.len == websocket_welcome_msg_len);
	ck_assert(!memcmp(ws_client4_data.data,
	                  websocket_welcome_msg,
	                  websocket_welcome_msg_len));
	free(ws_client4_data.data);
	ws_client4_data.data = NULL;
	ws_client4_data.len = 0;

	/* stop the server without closing this connection */

#endif

	/* Close the server */
	g_ctx = NULL;
	test_mg_stop(ctx);
	mark_point();

#ifdef USE_WEBSOCKET
	for (i = 0; i < 100; i++) {
		test_sleep(1);
		if (ws_client3_data.closed != 0) {
			mark_point();
			break;
		}
	}

	ck_assert_int_eq(ws_client4_data.closed, 1);

	/* Free data in ws_client4_conn */
	mg_close_connection(ws_client4_conn);

#endif
	mark_point();
}
END_TEST


static int g_field_found_return = -999;

static int
field_found(const char *key,
            const char *filename,
            char *path,
            size_t pathlen,
            void *user_data)
{
	ck_assert_ptr_ne(key, NULL);
	ck_assert_ptr_ne(filename, NULL);
	ck_assert_ptr_ne(path, NULL);
	ck_assert_uint_gt(pathlen, 128);
	ck_assert_ptr_eq(user_data, (void *)&g_field_found_return);

	ck_assert((g_field_found_return == MG_FORM_FIELD_STORAGE_GET)
	          || (g_field_found_return == MG_FORM_FIELD_STORAGE_STORE)
	          || (g_field_found_return == MG_FORM_FIELD_STORAGE_SKIP)
	          || (g_field_found_return == MG_FORM_FIELD_STORAGE_ABORT));

	ck_assert_str_ne(key, "dontread");

	if (!strcmp(key, "break_field_handler")) {
		return MG_FORM_FIELD_STORAGE_ABORT;
	}
	if (!strcmp(key, "continue_field_handler")) {
		return MG_FORM_FIELD_STORAGE_SKIP;
	}

	if (g_field_found_return == MG_FORM_FIELD_STORAGE_STORE) {
		strncpy(path, key, pathlen - 8);
		strcat(path, ".txt");
	}

	mark_point();

	return g_field_found_return;
}


static int g_field_step;

static int
field_get(const char *key,
          const char *value_untruncated,
          size_t valuelen,
          void *user_data)
{
	/* Copy the untruncated value, so string compare functions can be used. */
	/* The check unit test library does not have build in memcmp functions. */
	char *value = (char *)malloc(valuelen + 1);
	ck_assert(value != NULL);
	memcpy(value, value_untruncated, valuelen);
	value[valuelen] = 0;

	ck_assert_ptr_eq(user_data, (void *)&g_field_found_return);
	ck_assert_int_ge(g_field_step, 0);

	++g_field_step;
	switch (g_field_step) {
	case 1:
		ck_assert_str_eq(key, "textin");
		ck_assert_uint_eq(valuelen, 4);
		ck_assert_str_eq(value, "text");
		break;
	case 2:
		ck_assert_str_eq(key, "passwordin");
		ck_assert_uint_eq(valuelen, 0);
		ck_assert_str_eq(value, "");
		break;
	case 3:
		ck_assert_str_eq(key, "radio1");
		ck_assert_uint_eq(valuelen, 4);
		ck_assert_str_eq(value, "val1");
		break;
	case 4:
		ck_assert_str_eq(key, "radio2");
		ck_assert_uint_eq(valuelen, 4);
		ck_assert_str_eq(value, "val1");
		break;
	case 5:
		ck_assert_str_eq(key, "check1");
		ck_assert_uint_eq(valuelen, 4);
		ck_assert_str_eq(value, "val1");
		break;
	case 6:
		ck_assert_str_eq(key, "numberin");
		ck_assert_uint_eq(valuelen, 1);
		ck_assert_str_eq(value, "1");
		break;
	case 7:
		ck_assert_str_eq(key, "datein");
		ck_assert_uint_eq(valuelen, 8);
		ck_assert_str_eq(value, "1.1.2016");
		break;
	case 8:
		ck_assert_str_eq(key, "colorin");
		ck_assert_uint_eq(valuelen, 7);
		ck_assert_str_eq(value, "#80ff00");
		break;
	case 9:
		ck_assert_str_eq(key, "rangein");
		ck_assert_uint_eq(valuelen, 1);
		ck_assert_str_eq(value, "3");
		break;
	case 10:
		ck_assert_str_eq(key, "monthin");
		ck_assert_uint_eq(valuelen, 0);
		ck_assert_str_eq(value, "");
		break;
	case 11:
		ck_assert_str_eq(key, "weekin");
		ck_assert_uint_eq(valuelen, 0);
		ck_assert_str_eq(value, "");
		break;
	case 12:
		ck_assert_str_eq(key, "timein");
		ck_assert_uint_eq(valuelen, 0);
		ck_assert_str_eq(value, "");
		break;
	case 13:
		ck_assert_str_eq(key, "datetimen");
		ck_assert_uint_eq(valuelen, 0);
		ck_assert_str_eq(value, "");
		break;
	case 14:
		ck_assert_str_eq(key, "datetimelocalin");
		ck_assert_uint_eq(valuelen, 0);
		ck_assert_str_eq(value, "");
		break;
	case 15:
		ck_assert_str_eq(key, "emailin");
		ck_assert_uint_eq(valuelen, 0);
		ck_assert_str_eq(value, "");
		break;
	case 16:
		ck_assert_str_eq(key, "searchin");
		ck_assert_uint_eq(valuelen, 0);
		ck_assert_str_eq(value, "");
		break;
	case 17:
		ck_assert_str_eq(key, "telin");
		ck_assert_uint_eq(valuelen, 0);
		ck_assert_str_eq(value, "");
		break;
	case 18:
		ck_assert_str_eq(key, "urlin");
		ck_assert_uint_eq(valuelen, 0);
		ck_assert_str_eq(value, "");
		break;
	case 19:
		ck_assert_str_eq(key, "filein");
		ck_assert_uint_eq(valuelen, 0);
		ck_assert_str_eq(value, "");
		break;
	case 20:
		ck_assert_str_eq(key, "filesin");
		ck_assert_uint_eq(valuelen, 0);
		ck_assert_str_eq(value, "");
		break;
	case 21:
		ck_assert_str_eq(key, "selectin");
		ck_assert_uint_eq(valuelen, 4);
		ck_assert_str_eq(value, "opt1");
		break;
	case 22:
		ck_assert_str_eq(key, "message");
		ck_assert_uint_eq(valuelen, 23);
		ck_assert_str_eq(value, "Text area default text.");
		break;
	default:
		ck_abort_msg("field_get called with g_field_step == %i",
		             (int)g_field_step);
	}

	free(value);
	mark_point();

	return 0;
}


static const char *myfile_content = "Content of myfile.txt\r\n";
static const int myfile_content_rep = 500;
static int myfile_content_len = 23; /* (int)strlen(myfile_content); */


static int
field_store(const char *path, long long file_size, void *user_data)
{
	FILE *f;

	ck_assert_int_eq(myfile_content_len, 23);
	ck_assert_int_eq(myfile_content_len, (int)strlen(myfile_content));

	ck_assert_ptr_eq(user_data, (void *)&g_field_found_return);
	ck_assert_int_ge(g_field_step, 100);

	++g_field_step;
	switch (g_field_step) {
	case 101:
		ck_assert_str_eq(path, "storeme.txt");
		ck_assert_int_eq(file_size, 9);
		f = fopen(path, "r");
		ck_assert_ptr_ne(f, NULL);
		if (f) {
			char buf[32] = {0};
			int i = (int)fread(buf, 1, sizeof(buf) - 1, f);
			ck_assert_int_eq(i, 9);
			fclose(f);
			ck_assert_str_eq(buf, "storetest");
		}
		break;
	case 102:
		ck_assert_str_eq(path, "file2store.txt");
		ck_assert_int_eq(myfile_content_len, (int)strlen(myfile_content));
		ck_assert_int_eq(file_size, myfile_content_len * myfile_content_rep);
#ifdef _WIN32
		f = fopen(path, "rb");
#else
		f = fopen(path, "r");
#endif
		ck_assert_ptr_ne(f, NULL);
		if (f) {
			char buf[32] = {0};
			int r, i;
			for (r = 0; r < myfile_content_rep; r++) {
				i = (int)fread(buf, 1, myfile_content_len, f);
				ck_assert_int_eq(i, myfile_content_len);
				ck_assert_str_eq(buf, myfile_content);
			}
			i = (int)fread(buf, 1, myfile_content_len, f);
			ck_assert_int_eq(i, 0);
			fclose(f);
		}
		break;
	default:
		ck_abort_msg("field_get called with g_field_step == %i",
		             (int)g_field_step);
	}
	mark_point();

	return 0;
}


static int
FormGet(struct mg_connection *conn, void *cbdata)
{
	const struct mg_request_info *req_info = mg_get_request_info(conn);
	int ret;
	struct mg_form_data_handler fdh = {field_found, field_get, NULL, NULL};

	(void)cbdata;

	ck_assert(req_info != NULL);

	mg_printf(conn, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\n");
	fdh.user_data = (void *)&g_field_found_return;

	/* Call the form handler */
	g_field_step = 0;
	g_field_found_return = MG_FORM_FIELD_STORAGE_GET;
	ret = mg_handle_form_request(conn, &fdh);
	g_field_found_return = -888;
	ck_assert_int_eq(ret, 22);
	ck_assert_int_eq(g_field_step, 22);
	mg_printf(conn, "%i\r\n", ret);
	g_field_step = 1000;

	mark_point();

	return 1;
}


static int
FormStore(struct mg_connection *conn,
          void *cbdata,
          int ret_expected,
          int field_step_expected)
{
	const struct mg_request_info *req_info = mg_get_request_info(conn);
	int ret;
	struct mg_form_data_handler fdh = {field_found,
	                                   field_get,
	                                   field_store,
	                                   NULL};

	(void)cbdata;

	ck_assert(req_info != NULL);

	mg_printf(conn, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\n");
	fdh.user_data = (void *)&g_field_found_return;

	/* Call the form handler */
	g_field_step = 100;
	g_field_found_return = MG_FORM_FIELD_STORAGE_STORE;
	ret = mg_handle_form_request(conn, &fdh);
	ck_assert_int_eq(ret, ret_expected);
	ck_assert_int_eq(g_field_step, field_step_expected);
	mg_printf(conn, "%i\r\n", ret);
	g_field_step = 1000;

	mark_point();

	return 1;
}


static int
FormStore1(struct mg_connection *conn, void *cbdata)
{
	mark_point();
	return FormStore(conn, cbdata, 3, 101);
}


static int
FormStore2(struct mg_connection *conn, void *cbdata)
{
	mark_point();
	return FormStore(conn, cbdata, 4, 102);
}


static void
send_chunk_stringl(struct mg_connection *conn,
                   const char *chunk,
                   unsigned int chunk_len)
{
	char lenbuf[16];
	size_t lenbuf_len;
	int ret;

	mark_point();

	/* First store the length information in a text buffer. */
	sprintf(lenbuf, "%x\r\n", chunk_len);
	lenbuf_len = strlen(lenbuf);

	/* Then send length information, chunk and terminating \r\n. */
	ret = mg_write(conn, lenbuf, lenbuf_len);
	ck_assert_int_eq(ret, (int)lenbuf_len);

	ret = mg_write(conn, chunk, chunk_len);
	ck_assert_int_eq(ret, (int)chunk_len);

	ret = mg_write(conn, "\r\n", 2);
	ck_assert_int_eq(ret, 2);
}


static void
send_chunk_string(struct mg_connection *conn, const char *chunk)
{
	mark_point();
	send_chunk_stringl(conn, chunk, (unsigned int)strlen(chunk));
	mark_point();
}


START_TEST(test_handle_form)
{
	struct mg_context *ctx;
	struct mg_connection *client_conn;
	const struct mg_response_info *client_ri;
	const char *OPTIONS[8];
	const char *opt;
	int opt_idx = 0;
	char ebuf[1024];
	const char *multipart_body;
	const char *boundary;
	size_t body_len, body_sent, chunk_len, bound_len;
	int sleep_cnt;

	mark_point();

	memset((void *)OPTIONS, 0, sizeof(OPTIONS));
	OPTIONS[opt_idx++] = "listening_ports";
	OPTIONS[opt_idx++] = "8884";
	ck_assert_int_le(opt_idx, (int)(sizeof(OPTIONS) / sizeof(OPTIONS[0])));
	ck_assert(OPTIONS[sizeof(OPTIONS) / sizeof(OPTIONS[0]) - 1] == NULL);
	ck_assert(OPTIONS[sizeof(OPTIONS) / sizeof(OPTIONS[0]) - 2] == NULL);

	ctx = test_mg_start(NULL, &g_ctx, OPTIONS);

	ck_assert(ctx != NULL);
	g_ctx = ctx;

	opt = mg_get_option(ctx, "listening_ports");
	ck_assert_str_eq(opt, "8884");

	mg_set_request_handler(ctx, "/handle_form", FormGet, NULL);
	mg_set_request_handler(ctx, "/handle_form_store", FormStore1, NULL);
	mg_set_request_handler(ctx, "/handle_form_store2", FormStore2, NULL);

	test_sleep(1);

	/* Handle form: "GET" */
	client_conn = mg_download("localhost",
	                          8884,
	                          0,
	                          ebuf,
	                          sizeof(ebuf),
	                          "%s",
	                          "GET /handle_form"
	                          "?textin=text&passwordin=&radio1=val1"
	                          "&radio2=val1&check1=val1&numberin=1"
	                          "&datein=1.1.2016&colorin=%2380ff00"
	                          "&rangein=3&monthin=&weekin=&timein="
	                          "&datetimen=&datetimelocalin=&emailin="
	                          "&searchin=&telin=&urlin=&filein="
	                          "&filesin=&selectin=opt1"
	                          "&message=Text+area+default+text. "
	                          "HTTP/1.0\r\n"
	                          "Host: localhost:8884\r\n"
	                          "Connection: close\r\n\r\n");
	ck_assert(client_conn != NULL);
	for (sleep_cnt = 0; sleep_cnt < 30; sleep_cnt++) {
		test_sleep(1);
		if (g_field_step == 1000) {
			break;
		}
	}
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	mg_close_connection(client_conn);

	/* Handle form: "POST x-www-form-urlencoded" */
	client_conn =
	    mg_download("localhost",
	                8884,
	                0,
	                ebuf,
	                sizeof(ebuf),
	                "%s",
	                "POST /handle_form HTTP/1.1\r\n"
	                "Host: localhost:8884\r\n"
	                "Connection: close\r\n"
	                "Content-Type: application/x-www-form-urlencoded\r\n"
	                "Content-Length: 263\r\n"
	                "\r\n"
	                "textin=text&passwordin=&radio1=val1&radio2=val1"
	                "&check1=val1&numberin=1&datein=1.1.2016"
	                "&colorin=%2380ff00&rangein=3&monthin=&weekin="
	                "&timein=&datetimen=&datetimelocalin=&emailin="
	                "&searchin=&telin=&urlin=&filein=&filesin="
	                "&selectin=opt1&message=Text+area+default+text.");
	ck_assert(client_conn != NULL);
	for (sleep_cnt = 0; sleep_cnt < 30; sleep_cnt++) {
		test_sleep(1);
		if (g_field_step == 1000) {
			break;
		}
	}
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	mg_close_connection(client_conn);

	/* Handle form: "POST multipart/form-data" */
	multipart_body =
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"textin\"\r\n"
	    "\r\n"
	    "text\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"passwordin\"\r\n"
	    "\r\n"
	    "\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"radio1\"\r\n"
	    "\r\n"
	    "val1\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=radio2\r\n"
	    "\r\n"
	    "val1\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"check1\"\r\n"
	    "\r\n"
	    "val1\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"numberin\"\r\n"
	    "\r\n"
	    "1\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"datein\"\r\n"
	    "\r\n"
	    "1.1.2016\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"colorin\"\r\n"
	    "\r\n"
	    "#80ff00\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"rangein\"\r\n"
	    "\r\n"
	    "3\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"monthin\"\r\n"
	    "\r\n"
	    "\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"weekin\"\r\n"
	    "\r\n"
	    "\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"timein\"\r\n"
	    "\r\n"
	    "\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"datetimen\"\r\n"
	    "\r\n"
	    "\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"datetimelocalin\"\r\n"
	    "\r\n"
	    "\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"emailin\"\r\n"
	    "\r\n"
	    "\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"searchin\"\r\n"
	    "\r\n"
	    "\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"telin\"\r\n"
	    "\r\n"
	    "\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"urlin\"\r\n"
	    "\r\n"
	    "\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"filein\"; filename=\"\"\r\n"
	    "Content-Type: application/octet-stream\r\n"
	    "\r\n"
	    "\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=filesin; filename=\r\n"
	    "Content-Type: application/octet-stream\r\n"
	    "\r\n"
	    "\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"selectin\"\r\n"
	    "\r\n"
	    "opt1\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388\r\n"
	    "Content-Disposition: form-data; name=\"message\"\r\n"
	    "\r\n"
	    "Text area default text.\r\n"
	    "--multipart-form-data-boundary--see-RFC-2388--\r\n";
	body_len = strlen(multipart_body);
	ck_assert_uint_eq(body_len, 2368); /* not required */

	client_conn =
	    mg_download("localhost",
	                8884,
	                0,
	                ebuf,
	                sizeof(ebuf),
	                "POST /handle_form HTTP/1.1\r\n"
	                "Host: localhost:8884\r\n"
	                "Connection: close\r\n"
	                "Content-Type: multipart/form-data; "
	                "boundary=multipart-form-data-boundary--see-RFC-2388\r\n"
	                "Content-Length: %u\r\n"
	                "\r\n%s",
	                (unsigned int)body_len,
	                multipart_body);

	ck_assert(client_conn != NULL);
	for (sleep_cnt = 0; sleep_cnt < 30; sleep_cnt++) {
		test_sleep(1);
		if (g_field_step == 1000) {
			break;
		}
	}
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	mg_close_connection(client_conn);


	/* Handle form: "POST multipart/form-data" with chunked transfer encoding */
	client_conn =
	    mg_download("localhost",
	                8884,
	                0,
	                ebuf,
	                sizeof(ebuf),
	                "%s",
	                "POST /handle_form HTTP/1.1\r\n"
	                "Host: localhost:8884\r\n"
	                "Connection: close\r\n"
	                "Content-Type: multipart/form-data; "
	                "boundary=multipart-form-data-boundary--see-RFC-2388\r\n"
	                "Transfer-Encoding: chunked\r\n"
	                "\r\n");

	ck_assert(client_conn != NULL);

	body_len = strlen(multipart_body);
	chunk_len = 1;
	body_sent = 0;
	while (body_len > body_sent) {
		if (chunk_len > (body_len - body_sent)) {
			chunk_len = body_len - body_sent;
		}
		ck_assert_int_gt((int)chunk_len, 0);
		mg_printf(client_conn, "%x\r\n", (unsigned int)chunk_len);
		mg_write(client_conn, multipart_body + body_sent, chunk_len);
		mg_printf(client_conn, "\r\n");
		body_sent += chunk_len;
		chunk_len = (chunk_len % 40) + 1;
	}
	mg_printf(client_conn, "0\r\n\r\n");

	for (sleep_cnt = 0; sleep_cnt < 30; sleep_cnt++) {
		test_sleep(1);
		if (g_field_step == 1000) {
			break;
		}
	}
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	mg_close_connection(client_conn);

	/* Handle form: "POST multipart/form-data" with chunked transfer
	 * encoding, using a quoted boundary string */
	client_conn = mg_download(
	    "localhost",
	    8884,
	    0,
	    ebuf,
	    sizeof(ebuf),
	    "%s",
	    "POST /handle_form HTTP/1.1\r\n"
	    "Host: localhost:8884\r\n"
	    "Connection: close\r\n"
	    "Content-Type: multipart/form-data; "
	    "boundary=\"multipart-form-data-boundary--see-RFC-2388\"\r\n"
	    "Transfer-Encoding: chunked\r\n"
	    "\r\n");

	ck_assert(client_conn != NULL);

	body_len = strlen(multipart_body);
	chunk_len = 1;
	body_sent = 0;
	while (body_len > body_sent) {
		if (chunk_len > (body_len - body_sent)) {
			chunk_len = body_len - body_sent;
		}
		ck_assert_int_gt((int)chunk_len, 0);
		mg_printf(client_conn, "%x\r\n", (unsigned int)chunk_len);
		mg_write(client_conn, multipart_body + body_sent, chunk_len);
		mg_printf(client_conn, "\r\n");
		body_sent += chunk_len;
		chunk_len = (chunk_len % 40) + 1;
	}
	mg_printf(client_conn, "0\r\n\r\n");

	for (sleep_cnt = 0; sleep_cnt < 30; sleep_cnt++) {
		test_sleep(1);
		if (g_field_step == 1000) {
			break;
		}
	}
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	mg_close_connection(client_conn);


	/* Now test form_store */

	/* First test with GET */
	client_conn = mg_download("localhost",
	                          8884,
	                          0,
	                          ebuf,
	                          sizeof(ebuf),
	                          "%s",
	                          "GET /handle_form_store"
	                          "?storeme=storetest"
	                          "&continue_field_handler=ignore"
	                          "&break_field_handler=abort"
	                          "&dontread=xyz "
	                          "HTTP/1.0\r\n"
	                          "Host: localhost:8884\r\n"
	                          "Connection: close\r\n\r\n");

	ck_assert(client_conn != NULL);

	for (sleep_cnt = 0; sleep_cnt < 30; sleep_cnt++) {
		test_sleep(1);
		if (g_field_step == 1000) {
			break;
		}
	}
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	mg_close_connection(client_conn);


	/* Handle form: "POST x-www-form-urlencoded", chunked, store */
	client_conn =
	    mg_download("localhost",
	                8884,
	                0,
	                ebuf,
	                sizeof(ebuf),
	                "%s",
	                "POST /handle_form_store HTTP/1.0\r\n"
	                "Host: localhost:8884\r\n"
	                "Connection: close\r\n"
	                "Content-Type: application/x-www-form-urlencoded\r\n"
	                "Transfer-Encoding: chunked\r\n"
	                "\r\n");
	ck_assert(client_conn != NULL);

	send_chunk_string(client_conn, "storeme=store");
	send_chunk_string(client_conn, "test&");
	send_chunk_string(client_conn, "continue_field_handler=ignore");
	send_chunk_string(client_conn, "&br");
	test_sleep(1);
	send_chunk_string(client_conn, "eak_field_handler=abort&");
	send_chunk_string(client_conn, "dontread=xyz");
	mg_printf(client_conn, "0\r\n\r\n");

	for (sleep_cnt = 0; sleep_cnt < 30; sleep_cnt++) {
		test_sleep(1);
		if (g_field_step == 1000) {
			break;
		}
	}
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	mg_close_connection(client_conn);

	/* Handle form: "POST multipart/form-data", chunked, store */
	client_conn =
	    mg_download("localhost",
	                8884,
	                0,
	                ebuf,
	                sizeof(ebuf),
	                "%s",
	                "POST /handle_form_store HTTP/1.0\r\n"
	                "Host: localhost:8884\r\n"
	                "Connection: close\r\n"
	                "Content-Type: multipart/form-data; "
	                "boundary=multipart-form-data-boundary--see-RFC-2388\r\n"
	                "Transfer-Encoding: chunked\r\n"
	                "\r\n");
	ck_assert(client_conn != NULL);

	send_chunk_string(client_conn, "--multipart-form-data-boundary");
	send_chunk_string(client_conn, "--see-RFC-2388\r\n");
	send_chunk_string(client_conn, "Content-Disposition: form-data; ");
	send_chunk_string(client_conn, "name=\"storeme\"\r\n");
	send_chunk_string(client_conn, "\r\n");
	send_chunk_string(client_conn, "storetest\r\n");

	send_chunk_string(client_conn, "--multipart-form-data-boundary-");
	send_chunk_string(client_conn, "-see-RFC-2388\r\n");
	send_chunk_string(client_conn, "Content-Disposition: form-data; ");
	send_chunk_string(client_conn, "name=\"continue_field_handler\"\r\n");
	send_chunk_string(client_conn, "\r\n");
	send_chunk_string(client_conn, "ignore\r\n");

	send_chunk_string(client_conn, "--multipart-form-data-boundary-");
	send_chunk_string(client_conn, "-see-RFC-2388\r\n");
	send_chunk_string(client_conn, "Content-Disposition: form-data; ");
	send_chunk_string(client_conn, "name=\"break_field_handler\"\r\n");
	send_chunk_string(client_conn, "\r\n");
	send_chunk_string(client_conn, "abort\r\n");

	send_chunk_string(client_conn, "--multipart-form-data-boundary-");
	send_chunk_string(client_conn, "-see-RFC-2388\r\n");
	send_chunk_string(client_conn, "Content-Disposition: form-data; ");
	send_chunk_string(client_conn, "name=\"dontread\"\r\n");
	send_chunk_string(client_conn, "\r\n");
	send_chunk_string(client_conn, "xyz\r\n");
	send_chunk_string(client_conn, "--multipart-form-data-boundary");
	send_chunk_string(client_conn, "--see-RFC-2388--\r\n");
	mg_printf(client_conn, "0\r\n\r\n");

	for (sleep_cnt = 0; sleep_cnt < 30; sleep_cnt++) {
		test_sleep(1);
		if (g_field_step == 1000) {
			break;
		}
	}
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	mg_close_connection(client_conn);


	/* Handle form: "POST multipart/form-data", chunked, store, with files */
	client_conn =
	    mg_download("localhost",
	                8884,
	                0,
	                ebuf,
	                sizeof(ebuf),
	                "%s",
	                "POST /handle_form_store2 HTTP/1.0\r\n"
	                "Host: localhost:8884\r\n"
	                "Connection: close\r\n"
	                "Content-Type: multipart/form-data; "
	                "boundary=multipart-form-data-boundary--see-RFC-2388\r\n"
	                "Transfer-Encoding: chunked\r\n"
	                "\r\n");
	ck_assert(client_conn != NULL);

	boundary = "--multipart-form-data-boundary--see-RFC-2388\r\n";

	send_chunk_string(client_conn, boundary);
	send_chunk_string(client_conn, "Content-Disposition: form-data; ");
	send_chunk_string(client_conn, "name=\"storeme\"\r\n");
	send_chunk_string(client_conn, "\r\n");
	send_chunk_string(client_conn, "storetest\r\n");

	send_chunk_string(client_conn, boundary);
	send_chunk_string(client_conn, "Content-Disposition: form-data; ");
	send_chunk_string(client_conn, "name=\"continue_field_handler\";");
	send_chunk_string(client_conn, "filename=\"file_ignored.txt\"\r\n");
	send_chunk_string(client_conn, "Content-Type: ");
	send_chunk_string(client_conn, "application/octet-stream\r\n");
	send_chunk_string(client_conn, "X-Ignored-Header: xyz\r\n");
	send_chunk_string(client_conn, "\r\n");

	/* send some kilobyte of data */
	/* sending megabytes to localhost does not always work in CI test
	 * environments (depending on the network stack) */
	body_sent = 0;
	bound_len = strlen(boundary);
	do {
		send_chunk_string(client_conn, "ignore\r\n");
		body_sent += 8;
		/* send some strings that are almost boundaries */
		for (chunk_len = 1; chunk_len < bound_len; chunk_len++) {
			/* chunks from 1 byte to strlen(boundary)-1 */
			send_chunk_stringl(client_conn, boundary, (unsigned int)chunk_len);
			body_sent += chunk_len;
		}
	} while (body_sent < 8 * 1024);
	send_chunk_string(client_conn, "\r\n");

	send_chunk_string(client_conn, boundary);
	send_chunk_string(client_conn, "Content-Disposition: form-data; ");
	send_chunk_string(client_conn, "name=\"file2store\";");
	send_chunk_string(client_conn, "filename=\"myfile.txt\"\r\n");
	send_chunk_string(client_conn, "Content-Type: ");
	send_chunk_string(client_conn, "application/octet-stream\r\n");
	send_chunk_string(client_conn, "X-Ignored-Header: xyz\r\n");
	send_chunk_string(client_conn, "\r\n");
	for (body_sent = 0; (int)body_sent < (int)myfile_content_rep; body_sent++) {
		send_chunk_string(client_conn, myfile_content);
	}
	send_chunk_string(client_conn, "\r\n");

	send_chunk_string(client_conn, boundary);
	send_chunk_string(client_conn, "Content-Disposition: form-data; ");
	send_chunk_string(client_conn, "name=\"break_field_handler\"\r\n");
	send_chunk_string(client_conn, "\r\n");
	send_chunk_string(client_conn, "abort\r\n");

	send_chunk_string(client_conn, boundary);
	send_chunk_string(client_conn, "Content-Disposition: form-data; ");
	send_chunk_string(client_conn, "name=\"dontread\"\r\n");
	send_chunk_string(client_conn, "\r\n");
	send_chunk_string(client_conn, "xyz\r\n");
	send_chunk_string(client_conn, "--multipart-form-data-boundary");
	send_chunk_string(client_conn, "--see-RFC-2388--\r\n");
	mg_printf(client_conn, "0\r\n\r\n");

	for (sleep_cnt = 0; sleep_cnt < 30; sleep_cnt++) {
		test_sleep(1);
		if (g_field_step == 1000) {
			break;
		}
	}
	client_ri = mg_get_response_info(client_conn);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);
	mg_close_connection(client_conn);


	/* Close the server */
	g_ctx = NULL;
	test_mg_stop(ctx);
	mark_point();
}
END_TEST


START_TEST(test_http_auth)
{
#if !defined(NO_FILES)
	const char *OPTIONS[] = {
		"document_root",
		".",
		"listening_ports",
		"8080",
#if !defined(NO_CACHING)
		"static_file_max_age",
		"0",
#endif
		"put_delete_auth_file",
		"put_delete_auth_file.csv",
		NULL,
	};

	struct mg_context *ctx;
	struct mg_connection *client_conn;
	char client_err[256], nonce[256];
	const struct mg_response_info *client_ri;
	int client_res;
	FILE *f;
	const char *passwd_file = ".htpasswd";
	const char *test_file = "test_http_auth.test_file.txt";
	const char *test_content = "test_http_auth test_file content";
	const char *domain;
	const char *doc_root;
	const char *auth_request;
	const char *str;
	size_t len;
	int i;
	char HA1[256], HA2[256];
	char HA1_md5_buf[33], HA2_md5_buf[33], HA_md5_buf[33];
	char *HA1_md5_ret, *HA2_md5_ret, *HA_md5_ret;
	const char *nc = "00000001";
	const char *cnonce = "6789ABCD";

	mark_point();

	/* Start with default options */
	ctx = test_mg_start(NULL, NULL, OPTIONS);

	ck_assert(ctx != NULL);
	domain = mg_get_option(ctx, "authentication_domain");
	ck_assert(domain != NULL);
	len = strlen(domain);
	ck_assert_uint_gt(len, 0);
	ck_assert_uint_lt(len, 64);
	doc_root = mg_get_option(ctx, "document_root");
	ck_assert_str_eq(doc_root, ".");

	/* Create a default file in the document root */
	f = fopen(test_file, "w");
	if (f) {
		fprintf(f, "%s", test_content);
		fclose(f);
	} else {
		ck_abort_msg("Cannot create file %s", test_file);
	}

	(void)remove(passwd_file);
	(void)remove("put_delete_auth_file.csv");

	client_res = mg_modify_passwords_file("put_delete_auth_file.csv",
	                                      domain,
	                                      "admin",
	                                      "adminpass");
	ck_assert_int_eq(client_res, 1);

	/* Read file before a .htpasswd file has been created */
	memset(client_err, 0, sizeof(client_err));
	client_conn =
	    mg_connect_client("127.0.0.1", 8080, 0, client_err, sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "GET /%s HTTP/1.0\r\n\r\n", test_file);
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

	ck_assert_int_eq(client_ri->status_code, 200);
	client_res = (int)mg_read(client_conn, client_err, sizeof(client_err));
	ck_assert_int_gt(client_res, 0);
	ck_assert_int_le(client_res, sizeof(client_err));
	ck_assert_str_eq(client_err, test_content);
	mg_close_connection(client_conn);

	test_sleep(1);

	/* Create a .htpasswd file */
	client_res = mg_modify_passwords_file(passwd_file, domain, "user", "pass");
	ck_assert_int_eq(client_res, 1);

	client_res = mg_modify_passwords_file(NULL, domain, "user", "pass");
	ck_assert_int_eq(client_res, 0); /* Filename is required */

	test_sleep(1);

	/* Repeat test after .htpasswd is created */
	memset(client_err, 0, sizeof(client_err));
	client_conn =
	    mg_connect_client("127.0.0.1", 8080, 0, client_err, sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "GET /%s HTTP/1.0\r\n\r\n", test_file);
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

	ck_assert_int_eq(client_ri->status_code, 401);

	auth_request = NULL;
	for (i = 0; i < client_ri->num_headers; i++) {
		if (!mg_strcasecmp(client_ri->http_headers[i].name,
		                   "WWW-Authenticate")) {
			ck_assert_ptr_eq(auth_request, NULL);
			auth_request = client_ri->http_headers[i].value;
			ck_assert_ptr_ne(auth_request, NULL);
		}
	}
	ck_assert_ptr_ne(auth_request, NULL);
	str = "Digest qop=\"auth\", realm=\"";
	len = strlen(str);
	ck_assert(!mg_strncasecmp(auth_request, str, len));
	ck_assert(!strncmp(auth_request + len, domain, strlen(domain)));
	len += strlen(domain);
	str = "\", nonce=\"";
	ck_assert(!strncmp(auth_request + len, str, strlen(str)));
	len += strlen(str);
	str = strchr(auth_request + len, '\"');
	ck_assert_ptr_ne(str, NULL);
	ck_assert_ptr_ne(str, auth_request + len);
	/* nonce is from including (auth_request + len) to excluding (str) */
	ck_assert_int_gt((ptrdiff_t)(str) - (ptrdiff_t)(auth_request + len), 0);
	ck_assert_int_lt((ptrdiff_t)(str) - (ptrdiff_t)(auth_request + len),
	                 (ptrdiff_t)sizeof(nonce));
	memset(nonce, 0, sizeof(nonce));
	memcpy(nonce,
	       auth_request + len,
	       (size_t)((ptrdiff_t)(str) - (ptrdiff_t)(auth_request + len)));
	memset(HA1, 0, sizeof(HA1));
	memset(HA2, 0, sizeof(HA2));
	memset(HA1_md5_buf, 0, sizeof(HA1_md5_buf));
	memset(HA2_md5_buf, 0, sizeof(HA2_md5_buf));
	memset(HA_md5_buf, 0, sizeof(HA_md5_buf));

	sprintf(HA1, "%s:%s:%s", "user", domain, "pass");
	sprintf(HA2, "%s:/%s", "GET", test_file);
	HA1_md5_ret = mg_md5(HA1_md5_buf, HA1, NULL);
	HA2_md5_ret = mg_md5(HA2_md5_buf, HA2, NULL);

	ck_assert_ptr_eq(HA1_md5_ret, HA1_md5_buf);
	ck_assert_ptr_eq(HA2_md5_ret, HA2_md5_buf);

	HA_md5_ret = mg_md5(HA_md5_buf, "user", ":", domain, ":", "pass", NULL);
	ck_assert_ptr_eq(HA_md5_ret, HA_md5_buf);
	ck_assert_str_eq(HA1_md5_ret, HA_md5_buf);

	HA_md5_ret = mg_md5(HA_md5_buf, "GET", ":", "/", test_file, NULL);
	ck_assert_ptr_eq(HA_md5_ret, HA_md5_buf);
	ck_assert_str_eq(HA2_md5_ret, HA_md5_buf);

	HA_md5_ret = mg_md5(HA_md5_buf,
	                    HA1_md5_buf,
	                    ":",
	                    nonce,
	                    ":",
	                    nc,
	                    ":",
	                    cnonce,
	                    ":",
	                    "auth",
	                    ":",
	                    HA2_md5_buf,
	                    NULL);
	ck_assert_ptr_eq(HA_md5_ret, HA_md5_buf);

	mg_close_connection(client_conn);

	/* Retry with authorization */
	memset(client_err, 0, sizeof(client_err));
	client_conn =
	    mg_connect_client("127.0.0.1", 8080, 0, client_err, sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "GET /%s HTTP/1.0\r\n", test_file);
	mg_printf(client_conn,
	          "Authorization: Digest "
	          "username=\"%s\", "
	          "realm=\"%s\", "
	          "nonce=\"%s\", "
	          "uri=\"/%s\", "
	          "qop=auth, "
	          "nc=%s, "
	          "cnonce=\"%s\", "
	          "response=\"%s\"\r\n\r\n",
	          "user",
	          domain,
	          nonce,
	          test_file,
	          nc,
	          cnonce,
	          HA_md5_buf);
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

	ck_assert_int_eq(client_ri->status_code, 200);
	client_res = (int)mg_read(client_conn, client_err, sizeof(client_err));
	ck_assert_int_gt(client_res, 0);
	ck_assert_int_le(client_res, sizeof(client_err));
	ck_assert_str_eq(client_err, test_content);
	mg_close_connection(client_conn);

	test_sleep(1);

	/* Retry DELETE with authorization of a user not authorized for DELETE */
	memset(client_err, 0, sizeof(client_err));
	client_conn =
	    mg_connect_client("127.0.0.1", 8080, 0, client_err, sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "DELETE /%s HTTP/1.0\r\n", test_file);
	mg_printf(client_conn,
	          "Authorization: Digest "
	          "username=\"%s\", "
	          "realm=\"%s\", "
	          "nonce=\"%s\", "
	          "uri=\"/%s\", "
	          "qop=auth, "
	          "nc=%s, "
	          "cnonce=\"%s\", "
	          "response=\"%s\"\r\n\r\n",
	          "user",
	          domain,
	          nonce,
	          test_file,
	          nc,
	          cnonce,
	          HA_md5_buf);
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

	ck_assert_int_eq(client_ri->status_code, 401);
	mg_close_connection(client_conn);

	test_sleep(1);

	/* Remove the user from the .htpasswd file again */
	client_res = mg_modify_passwords_file(passwd_file, domain, "user", NULL);
	ck_assert_int_eq(client_res, 1);

	test_sleep(1);


	/* Try to access the file again. Expected: 401 error */
	memset(client_err, 0, sizeof(client_err));
	client_conn =
	    mg_connect_client("127.0.0.1", 8080, 0, client_err, sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "GET /%s HTTP/1.0\r\n\r\n", test_file);
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

	ck_assert_int_eq(client_ri->status_code, 401);
	mg_close_connection(client_conn);

	test_sleep(1);


	/* Now remove the password file */
	(void)remove(passwd_file);
	test_sleep(1);


	/* Access to the file must work like before */
	memset(client_err, 0, sizeof(client_err));
	client_conn =
	    mg_connect_client("127.0.0.1", 8080, 0, client_err, sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "GET /%s HTTP/1.0\r\n\r\n", test_file);
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

	ck_assert_int_eq(client_ri->status_code, 200);
	client_res = (int)mg_read(client_conn, client_err, sizeof(client_err));
	ck_assert_int_gt(client_res, 0);
	ck_assert_int_le(client_res, sizeof(client_err));
	ck_assert_str_eq(client_err, test_content);
	mg_close_connection(client_conn);

	test_sleep(1);


	/* Stop the server and clean up */
	test_mg_stop(ctx);
	(void)remove(test_file);
	(void)remove(passwd_file);
	(void)remove("put_delete_auth_file.csv");

#endif
	mark_point();
}
END_TEST


START_TEST(test_keep_alive)
{
	struct mg_context *ctx;
	const char *OPTIONS[] =
	{ "listening_ports",
	  "8081",
	  "request_timeout_ms",
	  "10000",
	  "enable_keep_alive",
	  "yes",
#if !defined(NO_FILES)
	  "document_root",
	  ".",
	  "enable_directory_listing",
	  "no",
#endif
	  NULL };

	struct mg_connection *client_conn;
	char client_err[256];
	const struct mg_response_info *client_ri;
	int client_res, i;
	const char *connection_header;

	mark_point();

	ctx = test_mg_start(NULL, NULL, OPTIONS);

	ck_assert(ctx != NULL);

	/* HTTP 1.1 GET request */
	memset(client_err, 0, sizeof(client_err));
	client_conn =
	    mg_connect_client("127.0.0.1", 8081, 0, client_err, sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn,
	          "GET / HTTP/1.1\r\nHost: "
	          "localhost:8081\r\nConnection: keep-alive\r\n\r\n");
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

#if defined(NO_FILES)
	ck_assert_int_eq(client_ri->status_code, 404);
#else
	ck_assert_int_eq(client_ri->status_code, 403);
#endif

	connection_header = 0;
	for (i = 0; i < client_ri->num_headers; i++) {
		if (!mg_strcasecmp(client_ri->http_headers[i].name, "Connection")) {
			ck_assert_ptr_eq(connection_header, NULL);
			connection_header = client_ri->http_headers[i].value;
			ck_assert_ptr_ne(connection_header, NULL);
		}
	}
	/* Error replies will close the connection, even if keep-alive is set. */
	ck_assert_ptr_ne(connection_header, NULL);
	ck_assert_str_eq(connection_header, "close");
	mg_close_connection(client_conn);

	test_sleep(1);

	/* TODO: request a file and keep alive
	 * (will only work if NO_FILES is not set). */

	/* Stop the server and clean up */
	test_mg_stop(ctx);

	mark_point();
}
END_TEST


START_TEST(test_error_handling)
{
	struct mg_context *ctx;
	FILE *f;

	char bad_thread_num[32] = "badnumber";

	struct mg_callbacks callbacks;
	char errmsg[256];

	struct mg_connection *client_conn;
	char client_err[256];
	const struct mg_response_info *client_ri;
	int client_res, i;

	const char *OPTIONS[32];
	int opt_cnt = 0;

	mark_point();

#if !defined(NO_FILES)
	OPTIONS[opt_cnt++] = "document_root";
	OPTIONS[opt_cnt++] = ".";
#endif
	OPTIONS[opt_cnt++] = "error_pages";
	OPTIONS[opt_cnt++] = "./";
	OPTIONS[opt_cnt++] = "listening_ports";
	OPTIONS[opt_cnt++] = "8080";
	OPTIONS[opt_cnt++] = "num_threads";
	OPTIONS[opt_cnt++] = bad_thread_num;
	OPTIONS[opt_cnt++] = "unknown_option";
	OPTIONS[opt_cnt++] = "unknown_option_value";
	OPTIONS[opt_cnt] = NULL;

	memset(&callbacks, 0, sizeof(callbacks));

	callbacks.log_message = log_msg_func;

	/* test with unknown option */
	memset(errmsg, 0, sizeof(errmsg));
	ctx = test_mg_start(&callbacks, (void *)errmsg, OPTIONS);

	/* Details of errmsg may vary, but it may not be empty */
	ck_assert_str_ne(errmsg, "");
	ck_assert(ctx == NULL);
	ck_assert_str_eq(errmsg, "Invalid option: unknown_option");

	/* Remove invalid option */
	for (i = 0; OPTIONS[i]; i++) {
		if (strstr(OPTIONS[i], "unknown_option")) {
			OPTIONS[i] = 0;
		}
	}

	/* Test with bad num_thread option */
	memset(errmsg, 0, sizeof(errmsg));
	ctx = test_mg_start(&callbacks, (void *)errmsg, OPTIONS);

	/* Details of errmsg may vary, but it may not be empty */
	ck_assert_str_ne(errmsg, "");
	ck_assert(ctx == NULL);
	ck_assert_str_eq(errmsg, "Invalid number of worker threads");

/* Set to a number - but use a number above the limit */
#ifdef MAX_WORKER_THREADS
	sprintf(bad_thread_num, "%u", MAX_WORKER_THREADS + 1);
#else
	sprintf(bad_thread_num, "%lu", 1000000000lu);
#endif

	/* Test with bad num_thread option */
	memset(errmsg, 0, sizeof(errmsg));
	ctx = test_mg_start(&callbacks, (void *)errmsg, OPTIONS);

	/* Details of errmsg may vary, but it may not be empty */
	ck_assert_str_ne(errmsg, "");
	ck_assert(ctx == NULL);
	ck_assert_str_eq(errmsg, "Too many worker threads");


	/* HTTP 1.0 GET request - server is not running */
	memset(client_err, 0, sizeof(client_err));
	client_conn =
	    mg_connect_client("127.0.0.1", 8080, 0, client_err, sizeof(client_err));
	ck_assert(client_conn == NULL);

	/* Error message detail may vary - it may not be empty and should contain
	 * some information "connect" failed */
	ck_assert_str_ne(client_err, "");
	ck_assert(strstr(client_err, "connect"));


	/* This time start the server with a valid configuration */
	sprintf(bad_thread_num, "%i", 1);
	memset(errmsg, 0, sizeof(errmsg));
	ctx = test_mg_start(&callbacks, (void *)errmsg, OPTIONS);

	ck_assert_str_eq(errmsg, "");
	ck_assert(ctx != NULL);


	/* Server is running now */
	test_sleep(1);

	/* Remove error files (in case they exist) */
	(void)remove("error.htm");
	(void)remove("error4xx.htm");
	(void)remove("error404.htm");


	/* Ask for something not existing - should get default 404 */
	memset(client_err, 0, sizeof(client_err));
	client_conn =
	    mg_connect_client("127.0.0.1", 8080, 0, client_err, sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "GET /something/not/existing HTTP/1.0\r\n\r\n");
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

	ck_assert_int_eq(client_ri->status_code, 404);
	mg_close_connection(client_conn);
	test_sleep(1);

	/* Create an error.htm file */
	f = fopen("error.htm", "wt");
	ck_assert(f != NULL);
	(void)fprintf(f, "err-all");
	(void)fclose(f);


	/* Ask for something not existing - should get error.htm */
	memset(client_err, 0, sizeof(client_err));
	client_conn =
	    mg_connect_client("127.0.0.1", 8080, 0, client_err, sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "GET /something/not/existing HTTP/1.0\r\n\r\n");
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

	ck_assert_int_eq(client_ri->status_code, 200);

	client_res = (int)mg_read(client_conn, client_err, sizeof(client_err));
	mg_close_connection(client_conn);
	ck_assert_int_eq(client_res, 7);
	client_err[8] = 0;
	ck_assert_str_eq(client_err, "err-all");
	test_sleep(1);

	/* Create an error4xx.htm file */
	f = fopen("error4xx.htm", "wt");
	ck_assert(f != NULL);
	(void)fprintf(f, "err-4xx");
	(void)fclose(f);


	/* Ask for something not existing - should get error4xx.htm */
	memset(client_err, 0, sizeof(client_err));
	client_conn =
	    mg_connect_client("127.0.0.1", 8080, 0, client_err, sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "GET /something/not/existing HTTP/1.0\r\n\r\n");
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

	ck_assert_int_eq(client_ri->status_code, 200);

	client_res = (int)mg_read(client_conn, client_err, sizeof(client_err));
	mg_close_connection(client_conn);
	ck_assert_int_eq(client_res, 7);
	client_err[8] = 0;
	ck_assert_str_eq(client_err, "err-4xx");
	test_sleep(1);

	/* Create an error404.htm file */
	f = fopen("error404.htm", "wt");
	ck_assert(f != NULL);
	(void)fprintf(f, "err-404");
	(void)fclose(f);


	/* Ask for something not existing - should get error404.htm */
	memset(client_err, 0, sizeof(client_err));
	client_conn =
	    mg_connect_client("127.0.0.1", 8080, 0, client_err, sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "GET /something/not/existing HTTP/1.0\r\n\r\n");
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

	ck_assert_int_eq(client_ri->status_code, 200);

	client_res = (int)mg_read(client_conn, client_err, sizeof(client_err));
	mg_close_connection(client_conn);
	ck_assert_int_eq(client_res, 7);
	client_err[8] = 0;
	ck_assert_str_eq(client_err, "err-404");
	test_sleep(1);


	/* Ask in a malformed way - should get error4xx.htm */
	memset(client_err, 0, sizeof(client_err));
	client_conn =
	    mg_connect_client("127.0.0.1", 8080, 0, client_err, sizeof(client_err));

	ck_assert_str_eq(client_err, "");
	ck_assert(client_conn != NULL);

	mg_printf(client_conn, "Gimme some file!\r\n\r\n");
	client_res =
	    mg_get_response(client_conn, client_err, sizeof(client_err), 10000);
	ck_assert_int_ge(client_res, 0);
	ck_assert_str_eq(client_err, "");
	client_ri = mg_get_response_info(client_conn);
	ck_assert(client_ri != NULL);

	ck_assert_int_eq(client_ri->status_code, 200);

	client_res = (int)mg_read(client_conn, client_err, sizeof(client_err));
	mg_close_connection(client_conn);
	ck_assert_int_eq(client_res, 7);
	client_err[8] = 0;
	ck_assert_str_eq(client_err, "err-4xx");
	test_sleep(1);


	/* Remove all error files created by this test */
	(void)remove("error.htm");
	(void)remove("error4xx.htm");
	(void)remove("error404.htm");


	/* Stop the server */
	test_mg_stop(ctx);


	/* HTTP 1.1 GET request - must not work, since server is already stopped  */
	memset(client_err, 0, sizeof(client_err));
	client_conn =
	    mg_connect_client("127.0.0.1", 8080, 0, client_err, sizeof(client_err));

	ck_assert(client_conn == NULL);
	ck_assert_str_ne(client_err, "");

	test_sleep(1);

	mark_point();
}
END_TEST


START_TEST(test_error_log_file)
{
	/* Server var */
	struct mg_context *ctx;
	const char *OPTIONS[32];
	int opt_cnt = 0;

	/* Client var */
	struct mg_connection *client;
	char client_err_buf[256];
	char client_data_buf[256];
	const struct mg_response_info *client_ri;

	/* File content check var */
	FILE *f;
	char buf[1024];
	int len, ok;

	mark_point();

	/* Set options and start server */
	OPTIONS[opt_cnt++] = "listening_ports";
	OPTIONS[opt_cnt++] = "8080";
	OPTIONS[opt_cnt++] = "error_log_file";
	OPTIONS[opt_cnt++] = "error.log";
	OPTIONS[opt_cnt++] = "access_log_file";
	OPTIONS[opt_cnt++] = "access.log";
#if !defined(NO_FILES)
	OPTIONS[opt_cnt++] = "document_root";
	OPTIONS[opt_cnt++] = ".";
#endif
	OPTIONS[opt_cnt] = NULL;

	ctx = test_mg_start(NULL, 0, OPTIONS);
	ck_assert(ctx != NULL);

	/* Remove log files (they may exist from previous incomplete runs of
	 * this test) */
	(void)remove("error.log");
	(void)remove("access.log");

	/* connect client */
	memset(client_err_buf, 0, sizeof(client_err_buf));
	memset(client_data_buf, 0, sizeof(client_data_buf));

	client = mg_download("127.0.0.1",
	                     8080,
	                     0,
	                     client_err_buf,
	                     sizeof(client_err_buf),
	                     "GET /not_existing_file.ext HTTP/1.0\r\n\r\n");

	ck_assert(ctx != NULL);
	ck_assert_str_eq(client_err_buf, "");

	client_ri = mg_get_response_info(client);

	/* Check status - should be 404 Not Found */
	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 404);

	/* Get body data (could exist, but does not have to) */
	len = mg_read(client, client_data_buf, sizeof(client_data_buf));
	ck_assert_int_ge(len, 0);

	/* Close the client connection */
	mg_close_connection(client);

	/* Stop the server */
	test_mg_stop(ctx);


	/* Check access.log */
	memset(buf, 0, sizeof(buf));
	f = fopen("access.log", "r");
	ck_assert_msg(f != NULL, "Cannot open access log file");
	ok = (NULL != fgets(buf, sizeof(buf) - 1, f));
	(void)fclose(f);
	ck_assert_msg(ok, "Cannot read access log file");
	len = (int)strlen(buf);
	ck_assert_int_gt(len, 0);
	ok = (NULL != strstr(buf, "not_existing_file.ext"));
	ck_assert_msg(ok, "Did not find uri in access log file");
	ok = (NULL != strstr(buf, "404"));
	ck_assert_msg(ok, "Did not find HTTP status code in access log file");

	/* Check error.log */
	memset(buf, 0, sizeof(buf));
	f = fopen("error.log", "r");
	if (f) {
		(void)fgets(buf, sizeof(buf) - 1, f);
		fclose(f);
	}
	ck_assert_msg(f == NULL,
	              "Should not create error log file on 404, but got [%s]",
	              buf);

	/* Remove log files */
	(void)remove("error.log");
	(void)remove("access.log");

	/* Start server with bad options */
	ck_assert_str_eq(OPTIONS[0], "listening_ports");
	OPTIONS[1] = "bad !"; /* no r or s in string */

	ctx = test_mg_start(NULL, 0, OPTIONS);
	ck_assert_msg(
	    ctx == NULL,
	    "Should not be able to start server with bad port configuration");

	/* Check access.log */
	memset(buf, 0, sizeof(buf));
	f = fopen("access.log", "r");
	if (f) {
		(void)fgets(buf, sizeof(buf) - 1, f);
		fclose(f);
	}
	ck_assert_msg(
	    f == NULL,
	    "Should not create access log file if start fails, but got [%s]",
	    buf);

	/* Check error.log */
	memset(buf, 0, sizeof(buf));
	f = fopen("error.log", "r");
	ck_assert_msg(f != NULL, "Cannot open access log file");
	ok = (NULL != fgets(buf, sizeof(buf) - 1, f));
	(void)fclose(f);
	ck_assert_msg(ok, "Cannot read access log file");
	len = (int)strlen(buf);
	ck_assert_int_gt(len, 0);
	ok = (NULL != strstr(buf, "port"));
	ck_assert_msg(ok, "Did not find port as error reason in error log file");


	/* Remove log files */
	(void)remove("error.log");
	(void)remove("access.log");

	mark_point();
}
END_TEST


static int
test_throttle_begin_request(struct mg_connection *conn)
{
	const struct mg_request_info *ri;
	long unsigned len = 1024 * 10;
	const char *block = "0123456789";
	unsigned long i, blocklen;

	ck_assert(conn != NULL);
	ri = mg_get_request_info(conn);
	ck_assert(ri != NULL);

	ck_assert_str_eq(ri->request_method, "GET");
	ck_assert_str_eq(ri->request_uri, "/throttle");
	ck_assert_str_eq(ri->local_uri, "/throttle");
	ck_assert_str_eq(ri->http_version, "1.0");
	ck_assert_str_eq(ri->query_string, "q");
	ck_assert_str_eq(ri->remote_addr, "127.0.0.1");

	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\n"
	          "Content-Length: %lu\r\n"
	          "Connection: close\r\n\r\n",
	          len);

	blocklen = (unsigned long)strlen(block);

	for (i = 0; i < len; i += blocklen) {
		mg_write(conn, block, blocklen);
	}

	mark_point();

	return 987; /* Not a valid HTTP response code,
	             * but it should be written to the log and passed to
	             * end_request. */
}


static void
test_throttle_end_request(const struct mg_connection *conn,
                          int reply_status_code)
{
	const struct mg_request_info *ri;

	ck_assert(conn != NULL);
	ri = mg_get_request_info(conn);
	ck_assert(ri != NULL);

	ck_assert_str_eq(ri->request_method, "GET");
	ck_assert_str_eq(ri->request_uri, "/throttle");
	ck_assert_str_eq(ri->local_uri, "/throttle");
	ck_assert_str_eq(ri->http_version, "1.0");
	ck_assert_str_eq(ri->query_string, "q");
	ck_assert_str_eq(ri->remote_addr, "127.0.0.1");

	ck_assert_int_eq(reply_status_code, 987);
}


START_TEST(test_throttle)
{
	/* Server var */
	struct mg_context *ctx;
	struct mg_callbacks callbacks;
	const char *OPTIONS[32];
	int opt_cnt = 0;

	/* Client var */
	struct mg_connection *client;
	char client_err_buf[256];
	char client_data_buf[256];
	const struct mg_response_info *client_ri;

	/* timing test */
	int r, data_read;
	time_t t0, t1;
	double dt;

	mark_point();


/* Set options and start server */
#if !defined(NO_FILES)
	OPTIONS[opt_cnt++] = "document_root";
	OPTIONS[opt_cnt++] = ".";
#endif
	OPTIONS[opt_cnt++] = "listening_ports";
	OPTIONS[opt_cnt++] = "8080";
	OPTIONS[opt_cnt++] = "throttle";
	OPTIONS[opt_cnt++] = "*=1k";
	OPTIONS[opt_cnt] = NULL;

	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.begin_request = test_throttle_begin_request;
	callbacks.end_request = test_throttle_end_request;

	ctx = test_mg_start(&callbacks, 0, OPTIONS);
	ck_assert(ctx != NULL);

	/* connect client */
	memset(client_err_buf, 0, sizeof(client_err_buf));
	memset(client_data_buf, 0, sizeof(client_data_buf));

	strcpy(client_err_buf, "reset-content");
	client = mg_download("127.0.0.1",
	                     8080,
	                     0,
	                     client_err_buf,
	                     sizeof(client_err_buf),
	                     "GET /throttle?q HTTP/1.0\r\n\r\n");

	ck_assert(ctx != NULL);
	ck_assert_str_eq(client_err_buf, "");

	client_ri = mg_get_response_info(client);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);

	ck_assert_int_eq(client_ri->content_length, 1024 * 10);

	data_read = 0;
	t0 = time(NULL);
	while (data_read < client_ri->content_length) {
		r = mg_read(client, client_data_buf, sizeof(client_data_buf));
		ck_assert_int_ge(r, 0);
		data_read += r;
	}
	t1 = time(NULL);
	dt = difftime(t1, t0) * 1000.0; /* Elapsed time in ms - in most systems
	                                 * only with second resolution */

	/* Time estimation: Data size is 10 kB, with 1 kB/s speed limit.
	 * The first block (1st kB) is transferred immediately, the second
	 * block (2nd kB) one second later, the third block (3rd kB) two
	 * seconds later, .. the last block (10th kB) nine seconds later.
	 * The resolution of time measurement using the "time" C library
	 * function is 1 second, so we should add +/- one second tolerance.
	 * Thus, download of 10 kB with 1 kB/s should not be faster than
	 * 8 seconds. */

	/* Check if there are at least 8 seconds */
	ck_assert_int_ge((int)dt, 8 * 1000);

	/* Nothing left to read */
	r = mg_read(client, client_data_buf, sizeof(client_data_buf));
	ck_assert_int_eq(r, 0);

	/* Close the client connection */
	mg_close_connection(client);

	/* Stop the server */
	test_mg_stop(ctx);

	mark_point();
}
END_TEST


START_TEST(test_init_library)
{
	unsigned f_avail, f_ret;

	mark_point();

	f_avail = mg_check_feature(0xFF);
	f_ret = mg_init_library(f_avail);
	ck_assert_uint_eq(f_ret, f_avail);
}
END_TEST


#define LARGE_FILE_SIZE (1024 * 1024 * 10)

static int
test_large_file_begin_request(struct mg_connection *conn)
{
	const struct mg_request_info *ri;
	long unsigned len = LARGE_FILE_SIZE;
	const char *block = "0123456789";
	uint64_t i;
	size_t blocklen;

	ck_assert(conn != NULL);
	ri = mg_get_request_info(conn);
	ck_assert(ri != NULL);

	ck_assert_str_eq(ri->request_method, "GET");
	ck_assert_str_eq(ri->http_version, "1.1");
	ck_assert_str_eq(ri->remote_addr, "127.0.0.1");
	ck_assert_ptr_eq(ri->query_string, NULL);
	ck_assert_ptr_ne(ri->local_uri, NULL);

	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\n"
	          "Content-Length: %lu\r\n"
	          "Connection: close\r\n\r\n",
	          len);

	blocklen = strlen(block);

	for (i = 0; i < len; i += blocklen) {
		mg_write(conn, block, blocklen);
	}

	mark_point();

	return 200;
}


START_TEST(test_large_file)
{
	/* Server var */
	struct mg_context *ctx;
	struct mg_callbacks callbacks;
	const char *OPTIONS[32];
	int opt_cnt = 0;
#if !defined(NO_SSL)
	const char *ssl_cert = locate_ssl_cert();
#endif
	char errmsg[256] = {0};

	/* Client var */
	struct mg_connection *client;
	char client_err_buf[256];
	char client_data_buf[256];
	const struct mg_response_info *client_ri;
	int64_t data_read;
	int r;
	int retry, retry_ok_cnt, retry_fail_cnt;

	mark_point();

/* Set options and start server */
#if !defined(NO_FILES)
	OPTIONS[opt_cnt++] = "document_root";
	OPTIONS[opt_cnt++] = ".";
#endif
#if defined(NO_SSL)
	OPTIONS[opt_cnt++] = "listening_ports";
	OPTIONS[opt_cnt++] = "8080";
#else
	OPTIONS[opt_cnt++] = "listening_ports";
	OPTIONS[opt_cnt++] = "8443s";
	OPTIONS[opt_cnt++] = "ssl_certificate";
	OPTIONS[opt_cnt++] = ssl_cert;
#ifdef __MACH__
	/* The Apple builds on Travis CI seem to have problems with TLS1.x
	 * Allow SSLv3 and TLS */
	OPTIONS[opt_cnt++] = "ssl_protocol_version";
	OPTIONS[opt_cnt++] = "2";
#else
	/* The Linux builds on Travis CI work fine with TLS1.2 */
	OPTIONS[opt_cnt++] = "ssl_protocol_version";
	OPTIONS[opt_cnt++] = "4";
#endif
	ck_assert(ssl_cert != NULL);
#endif
	OPTIONS[opt_cnt] = NULL;


	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.begin_request = test_large_file_begin_request;
	callbacks.log_message = log_msg_func;

	ctx = test_mg_start(&callbacks, (void *)errmsg, OPTIONS);
	ck_assert_str_eq(errmsg, "");
	ck_assert(ctx != NULL);

	/* Try downloading several times */
	retry_ok_cnt = 0;
	retry_fail_cnt = 0;
	for (retry = 0; retry < 3; retry++) {
		int fail = 0;
		/* connect client */
		memset(client_err_buf, 0, sizeof(client_err_buf));
		memset(client_data_buf, 0, sizeof(client_data_buf));

		client =
		    mg_download("127.0.0.1",
#if defined(NO_SSL)
		                8080,
		                0,
#else
		                8443,
		                1,
#endif
		                client_err_buf,
		                sizeof(client_err_buf),
		                "GET /large.file HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n");

		ck_assert(client != NULL);
		ck_assert_str_eq(client_err_buf, "");

		client_ri = mg_get_response_info(client);

		ck_assert(client_ri != NULL);
		ck_assert_int_eq(client_ri->status_code, 200);

		ck_assert_int_eq(client_ri->content_length, LARGE_FILE_SIZE);

		data_read = 0;
		while (data_read < client_ri->content_length) {
			r = mg_read(client, client_data_buf, sizeof(client_data_buf));
			if (r < 0) {
				fail = 1;
				break;
			};
			data_read += r;
		}

		/* Nothing left to read */
		r = mg_read(client, client_data_buf, sizeof(client_data_buf));
		if (fail) {
			ck_assert_int_eq(r, -1);
			retry_fail_cnt++;
		} else {
			ck_assert_int_eq(r, 0);
			retry_ok_cnt++;
		}

		/* Close the client connection */
		mg_close_connection(client);
	}

#if defined(_WIN32)
// TODO: Check this problem on AppVeyor
// ck_assert_int_le(retry_fail_cnt, 2);
// ck_assert_int_ge(retry_ok_cnt, 1);
#else
	ck_assert_int_eq(retry_fail_cnt, 0);
	ck_assert_int_eq(retry_ok_cnt, 3);
#endif

	/* Stop the server */
	test_mg_stop(ctx);

	mark_point();
}
END_TEST


static int test_mg_store_body_con_len = 20000;


static int
test_mg_store_body_put_delete_handler(struct mg_connection *conn, void *ignored)
{
	char path[4096] = {0};
	const struct mg_request_info *info = mg_get_request_info(conn);
	int64_t rc;

	(void)ignored;

	mark_point();

	sprintf(path, "./%s", info->local_uri);
	rc = mg_store_body(conn, path);

	ck_assert_int_eq(test_mg_store_body_con_len, rc);

	if (rc < 0) {
		mg_printf(conn,
		          "HTTP/1.1 500 Internal Server Error\r\n"
		          "Content-Type:text/plain;charset=UTF-8\r\n"
		          "Connection:close\r\n\r\n"
		          "%s (ret: %ld)\n",
		          path,
		          (long)rc);
		mg_close_connection(conn);

		/* Debug output for tests */
		printf("mg_store_body(%s) failed (ret: %ld)\n", path, (long)rc);

		return 500;
	}

	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\n"
	          "Content-Type:text/plain;charset=UTF-8\r\n"
	          "Connection:close\r\n\r\n"
	          "%s OK (%ld bytes saved)\n",
	          path,
	          (long)rc);
	mg_close_connection(conn);

	/* Debug output for tests */
	printf("mg_store_body(%s) OK (%ld bytes)\n", path, (long)rc);

	mark_point();

	return 200;
}


static int
test_mg_store_body_begin_request_callback(struct mg_connection *conn)
{
	const struct mg_request_info *info = mg_get_request_info(conn);

	mark_point();

	/* Debug output for tests */
	printf("test_mg_store_body_begin_request_callback called (%s)\n",
	       info->request_method);

	if ((strcmp(info->request_method, "PUT") == 0)
	    || (strcmp(info->request_method, "DELETE") == 0)) {
		return test_mg_store_body_put_delete_handler(conn, NULL);
	}

	mark_point();

	return 0;
}


START_TEST(test_mg_store_body)
{
	/* Client data */
	char client_err_buf[256];
	char client_data_buf[1024];
	struct mg_connection *client;
	const struct mg_response_info *client_ri;
	int r;
	char check_data[256];
	char *check_ptr;
	char errmsg[256] = {0};

	/* Server context handle */
	struct mg_context *ctx;
	struct mg_callbacks callbacks;
	const char *options[] = {
#if !defined(NO_FILES)
		"document_root",
		".",
#endif
#if !defined(NO_CACHING)
		"static_file_max_age",
		"0",
#endif
		"listening_ports",
		"127.0.0.1:8082",
		"num_threads",
		"1",
		NULL
	};

	mark_point();

	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.begin_request = test_mg_store_body_begin_request_callback;
	callbacks.log_message = log_msg_func;

	/* Initialize the library */
	mg_init_library(0);

	/* Start the server */
	ctx = mg_start(&callbacks, (void *)errmsg, options);
	ck_assert_str_eq(errmsg, "");
	ck_assert(ctx != NULL);

	/* Run the server for 15 seconds */
	test_sleep(15);

	/* Call a test client */
	client = mg_connect_client(
	    "127.0.0.1", 8082, 0, client_err_buf, sizeof(client_err_buf));

	ck_assert_str_eq(client_err_buf, "");
	ck_assert(client != NULL);

	mg_printf(client,
	          "PUT /%s HTTP/1.0\r\nContent-Length: %i\r\n\r\n",
	          "test_file_name.txt",
	          test_mg_store_body_con_len);

	r = 0;
	while (r < test_mg_store_body_con_len) {
		int l = mg_write(client, "1234567890", 10);
		ck_assert_int_eq(l, 10);
		r += 10;
	}

	r = mg_get_response(client, client_err_buf, sizeof(client_err_buf), 10000);
	ck_assert_int_ge(r, 0);
	ck_assert_str_eq(client_err_buf, "");

	client_ri = mg_get_response_info(client);
	ck_assert(client_ri != NULL);

	/* Response must be 200 OK  */
	ck_assert_int_eq(client_ri->status_code, 200);

	/* Read PUT response */
	r = mg_read(client, client_data_buf, sizeof(client_data_buf) - 1);
	ck_assert_int_gt(r, 0);
	client_data_buf[r] = 0;

	sprintf(check_data, "(%i bytes saved)", test_mg_store_body_con_len);
	check_ptr = strstr(client_data_buf, check_data);
	ck_assert_ptr_ne(check_ptr, NULL);

	mg_close_connection(client);

	/* Run the server for 5 seconds */
	test_sleep(5);

	/* Stop the server */
	test_mg_stop(ctx);

	/* Un-initialize the library */
	mg_exit_library();

	mark_point();
}
END_TEST


#if defined(MG_USE_OPEN_FILE) && !defined(NO_FILES)

#define FILE_IN_MEM_SIZE (1024 * 100)
static char *file_in_mem_data;

static const char *
test_file_in_memory_open_file(const struct mg_connection *conn,
                              const char *file_path,
                              size_t *file_size)
{
	(void)conn;

	if (strcmp(file_path, "./file_in_mem") == 0) {
		/* File is in memory */
		*file_size = FILE_IN_MEM_SIZE;
		return file_in_mem_data;
	} else {
		/* File is not in memory */
		return NULL;
	}
}


START_TEST(test_file_in_memory)
{
	/* Server var */
	struct mg_context *ctx;
	struct mg_callbacks callbacks;
	const char *OPTIONS[32];
	int opt_cnt = 0;
#if !defined(NO_SSL)
	const char *ssl_cert = locate_ssl_cert();
#endif

	/* Client var */
	struct mg_connection *client;
	char client_err_buf[256];
	char client_data_buf[256];
	const struct mg_request_info *client_ri;
	int64_t data_read;
	int r, i;

	/* Prepare test data */
	file_in_mem_data = (char *)malloc(FILE_IN_MEM_SIZE);
	ck_assert_ptr_ne(file_in_mem_data, NULL);
	for (r = 0; r < FILE_IN_MEM_SIZE; r++) {
		file_in_mem_data[r] = (char)(r);
	}

	/* Set options and start server */
	OPTIONS[opt_cnt++] = "document_root";
	OPTIONS[opt_cnt++] = ".";
#if defined(NO_SSL)
	OPTIONS[opt_cnt++] = "listening_ports";
	OPTIONS[opt_cnt++] = "8080";
#else
	OPTIONS[opt_cnt++] = "listening_ports";
	OPTIONS[opt_cnt++] = "8443s";
	OPTIONS[opt_cnt++] = "ssl_certificate";
	OPTIONS[opt_cnt++] = ssl_cert;
	ck_assert(ssl_cert != NULL);
#endif
	OPTIONS[opt_cnt] = NULL;


	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.open_file = test_file_in_memory_open_file;

	ctx = test_mg_start(&callbacks, 0, OPTIONS);
	ck_assert(ctx != NULL);

	/* connect client */
	memset(client_err_buf, 0, sizeof(client_err_buf));
	memset(client_data_buf, 0, sizeof(client_data_buf));

	client =
	    mg_download("127.0.0.1",
#if defined(NO_SSL)
	                8080,
	                0,
#else
	                8443,
	                1,
#endif
	                client_err_buf,
	                sizeof(client_err_buf),
	                "GET /file_in_mem HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n");

	ck_assert(client != NULL);
	ck_assert_str_eq(client_err_buf, "");

	client_ri = mg_get_response_info(client);

	ck_assert(client_ri != NULL);
	ck_assert_int_eq(client_ri->status_code, 200);

	ck_assert_int_eq(client_ri->content_length, FILE_IN_MEM_SIZE);

	data_read = 0;
	while (data_read < client_ri->content_length) {
		r = mg_read(client, client_data_buf, sizeof(client_data_buf));
		if (r > 0) {
			for (i = 0; i < r; i++) {
				ck_assert_int_eq((int)client_data_buf[i],
				                 (int)file_in_mem_data[data_read + i]);
			}
			data_read += r;
		}
	}

	/* Nothing left to read */
	r = mg_read(client, client_data_buf, sizeof(client_data_buf));
	ck_assert_int_eq(r, 0);

	/* Close the client connection */
	mg_close_connection(client);

	/* Stop the server */
	test_mg_stop(ctx);

	/* Free test data */
	free(file_in_mem_data);
	file_in_mem_data = NULL;
}
END_TEST

#else /* defined(MG_USE_OPEN_FILE) */

START_TEST(test_file_in_memory)
{
	mark_point();
}
END_TEST

#endif


static void
minimal_http_https_client_impl(const char *server,
                               uint16_t port,
                               int use_ssl,
                               const char *uri)
{
	/* Client var */
	struct mg_connection *client;
	char client_err_buf[256];
	char client_data_buf[256];
	const struct mg_response_info *client_ri;
	int64_t data_read;
	int r;

	mark_point();

	client = mg_connect_client(
	    server, port, use_ssl, client_err_buf, sizeof(client_err_buf));

	if ((client == NULL) || (0 != strcmp(client_err_buf, ""))) {
		ck_abort_msg("%s connection to server [%s] port [%u] failed: [%s]",
		             use_ssl ? "HTTPS" : "HTTP",
		             server,
		             port,
		             client_err_buf);
	}

	mg_printf(client, "GET /%s HTTP/1.0\r\n\r\n", uri);

	r = mg_get_response(client, client_err_buf, sizeof(client_err_buf), 10000);

	if ((r < 0) || (0 != strcmp(client_err_buf, ""))) {
		ck_abort_msg(
		    "%s connection to server [%s] port [%u] did not respond: [%s]",
		    use_ssl ? "HTTPS" : "HTTP",
		    server,
		    port,
		    client_err_buf);
	}

	client_ri = mg_get_response_info(client);
	ck_assert(client_ri != NULL);

	/* Check for status code 200 OK or 30? moved */
	if ((client_ri->status_code < 300) || (client_ri->status_code > 308)) {
		ck_assert_int_eq(client_ri->status_code, 200);
	}

	data_read = 0;
	while (data_read < client_ri->content_length) {
		r = mg_read(client, client_data_buf, sizeof(client_data_buf));
		if (r > 0) {
			data_read += r;
		}
	}

	/* Nothing left to read */
	r = mg_read(client, client_data_buf, sizeof(client_data_buf));
	ck_assert_int_eq(r, 0);

	mark_point();

	mg_close_connection(client);

	mark_point();
}


static void
minimal_http_client_impl(const char *server, uint16_t port, const char *uri)
{
	minimal_http_https_client_impl(server, port, 0, uri);
}


#if !defined(NO_SSL)
static void
minimal_https_client_impl(const char *server, uint16_t port, const char *uri)
{
	minimal_http_https_client_impl(server, port, 1, uri);
}
#endif


START_TEST(test_minimal_client)
{
	mark_point();

	/* Initialize the library */
	mg_init_library(0);

	mark_point();

	/* Call a test client */
	minimal_http_client_impl("192.30.253.113" /* www.github.com */,
	                         80,
	                         "civetweb/civetweb/");

	mark_point();

	/* Un-initialize the library */
	mg_exit_library();

	mark_point();
}
END_TEST


START_TEST(test_minimal_tls_client)
{
	mark_point();

#if !defined(NO_SSL) /* dont run https test if SSL is not enabled */

#if (!defined(__MACH__) || defined(LOCAL_TEST)) && !defined(OPENSSL_API_1_1)
	/* dont run on Travis OSX worker with OpenSSL 1.0 */

	/* Initialize the library */
	mg_init_library(2);

	mark_point();

	/* Call a test client */
	minimal_https_client_impl("192.30.253.113" /* www.github.com */,
	                          443,
	                          "civetweb/civetweb/");

	mark_point();

	/* Un-initialize the library */
	mg_exit_library();

#endif
#endif

	mark_point();
}
END_TEST


static int
minimal_test_request_handler(struct mg_connection *conn, void *cbdata)
{
	const char *msg = (const char *)cbdata;
	unsigned long len = (unsigned long)strlen(msg);

	mark_point();

	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\n"
	          "Content-Length: %lu\r\n"
	          "Content-Type: text/plain\r\n"
	          "Connection: close\r\n\r\n",
	          len);

	mg_write(conn, msg, len);

	mark_point();

	return 200;
}


START_TEST(test_minimal_http_server_callback)
{
	/* This test should ensure the minimum server example in
	 * docs/Embedding.md is still running. */

	/* Server context handle */
	struct mg_context *ctx;

	mark_point();

	/* Initialize the library */
	mg_init_library(0);

	/* Start the server */
	ctx = test_mg_start(NULL, 0, NULL);
	ck_assert(ctx != NULL);

	/* Add some handler */
	mg_set_request_handler(ctx,
	                       "/hello",
	                       minimal_test_request_handler,
	                       (void *)"Hello world");
	mg_set_request_handler(ctx,
	                       "/8",
	                       minimal_test_request_handler,
	                       (void *)"Number eight");

	/* Run the server for 15 seconds */
	test_sleep(10);

	/* Call a test client */
	minimal_http_client_impl("127.0.0.1", 8080, "/hello");

	/* Run the server for 15 seconds */
	test_sleep(5);


	/* Stop the server */
	test_mg_stop(ctx);

	/* Un-initialize the library */
	mg_exit_library();

	mark_point();
}
END_TEST


START_TEST(test_minimal_https_server_callback)
{
#if !defined(NO_SSL)
	/* This test should show a HTTPS server with enhanced
	 * security settings.
	 *
	 * Articles:
	 * https://hynek.me/articles/hardening-your-web-servers-ssl-ciphers/
	 *
	 * Scanners:
	 * https://securityheaders.io/
	 * https://www.htbridge.com/ssl/
	 * https://www.htbridge.com/websec/
	 * https://www.ssllabs.com/ssltest/
	 * https://www.qualys.com/forms/freescan/
	 */

	/* Server context handle */
	struct mg_context *ctx;

	/* Server start parameters for HTTPS */
	const char *OPTIONS[32];
	int opt_idx = 0;

	/* HTTPS port - required */
	OPTIONS[opt_idx++] = "listening_ports";
	OPTIONS[opt_idx++] = "8443s";

	/* path to certificate file - required */
	OPTIONS[opt_idx++] = "ssl_certificate";
	OPTIONS[opt_idx++] = locate_ssl_cert();

#if defined(LOCAL_TEST) || defined(_WIN32)
	/* Do not set this on Travis CI, since the build containers
	 * contain older SSL libraries */

	/* set minimum SSL version to TLS 1.2 - recommended */
	OPTIONS[opt_idx++] = "ssl_protocol_version";
	OPTIONS[opt_idx++] = "4";

	/* set some modern ciphers - recommended */
	OPTIONS[opt_idx++] = "ssl_cipher_list";
	OPTIONS[opt_idx++] = "ECDH+AESGCM+AES256:!aNULL:!MD5:!DSS";
#endif

	/* set "HTTPS only" header - recommended */
	OPTIONS[opt_idx++] = "strict_transport_security_max_age";
	OPTIONS[opt_idx++] = "31622400";

	/* end of options - required */
	OPTIONS[opt_idx] = NULL;

	mark_point();

	/* Initialize the library */
	mg_init_library(0);


	/* Start the server */
	ctx = test_mg_start(NULL, 0, OPTIONS);
	ck_assert(ctx != NULL);

	/* Add some handler */
	mg_set_request_handler(ctx,
	                       "/hello",
	                       minimal_test_request_handler,
	                       (void *)"Hello world");
	mg_set_request_handler(ctx,
	                       "/8",
	                       minimal_test_request_handler,
	                       (void *)"Number eight");

	/* Run the server for 15 seconds */
	test_sleep(10);

	/* Call a test client */
	minimal_https_client_impl("127.0.0.1", 8443, "/hello");

	/* Run the server for 15 seconds */
	test_sleep(5);


	/* Stop the server */
	test_mg_stop(ctx);

	/* Un-initialize the library */
	mg_exit_library();
#endif
	mark_point();
}
END_TEST


#if !defined(REPLACE_CHECK_FOR_LOCAL_DEBUGGING)
Suite *
make_public_server_suite(void)
{
	Suite *const suite = suite_create("PublicServer");

	TCase *const tcase_checktestenv = tcase_create("Check test environment");
	TCase *const tcase_initlib = tcase_create("Init library");
	TCase *const tcase_startthreads = tcase_create("Start threads");
	TCase *const tcase_minimal_http_svr = tcase_create("Minimal HTTP Server");
	TCase *const tcase_minimal_https_svr = tcase_create("Minimal HTTPS Server");
	TCase *const tcase_minimal_http_cli = tcase_create("Minimal HTTP Client");
	TCase *const tcase_minimal_https_cli = tcase_create("Minimal HTTPS Client");
	TCase *const tcase_startstophttp = tcase_create("Start Stop HTTP Server");
	TCase *const tcase_startstophttp_ipv6 =
	    tcase_create("Start Stop HTTP Server IPv6");
	TCase *const tcase_startstophttps = tcase_create("Start Stop HTTPS Server");
	TCase *const tcase_serverandclienttls = tcase_create("TLS Server Client");
	TCase *const tcase_serverrequests = tcase_create("Server Requests");
	TCase *const tcase_storebody = tcase_create("Store Body");
	TCase *const tcase_handle_form = tcase_create("Handle Form");
	TCase *const tcase_http_auth = tcase_create("HTTP Authentication");
	TCase *const tcase_keep_alive = tcase_create("HTTP Keep Alive");
	TCase *const tcase_error_handling = tcase_create("Error handling");
	TCase *const tcase_error_log = tcase_create("Error logging");
	TCase *const tcase_throttle = tcase_create("Limit speed");
	TCase *const tcase_large_file = tcase_create("Large file");
	TCase *const tcase_file_in_mem = tcase_create("File in memory");


	tcase_add_test(tcase_checktestenv, test_the_test_environment);
	tcase_set_timeout(tcase_checktestenv, civetweb_min_test_timeout);
	suite_add_tcase(suite, tcase_checktestenv);

	tcase_add_test(tcase_initlib, test_init_library);
	tcase_set_timeout(tcase_initlib, civetweb_min_test_timeout);
	suite_add_tcase(suite, tcase_initlib);

	tcase_add_test(tcase_startthreads, test_threading);
	tcase_set_timeout(tcase_startthreads, civetweb_min_test_timeout);
	suite_add_tcase(suite, tcase_startthreads);

	tcase_add_test(tcase_minimal_http_svr, test_minimal_http_server_callback);
	tcase_set_timeout(tcase_minimal_http_svr, civetweb_min_server_test_timeout);
	suite_add_tcase(suite, tcase_minimal_http_svr);

	tcase_add_test(tcase_minimal_https_svr, test_minimal_https_server_callback);
	tcase_set_timeout(tcase_minimal_https_svr,
	                  civetweb_min_server_test_timeout);
	suite_add_tcase(suite, tcase_minimal_https_svr);

	tcase_add_test(tcase_minimal_http_cli, test_minimal_client);
	tcase_set_timeout(tcase_minimal_http_cli, civetweb_min_server_test_timeout);
	suite_add_tcase(suite, tcase_minimal_http_cli);

	tcase_add_test(tcase_minimal_https_cli, test_minimal_tls_client);
	tcase_set_timeout(tcase_minimal_https_cli,
	                  civetweb_min_server_test_timeout);
	suite_add_tcase(suite, tcase_minimal_https_cli);

	tcase_add_test(tcase_startstophttp, test_mg_start_stop_http_server);
	tcase_set_timeout(tcase_startstophttp, civetweb_min_server_test_timeout);
	suite_add_tcase(suite, tcase_startstophttp);

	tcase_add_test(tcase_startstophttp_ipv6,
	               test_mg_start_stop_http_server_ipv6);
	tcase_set_timeout(tcase_startstophttp_ipv6,
	                  civetweb_min_server_test_timeout);
	suite_add_tcase(suite, tcase_startstophttp_ipv6);

	tcase_add_test(tcase_startstophttps, test_mg_start_stop_https_server);
	tcase_set_timeout(tcase_startstophttps, civetweb_min_server_test_timeout);
	suite_add_tcase(suite, tcase_startstophttps);

	tcase_add_test(tcase_serverandclienttls, test_mg_server_and_client_tls);
	tcase_set_timeout(tcase_serverandclienttls,
	                  civetweb_min_server_test_timeout);
	suite_add_tcase(suite, tcase_serverandclienttls);

	tcase_add_test(tcase_serverrequests, test_request_handlers);
	tcase_set_timeout(tcase_serverrequests, civetweb_mid_server_test_timeout);
	suite_add_tcase(suite, tcase_serverrequests);

	tcase_add_test(tcase_storebody, test_mg_store_body);
	tcase_set_timeout(tcase_storebody, civetweb_mid_server_test_timeout);
	suite_add_tcase(suite, tcase_storebody);

	tcase_add_test(tcase_handle_form, test_handle_form);
	tcase_set_timeout(tcase_handle_form, civetweb_mid_server_test_timeout);
	suite_add_tcase(suite, tcase_handle_form);

	tcase_add_test(tcase_http_auth, test_http_auth);
	tcase_set_timeout(tcase_http_auth, civetweb_min_server_test_timeout);
	suite_add_tcase(suite, tcase_http_auth);

	tcase_add_test(tcase_keep_alive, test_keep_alive);
	tcase_set_timeout(tcase_keep_alive, civetweb_mid_server_test_timeout);
	suite_add_tcase(suite, tcase_keep_alive);

	tcase_add_test(tcase_error_handling, test_error_handling);
	tcase_set_timeout(tcase_error_handling, civetweb_mid_server_test_timeout);
	suite_add_tcase(suite, tcase_error_handling);

	tcase_add_test(tcase_error_log, test_error_log_file);
	tcase_set_timeout(tcase_error_log, civetweb_mid_server_test_timeout);
	suite_add_tcase(suite, tcase_error_log);

	tcase_add_test(tcase_throttle, test_throttle);
	tcase_set_timeout(tcase_throttle, civetweb_mid_server_test_timeout);
	suite_add_tcase(suite, tcase_throttle);

	tcase_add_test(tcase_large_file, test_large_file);
	tcase_set_timeout(tcase_large_file, civetweb_mid_server_test_timeout);
	suite_add_tcase(suite, tcase_large_file);

	tcase_add_test(tcase_file_in_mem, test_file_in_memory);
	tcase_set_timeout(tcase_file_in_mem, civetweb_mid_server_test_timeout);
	suite_add_tcase(suite, tcase_file_in_mem);

	return suite;
}
#endif


#ifdef REPLACE_CHECK_FOR_LOCAL_DEBUGGING
/* Used to debug test cases without using the check framework */
/* Build command for Linux:
gcc test/public_server.c src/civetweb.c -I include/ -I test/ -l pthread -l dl -D
LOCAL_TEST -D REPLACE_CHECK_FOR_LOCAL_DEBUGGING -D MAIN_PUBLIC_SERVER=main
*/

static int chk_ok = 0;
static int chk_failed = 0;


void
MAIN_PUBLIC_SERVER(void)
{
	unsigned f_avail = mg_check_feature(0xFF);
	unsigned f_ret = mg_init_library(f_avail);
	ck_assert_uint_eq(f_ret, f_avail);

	test_handle_form(0);

	test_the_test_environment(0);
	test_threading(0);

	test_minimal_client(0);

	test_mg_start_stop_http_server(0);
	test_mg_start_stop_https_server(0);
	test_request_handlers(0);
	test_mg_store_body(0);
	test_mg_server_and_client_tls(0);
	test_handle_form(0);
	test_http_auth(0);
	test_keep_alive(0);
	test_error_handling(0);
	test_error_log_file(0);
	test_throttle(0);
	test_large_file(0);
	test_file_in_memory(0);

	mg_exit_library();

	printf("\nok: %i\nfailed: %i\n\n", chk_ok, chk_failed);
}

void
_ck_assert_failed(const char *file, int line, const char *expr, ...)
{
	va_list va;
	va_start(va, expr);
	fprintf(stderr, "Error: %s, line %i\n", file, line); /* breakpoint here ! */
	vfprintf(stderr, expr, va);
	fprintf(stderr, "\n\n");
	va_end(va);
	chk_failed++;
}

void
_ck_assert_msg(int cond, const char *file, int line, const char *expr, ...)
{
	va_list va;

	if (cond) {
		chk_ok++;
		return;
	}

	va_start(va, expr);
	fprintf(stderr, "Error: %s, line %i\n", file, line); /* breakpoint here ! */
	vfprintf(stderr, expr, va);
	fprintf(stderr, "\n\n");
	va_end(va);
	chk_failed++;
}

void
_mark_point(const char *file, int line)
{
	chk_ok++;
}

void
tcase_fn_start(const char *fname, const char *file, int line)
{
}
void suite_add_tcase(Suite *s, TCase *tc){};
void _tcase_add_test(TCase *tc,
                     TFun tf,
                     const char *fname,
                     int _signal,
                     int allowed_exit_value,
                     int start,
                     int end){};
TCase *
tcase_create(const char *name)
{
	return NULL;
};
Suite *
suite_create(const char *name)
{
	return NULL;
};
void tcase_set_timeout(TCase *tc, double timeout){};

#endif
