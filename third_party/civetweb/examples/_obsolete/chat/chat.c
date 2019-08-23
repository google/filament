// This file is part of the Civetweb project, http://code.google.com/p/civetweb
// It implements an online chat server. For more details,
// see the documentation on the project web site.
// To test the application,
// 1. type "make" in the directory where this file lives
// 2. point your browser to http://127.0.0.1:8081

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>

#include "civetweb.h"

#define MAX_USER_LEN  20
#define MAX_MESSAGE_LEN  100
#define MAX_MESSAGES 5
#define MAX_SESSIONS 2
#define SESSION_TTL 120

static const char *authorize_url = "/authorize";
static const char *login_url = "/login.html";
static const char *ajax_reply_start =
    "HTTP/1.1 200 OK\r\n"
    "Cache: no-cache\r\n"
    "Content-Type: application/x-javascript\r\n"
    "\r\n";

// Describes single message sent to a chat. If user is empty (0 length),
// the message is then originated from the server itself.
struct message {
    long id;                     // Message ID
    char user[MAX_USER_LEN];     // User that have sent the message
    char text[MAX_MESSAGE_LEN];  // Message text
    time_t timestamp;            // Message timestamp, UTC
};

// Describes web session.
struct session {
    char session_id[33];      // Session ID, must be unique
    char random[20];          // Random data used for extra user validation
    char user[MAX_USER_LEN];  // Authenticated user
    time_t expire;            // Expiration timestamp, UTC
};

static struct message messages[MAX_MESSAGES];  // Ringbuffer for messages
static struct session sessions[MAX_SESSIONS];  // Current sessions
static long last_message_id;

// Protects messages, sessions, last_message_id
static pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

// Get session object for the connection. Caller must hold the lock.
static struct session *get_session(const struct mg_connection *conn)
{
    int i;
    const char *cookie = mg_get_header(conn, "Cookie");
    char session_id[33];
    time_t now = time(NULL);
    mg_get_cookie(cookie, "session", session_id, sizeof(session_id));
    for (i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].expire != 0 &&
            sessions[i].expire > now &&
            strcmp(sessions[i].session_id, session_id) == 0) {
            break;
        }
    }
    return i == MAX_SESSIONS ? NULL : &sessions[i];
}

static void get_qsvar(const struct mg_request_info *request_info,
                      const char *name, char *dst, size_t dst_len)
{
    const char *qs = request_info->query_string;
    mg_get_var(qs, strlen(qs == NULL ? "" : qs), name, dst, dst_len);
}

// Get a get of messages with IDs greater than last_id and transform them
// into a JSON string. Return that string to the caller. The string is
// dynamically allocated, caller must free it. If there are no messages,
// NULL is returned.
static char *messages_to_json(long last_id)
{
    const struct message *message;
    int max_msgs, len;
    char buf[sizeof(messages)];  // Large enough to hold all messages

    // Read-lock the ringbuffer. Loop over all messages, making a JSON string.
    pthread_rwlock_rdlock(&rwlock);
    len = 0;
    max_msgs = sizeof(messages) / sizeof(messages[0]);
    // If client is too far behind, return all messages.
    if (last_message_id - last_id > max_msgs) {
        last_id = last_message_id - max_msgs;
    }
    for (; last_id < last_message_id; last_id++) {
        message = &messages[last_id % max_msgs];
        if (message->timestamp == 0) {
            break;
        }
        // buf is allocated on stack and hopefully is large enough to hold all
        // messages (it may be too small if the ringbuffer is full and all
        // messages are large. in this case asserts will trigger).
        len += snprintf(buf + len, sizeof(buf) - len,
                        "{user: '%s', text: '%s', timestamp: %lu, id: %ld},",
                        message->user, message->text, message->timestamp, message->id);
        assert(len > 0);
        assert((size_t) len < sizeof(buf));
    }
    pthread_rwlock_unlock(&rwlock);

    return len == 0 ? NULL : strdup(buf);
}

// If "callback" param is present in query string, this is JSONP call.
// Return 1 in this case, or 0 if "callback" is not specified.
// Wrap an output in Javascript function call.
static int handle_jsonp(struct mg_connection *conn,
                        const struct mg_request_info *request_info)
{
    char cb[64];

    get_qsvar(request_info, "callback", cb, sizeof(cb));
    if (cb[0] != '\0') {
        mg_printf(conn, "%s(", cb);
    }

    return cb[0] == '\0' ? 0 : 1;
}

// A handler for the /ajax/get_messages endpoint.
// Return a list of messages with ID greater than requested.
static void ajax_get_messages(struct mg_connection *conn,
                              const struct mg_request_info *request_info)
{
    char last_id[32], *json;
    int is_jsonp;

    mg_printf(conn, "%s", ajax_reply_start);
    is_jsonp = handle_jsonp(conn, request_info);

    get_qsvar(request_info, "last_id", last_id, sizeof(last_id));
    if ((json = messages_to_json(strtoul(last_id, NULL, 10))) != NULL) {
        mg_printf(conn, "[%s]", json);
        free(json);
    }

    if (is_jsonp) {
        mg_printf(conn, "%s", ")");
    }
}

// Allocate new message. Caller must hold the lock.
static struct message *new_message(void)
{
    static int size = sizeof(messages) / sizeof(messages[0]);
    struct message *message = &messages[last_message_id % size];
    message->id = last_message_id++;
    message->timestamp = time(0);
    return message;
}

static void my_strlcpy(char *dst, const char *src, size_t len)
{
    strncpy(dst, src, len);
    dst[len - 1] = '\0';
}

// A handler for the /ajax/send_message endpoint.
static void ajax_send_message(struct mg_connection *conn,
                              const struct mg_request_info *request_info)
{
    struct message *message;
    struct session *session;
    char text[sizeof(message->text) - 1];
    int is_jsonp;

    mg_printf(conn, "%s", ajax_reply_start);
    is_jsonp = handle_jsonp(conn, request_info);

    get_qsvar(request_info, "text", text, sizeof(text));
    if (text[0] != '\0') {
        // We have a message to store. Write-lock the ringbuffer,
        // grab the next message and copy data into it.
        pthread_rwlock_wrlock(&rwlock);
        message = new_message();
        // TODO(lsm): JSON-encode all text strings
        session = get_session(conn);
        assert(session != NULL);
        my_strlcpy(message->text, text, sizeof(text));
        my_strlcpy(message->user, session->user, sizeof(message->user));
        pthread_rwlock_unlock(&rwlock);
    }

    mg_printf(conn, "%s", text[0] == '\0' ? "false" : "true");

    if (is_jsonp) {
        mg_printf(conn, "%s", ")");
    }
}

// Redirect user to the login form. In the cookie, store the original URL
// we came from, so that after the authorization we could redirect back.
static void redirect_to_login(struct mg_connection *conn,
                              const struct mg_request_info *request_info)
{
    mg_printf(conn, "HTTP/1.1 302 Found\r\n"
              "Set-Cookie: original_url=%s\r\n"
              "Location: %s\r\n\r\n",
              request_info->uri, login_url);
}

// Return 1 if username/password is allowed, 0 otherwise.
static int check_password(const char *user, const char *password)
{
    // In production environment we should ask an authentication system
    // to authenticate the user.
    // Here however we do trivial check that user and password are not empty
    return (user[0] && password[0]);
}

// Allocate new session object
static struct session *new_session(void)
{
    int i;
    time_t now = time(NULL);
    pthread_rwlock_wrlock(&rwlock);
    for (i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].expire == 0 || sessions[i].expire < now) {
            sessions[i].expire = time(0) + SESSION_TTL;
            break;
        }
    }
    pthread_rwlock_unlock(&rwlock);
    return i == MAX_SESSIONS ? NULL : &sessions[i];
}

// Generate session ID. buf must be 33 bytes in size.
// Note that it is easy to steal session cookies by sniffing traffic.
// This is why all communication must be SSL-ed.
static void generate_session_id(char *buf, const char *random,
                                const char *user)
{
    mg_md5(buf, random, user, NULL);
}

static void send_server_message(const char *fmt, ...)
{
    va_list ap;
    struct message *message;

    pthread_rwlock_wrlock(&rwlock);
    message = new_message();
    message->user[0] = '\0';  // Empty user indicates server message
    va_start(ap, fmt);
    vsnprintf(message->text, sizeof(message->text), fmt, ap);
    va_end(ap);

    pthread_rwlock_unlock(&rwlock);
}

// A handler for the /authorize endpoint.
// Login page form sends user name and password to this endpoint.
static void authorize(struct mg_connection *conn,
                      const struct mg_request_info *request_info)
{
    char user[MAX_USER_LEN], password[MAX_USER_LEN];
    struct session *session;

    // Fetch user name and password.
    get_qsvar(request_info, "user", user, sizeof(user));
    get_qsvar(request_info, "password", password, sizeof(password));

    if (check_password(user, password) && (session = new_session()) != NULL) {
        // Authentication success:
        //   1. create new session
        //   2. set session ID token in the cookie
        //   3. remove original_url from the cookie - not needed anymore
        //   4. redirect client back to the original URL
        //
        // The most secure way is to stay HTTPS all the time. However, just to
        // show the technique, we redirect to HTTP after the successful
        // authentication. The danger of doing this is that session cookie can
        // be stolen and an attacker may impersonate the user.
        // Secure application must use HTTPS all the time.
        my_strlcpy(session->user, user, sizeof(session->user));
        snprintf(session->random, sizeof(session->random), "%d", rand());
        generate_session_id(session->session_id, session->random, session->user);
        send_server_message("<%s> joined", session->user);
        mg_printf(conn, "HTTP/1.1 302 Found\r\n"
                  "Set-Cookie: session=%s; max-age=3600; http-only\r\n"  // Session ID
                  "Set-Cookie: user=%s\r\n"  // Set user, needed by Javascript code
                  "Set-Cookie: original_url=/; max-age=0\r\n"  // Delete original_url
                  "Location: /\r\n\r\n",
                  session->session_id, session->user);
    } else {
        // Authentication failure, redirect to login.
        redirect_to_login(conn, request_info);
    }
}

// Return 1 if request is authorized, 0 otherwise.
static int is_authorized(const struct mg_connection *conn,
                         const struct mg_request_info *request_info)
{
    struct session *session;
    char valid_id[33];
    int authorized = 0;

    // Always authorize accesses to login page and to authorize URI
    if (!strcmp(request_info->uri, login_url) ||
        !strcmp(request_info->uri, authorize_url)) {
        return 1;
    }

    pthread_rwlock_rdlock(&rwlock);
    if ((session = get_session(conn)) != NULL) {
        generate_session_id(valid_id, session->random, session->user);
        if (strcmp(valid_id, session->session_id) == 0) {
            session->expire = time(0) + SESSION_TTL;
            authorized = 1;
        }
    }
    pthread_rwlock_unlock(&rwlock);

    return authorized;
}

static void redirect_to_ssl(struct mg_connection *conn,
                            const struct mg_request_info *request_info)
{
    const char *p, *host = mg_get_header(conn, "Host");
    if (host != NULL && (p = strchr(host, ':')) != NULL) {
        mg_printf(conn, "HTTP/1.1 302 Found\r\n"
                  "Location: https://%.*s:8082/%s:8082\r\n\r\n",
                  (int) (p - host), host, request_info->uri);
    } else {
        mg_printf(conn, "%s", "HTTP/1.1 500 Error\r\n\r\nHost: header is not set");
    }
}

static int begin_request_handler(struct mg_connection *conn)
{
    const struct mg_request_info *request_info = mg_get_request_info(conn);
    int processed = 1;

    if (!request_info->is_ssl) {
        redirect_to_ssl(conn, request_info);
    } else if (!is_authorized(conn, request_info)) {
        redirect_to_login(conn, request_info);
    } else if (strcmp(request_info->uri, authorize_url) == 0) {
        authorize(conn, request_info);
    } else if (strcmp(request_info->uri, "/ajax/get_messages") == 0) {
        ajax_get_messages(conn, request_info);
    } else if (strcmp(request_info->uri, "/ajax/send_message") == 0) {
        ajax_send_message(conn, request_info);
    } else {
        // No suitable handler found, mark as not processed. Civetweb will
        // try to serve the request.
        processed = 0;
    }
    return processed;
}

static const char *options[] = {
    "document_root", "html",
    "listening_ports", "8081,8082s",
    "ssl_certificate", "ssl_cert.pem",
    "num_threads", "5",
    NULL
};

int main(void)
{
    struct mg_callbacks callbacks;
    struct mg_context *ctx;

    // Initialize random number generator. It will be used later on for
    // the session identifier creation.
    srand((unsigned) time(0));

    // Setup and start Civetweb
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.begin_request = begin_request_handler;
    if ((ctx = mg_start(&callbacks, NULL, options)) == NULL) {
        printf("%s\n", "Cannot start chat server, fatal exit");
        exit(EXIT_FAILURE);
    }

    // Wait until enter is pressed, then exit
    printf("Chat server started on ports %s, press enter to quit.\n",
           mg_get_option(ctx, "listening_ports"));
    getchar();
    mg_stop(ctx);
    printf("%s\n", "Chat server stopped.");

    return EXIT_SUCCESS;
}

// vim:ts=2:sw=2:et
