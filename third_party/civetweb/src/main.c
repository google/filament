/* Copyright (c) 2013-2018 the Civetweb developers
 * Copyright (c) 2004-2013 Sergey Lyubka
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

#if defined(_WIN32)

#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS /* Disable deprecation warning in VS2005 */
#endif
#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE
#endif
#if defined(WIN32_LEAN_AND_MEAN)
#undef WIN32_LEAN_AND_MEAN /* Required for some functions (tray icons, ...) */
#endif

#else

#define _XOPEN_SOURCE 600 /* For PATH_MAX on linux */
/* This should also be sufficient for "realpath", according to
 * http://man7.org/linux/man-pages/man3/realpath.3.html, but in
 * reality it does not seem to work. */
/* In case this causes a problem, disable the warning:
 * #pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
 * #pragma clang diagnostic ignored "-Wimplicit-function-declaration"
 */
#endif

#if !defined(IGNORE_UNUSED_RESULT)
#define IGNORE_UNUSED_RESULT(a) ((void)((a) && 1))
#endif

#if defined(__cplusplus) && (__cplusplus >= 201103L)
#define NO_RETURN [[noreturn]]
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#define NO_RETURN _Noreturn
#elif defined(__GNUC__)
#define NO_RETURN __attribute((noreturn))
#else
#define NO_RETURN
#endif

/* Use same defines as in civetweb.c before including system headers. */
#if !defined(_LARGEFILE_SOURCE)
#define _LARGEFILE_SOURCE /* For fseeko(), ftello() */
#endif
#if !defined(_FILE_OFFSET_BITS)
#define _FILE_OFFSET_BITS 64 /* Use 64-bit file offsets by default */
#endif
#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS /* <inttypes.h> wants this for C++ */
#endif
#if !defined(__STDC_LIMIT_MACROS)
#define __STDC_LIMIT_MACROS /* C++ wants that for INT64_MAX */
#endif

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "civetweb.h"

#undef printf
#define printf                                                                 \
	DO_NOT_USE_THIS_FUNCTION__USE_fprintf /* Required for unit testing */

#if defined(_WIN32) && !defined(__SYMBIAN32__) /* WINDOWS include block */
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0501 /* for tdm-gcc so we can use getconsolewindow */
#endif
#undef UNICODE
#include <io.h>
#include <shlobj.h>
#include <windows.h>
#include <winsvc.h>

#define getcwd(a, b) (_getcwd(a, b))
#if !defined(__MINGW32__)
extern char *_getcwd(char *buf, size_t size);
#endif

#if !defined(PATH_MAX)
#define PATH_MAX MAX_PATH
#endif

#if !defined(S_ISDIR)
#define S_ISDIR(x) ((x)&_S_IFDIR)
#endif

#define DIRSEP '\\'
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define sleep(x) (Sleep((x)*1000))
#define WINCDECL __cdecl
#define abs_path(rel, abs, abs_size) (_fullpath((abs), (rel), (abs_size)))

#else /* defined(_WIN32) && !defined(__SYMBIAN32__) - WINDOWS / UNIX include   \
         block */

#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

#define DIRSEP '/'
#define WINCDECL
#define abs_path(rel, abs, abs_size) (realpath((rel), (abs)))

#endif /* defined(_WIN32) && !defined(__SYMBIAN32__) - WINDOWS / UNIX include  \
          block */

#if !defined(DEBUG_ASSERT)
#if defined(DEBUG)

#if defined(_MSC_VER)
/* DEBUG_ASSERT has some const conditions */
#pragma warning(disable : 4127)
#endif

#define DEBUG_ASSERT(cond)                                                     \
	do {                                                                       \
		if (!(cond)) {                                                         \
			fprintf(stderr, "ASSERTION FAILED: %s", #cond);                    \
			exit(2); /* Exit with error */                                     \
		}                                                                      \
	} while (0)

#else
#define DEBUG_ASSERT(cond)
#endif /* DEBUG */
#endif

#if !defined(PATH_MAX)
#define PATH_MAX (1024)
#endif

#define MAX_OPTIONS (50)
#define MAX_CONF_FILE_LINE_SIZE (8 * 1024)

struct tuser_data {
	char *first_message;
};


/* Exit flag for the main loop (read and writen by different threads, thus
 * volatile). */
volatile int g_exit_flag = 0; /* 0 = continue running main loop */


static char g_server_base_name[40]; /* Set by init_server_name() */

static const char *g_server_name; /* Default from init_server_name,
                                   * updated later from the server config */
static const char *g_icon_name;   /* Default from init_server_name,
                                   * updated later from the server config */
static const char *g_website;     /* Default from init_server_name,
                                   * updated later from the server config */
static int g_num_add_domains;     /* Default from init_server_name,
                                   * updated later from the server config */
static const char **g_add_domain; /* Default from init_server_name,
                                   * updated later from the server config */


static char *g_system_info; /* Set by init_system_info() */
static char g_config_file_name[PATH_MAX] =
    ""; /* Set by
         *  process_command_line_arguments() */

static struct mg_context *g_ctx; /* Set by start_civetweb() */
static struct tuser_data
    g_user_data; /* Passed to mg_start() by start_civetweb() */

#if !defined(CONFIG_FILE)
#define CONFIG_FILE "civetweb.conf"
#endif /* !CONFIG_FILE */

#if !defined(PASSWORDS_FILE_NAME)
#define PASSWORDS_FILE_NAME ".htpasswd"
#endif

/* backup config file */
#if !defined(CONFIG_FILE2) && defined(__linux__)
#define CONFIG_FILE2 "/usr/local/etc/civetweb.conf"
#endif

enum {
	OPTION_TITLE,
	OPTION_ICON,
	OPTION_WEBPAGE,
	OPTION_ADD_DOMAIN,
	NUM_MAIN_OPTIONS
};

static struct mg_option main_config_options[] = {
    {"title", MG_CONFIG_TYPE_STRING, NULL},
    {"icon", MG_CONFIG_TYPE_STRING, NULL},
    {"website", MG_CONFIG_TYPE_STRING, NULL},
    {"add_domain", MG_CONFIG_TYPE_STRING_LIST, NULL},
    {NULL, MG_CONFIG_TYPE_UNKNOWN, NULL}};


static void WINCDECL
signal_handler(int sig_num)
{
	g_exit_flag = sig_num;
}


static NO_RETURN void
die(const char *fmt, ...)
{
	va_list ap;
	char msg[512] = "";

	va_start(ap, fmt);
	(void)vsnprintf(msg, sizeof(msg) - 1, fmt, ap);
	msg[sizeof(msg) - 1] = 0;
	va_end(ap);

#if defined(_WIN32)
	MessageBox(NULL, msg, "Error", MB_OK);
#else
	fprintf(stderr, "%s\n", msg);
#endif

	exit(EXIT_FAILURE);
}


#if defined(WIN32)
static int MakeConsole(void);
#endif


static void
show_server_name(void)
{
#if defined(WIN32)
	(void)MakeConsole();
#endif

	fprintf(stderr, "CivetWeb v%s, built on %s\n", mg_version(), __DATE__);
}


static NO_RETURN void
show_usage_and_exit(const char *exeName)
{
	const struct mg_option *options;
	int i;

	if (exeName == 0 || *exeName == 0) {
		exeName = "civetweb";
	}

	show_server_name();

	fprintf(stderr, "\nUsage:\n");
	fprintf(stderr, "  Start server with a set of options:\n");
	fprintf(stderr, "    %s [config_file]\n", exeName);
	fprintf(stderr, "    %s [-option value ...]\n", exeName);
	fprintf(stderr, "  Run as client:\n");
	fprintf(stderr, "    %s -C url\n", exeName);
	fprintf(stderr, "  Show system information:\n");
	fprintf(stderr, "    %s -I\n", exeName);
	fprintf(stderr, "  Add user/change password:\n");
	fprintf(stderr,
	        "    %s -A <htpasswd_file> <realm> <user> <passwd>\n",
	        exeName);
	fprintf(stderr, "  Remove user:\n");
	fprintf(stderr, "    %s -R <htpasswd_file> <realm> <user>\n", exeName);
	fprintf(stderr, "\nOPTIONS:\n");

	options = mg_get_valid_options();
	for (i = 0; options[i].name != NULL; i++) {
		fprintf(stderr,
		        "  -%s %s\n",
		        options[i].name,
		        ((options[i].default_value == NULL)
		             ? "<empty>"
		             : options[i].default_value));
	}

	options = main_config_options;
	for (i = 0; options[i].name != NULL; i++) {
		fprintf(stderr,
		        "  -%s %s\n",
		        options[i].name,
		        ((options[i].default_value == NULL)
		             ? "<empty>"
		             : options[i].default_value));
	}

	exit(EXIT_FAILURE);
}


#if defined(_WIN32) || defined(USE_COCOA)
static const char *config_file_top_comment =
    "# CivetWeb web server configuration file.\n"
    "# For detailed description of every option, visit\n"
    "# https://github.com/civetweb/civetweb/blob/master/docs/UserManual.md\n"
    "# Lines starting with '#' and empty lines are ignored.\n"
    "# To make a change, remove leading '#', modify option's value,\n"
    "# save this file and then restart Civetweb.\n\n";

static const char *
get_url_to_first_open_port(const struct mg_context *ctx)
{
	static char url[100];
	const char *open_ports = mg_get_option(ctx, "listening_ports");
	int a, b, c, d, port, n;

	if (sscanf(open_ports, "%d.%d.%d.%d:%d%n", &a, &b, &c, &d, &port, &n)
	    == 5) {
		snprintf(url,
		         sizeof(url),
		         "%s://%d.%d.%d.%d:%d",
		         open_ports[n] == 's' ? "https" : "http",
		         a,
		         b,
		         c,
		         d,
		         port);
	} else if (sscanf(open_ports, "%d%n", &port, &n) == 1) {
		snprintf(url,
		         sizeof(url),
		         "%s://localhost:%d",
		         open_ports[n] == 's' ? "https" : "http",
		         port);
	} else {
		snprintf(url, sizeof(url), "%s", "http://localhost:8080");
	}

	return url;
}


#if defined(ENABLE_CREATE_CONFIG_FILE)
static void
create_config_file(const struct mg_context *ctx, const char *path)
{
	const struct mg_option *options;
	const char *value;
	FILE *fp;
	int i;

	/* Create config file if it is not present yet */
	if ((fp = fopen(path, "r")) != NULL) {
		fclose(fp);
	} else if ((fp = fopen(path, "a+")) != NULL) {
		fprintf(fp, "%s", config_file_top_comment);
		options = mg_get_valid_options();
		for (i = 0; options[i].name != NULL; i++) {
			value = mg_get_option(ctx, options[i].name);
			fprintf(fp,
			        "# %s %s\n",
			        options[i].name,
			        value ? value : "<value>");
		}
		fclose(fp);
	}
}
#endif
#endif


static char *
sdup(const char *str)
{
	size_t len;
	char *p;

	len = strlen(str) + 1;
	p = (char *)malloc(len);

	if (p == NULL) {
		die("Cannot allocate %u bytes", (unsigned)len);
	}

	memcpy(p, str, len);
	return p;
}


#if 0 /* Unused code from "string duplicate with escape" */
static unsigned
hex2dec(char x)
{
    if ((x >= '0') && (x <= '9')) {
        return (unsigned)x - (unsigned)'0';
    }
    if ((x >= 'A') && (x <= 'F')) {
        return (unsigned)x - (unsigned)'A' + 10u;
    }
    if ((x >= 'a') && (x <= 'f')) {
        return (unsigned)x - (unsigned)'a' + 10u;
    }
    return 0;
}


static char *
sdupesc(const char *str)
{
	char *p = sdup(str);

	if (p) {
		char *d = p;
		while ((d = strchr(d, '\\')) != NULL) {
			switch (d[1]) {
			case 'a':
				d[0] = '\a';
				memmove(d + 1, d + 2, strlen(d + 1));
				break;
			case 'b':
				d[0] = '\b';
				memmove(d + 1, d + 2, strlen(d + 1));
				break;
			case 'e':
				d[0] = 27;
				memmove(d + 1, d + 2, strlen(d + 1));
				break;
			case 'f':
				d[0] = '\f';
				memmove(d + 1, d + 2, strlen(d + 1));
				break;
			case 'n':
				d[0] = '\n';
				memmove(d + 1, d + 2, strlen(d + 1));
				break;
			case 'r':
				d[0] = '\r';
				memmove(d + 1, d + 2, strlen(d + 1));
				break;
			case 't':
				d[0] = '\t';
				memmove(d + 1, d + 2, strlen(d + 1));
				break;
			case 'u':
				if (isxdigit(d[2]) && isxdigit(d[3]) && isxdigit(d[4])
				    && isxdigit(d[5])) {
					unsigned short u = (unsigned short)(hex2dec(d[2]) * 4096
					                                    + hex2dec(d[3]) * 256
					                                    + hex2dec(d[4]) * 16
					                                    + hex2dec(d[5]));
					char mbc[16];
					int mbl = wctomb(mbc, (wchar_t)u);
					if ((mbl > 0) && (mbl < 6)) {
						memcpy(d, mbc, (unsigned)mbl);
						memmove(d + mbl, d + 6, strlen(d + 5));
						/* Advance mbl characters (+1 is below) */
						d += (mbl - 1);
					} else {
						/* Invalid multi byte character */
						/* TODO: define what to do */
					}
				} else {
					/* Invalid esc sequence */
					/* TODO: define what to do */
				}
				break;
			case 'v':
				d[0] = '\v';
				memmove(d + 1, d + 2, strlen(d + 1));
				break;
			case 'x':
				if (isxdigit(d[2]) && isxdigit(d[3])) {
					d[0] = (char)((unsigned char)(hex2dec(d[2]) * 16
					                              + hex2dec(d[3])));
					memmove(d + 1, d + 4, strlen(d + 3));
				} else {
					/* Invalid esc sequence */
					/* TODO: define what to do */
				}
				break;
			case 'z':
				d[0] = 0;
				memmove(d + 1, d + 2, strlen(d + 1));
				break;
			case '\\':
				d[0] = '\\';
				memmove(d + 1, d + 2, strlen(d + 1));
				break;
			case '\'':
				d[0] = '\'';
				memmove(d + 1, d + 2, strlen(d + 1));
				break;
			case '\"':
				d[0] = '\"';
				memmove(d + 1, d + 2, strlen(d + 1));
				break;
			case 0:
				if (d == p) {
					/* Line is only \ */
					free(p);
					return NULL;
				}
			/* no break */
			default:
				/* invalid ESC sequence */
				/* TODO: define what to do */
				break;
			}

			/* Advance to next character */
			d++;
		}
	}
	return p;
}
#endif


static const char *
get_option(char **options, const char *option_name)
{
	int i = 0;
	const char *opt_value = NULL;

	/* TODO (low, api makeover): options should be an array of key-value-pairs,
	 * like
	 *     struct {const char * key, const char * value} options[]
	 * but it currently is an array with
	 *     options[2*i] = key, options[2*i + 1] = value
	 * (probably with a MG_LEGACY_INTERFACE definition)
	 */
	while (options[2 * i] != NULL) {
		if (strcmp(options[2 * i], option_name) == 0) {
			opt_value = options[2 * i + 1];
			break;
		}
		i++;
	}
	return opt_value;
}


static int
set_option(char **options, const char *name, const char *value)
{
	int i, type;
	const struct mg_option *default_options = mg_get_valid_options();
	const char *multi_sep = NULL;

	for (i = 0; main_config_options[i].name != NULL; i++) {
		/* These options are evaluated by main.c, not civetweb.c.
		 * Do not add it to options, and return OK */
		if (!strcmp(name, main_config_options[OPTION_TITLE].name)) {
			g_server_name = sdup(value);
			return 1;
		}
		if (!strcmp(name, main_config_options[OPTION_ICON].name)) {

			g_icon_name = sdup(value);
			return 1;
		}
		if (!strcmp(name, main_config_options[OPTION_WEBPAGE].name)) {
			g_website = sdup(value);
			return 1;
		}
		if (!strcmp(name, main_config_options[OPTION_ADD_DOMAIN].name)) {
			if (g_num_add_domains > 0) {
				g_add_domain = (const char **)realloc(
				    (void *)g_add_domain,
				    sizeof(char *) * ((unsigned)g_num_add_domains + 1u));
				if (!g_add_domain) {
					die("Out of memory");
				}
				g_add_domain[g_num_add_domains] = sdup(value);
				g_num_add_domains++;
			} else {
				g_add_domain = (const char **)malloc(sizeof(char *));
				if (!g_add_domain) {
					die("Out of memory");
				}
				g_add_domain[0] = sdup(value);
				g_num_add_domains = 1;
			}
			return 1;
		}
	}

	/* Not an option of main.c, so check if it is a CivetWeb server option */
	type = MG_CONFIG_TYPE_UNKNOWN;
	for (i = 0; default_options[i].name != NULL; i++) {
		if (!strcmp(default_options[i].name, name)) {
			type = default_options[i].type;
		}
	}
	switch (type) {
	case MG_CONFIG_TYPE_UNKNOWN:
		/* unknown option */
		return 0;
	case MG_CONFIG_TYPE_NUMBER:
		/* integer number >= 0, e.g. number of threads */
		if (atol(value) < 0) {
			/* invalid number */
			return 0;
		}
		break;
	case MG_CONFIG_TYPE_STRING:
		/* any text */
		break;
	case MG_CONFIG_TYPE_STRING_LIST:
		/* list of text items, separated by , */
		multi_sep = ",";
		break;
	case MG_CONFIG_TYPE_STRING_MULTILINE:
		/* lines of text, separated by carriage return line feed */
		multi_sep = "\r\n";
		break;
	case MG_CONFIG_TYPE_BOOLEAN:
		/* boolean value, yes or no */
		if ((0 != strcmp(value, "yes")) && (0 != strcmp(value, "no"))) {
			/* invalid boolean */
			return 0;
		}
		break;
	case MG_CONFIG_TYPE_YES_NO_OPTIONAL:
		/* boolean value, yes or no */
		if ((0 != strcmp(value, "yes")) && (0 != strcmp(value, "no"))
		    && (0 != strcmp(value, "optional"))) {
			/* invalid boolean */
			return 0;
		}
		break;
	case MG_CONFIG_TYPE_FILE:
	case MG_CONFIG_TYPE_DIRECTORY:
		/* TODO (low): check this option when it is set, instead of calling
		 * verify_existence later */
		break;
	case MG_CONFIG_TYPE_EXT_PATTERN:
		/* list of patterns, separated by | */
		multi_sep = "|";
		break;
	default:
		die("Unknown option type - option %s", name);
	}

	for (i = 0; i < MAX_OPTIONS; i++) {
		if (options[2 * i] == NULL) {
			/* Option not set yet. Add new option */
			options[2 * i] = sdup(name);
			options[2 * i + 1] = sdup(value);
			options[2 * i + 2] = NULL;
			break;
		} else if (!strcmp(options[2 * i], name)) {
			if (multi_sep) {
				/* Option already set. Overwrite */
				char *s =
				    (char *)malloc(strlen(options[2 * i + 1])
				                   + strlen(multi_sep) + strlen(value) + 1);
				if (!s) {
					die("Out of memory");
				}
				sprintf(s, "%s%s%s", options[2 * i + 1], multi_sep, value);
				free(options[2 * i + 1]);
				options[2 * i + 1] = s;
			} else {
				/* Option already set. Overwrite */
				free(options[2 * i + 1]);
				options[2 * i + 1] = sdup(value);
			}
			break;
		}
	}

	if (i == MAX_OPTIONS) {
		die("Too many options specified");
	}

	if (options[2 * i] == NULL) {
		die("Out of memory");
	}
	if (options[2 * i + 1] == NULL) {
		die("Illegal escape sequence, or out of memory");
	}

	/* option set correctly */
	return 1;
}


static int
read_config_file(const char *config_file, char **options)
{
	char line[MAX_CONF_FILE_LINE_SIZE], *p;
	FILE *fp = NULL;
	size_t i, j, line_no = 0;

	/* Open the config file */
	fp = fopen(config_file, "r");
	if (fp == NULL) {
		/* Failed to open the file. Keep errno for the caller. */
		return 0;
	}

	/* Load config file settings first */
	fprintf(stdout, "Loading config file %s\n", config_file);

	/* Loop over the lines in config file */
	while (fgets(line, sizeof(line), fp) != NULL) {

		if (!line_no && !memcmp(line, "\xEF\xBB\xBF", 3)) {
			/* strip UTF-8 BOM */
			p = line + 3;
		} else {
			p = line;
		}
		line_no++;

		/* Ignore empty lines and comments */
		for (i = 0; isspace((unsigned char)p[i]);)
			i++;
		if (p[i] == '#' || p[i] == '\0') {
			continue;
		}

		/* Skip spaces, \r and \n at the end of the line */
		for (j = strlen(p); (j > 0)
		                    && (isspace((unsigned char)p[j - 1])
		                        || iscntrl((unsigned char)p[j - 1]));)
			p[--j] = 0;

		/* Find the space character between option name and value */
		for (j = i; !isspace((unsigned char)p[j]) && (p[j] != 0);)
			j++;

		/* Terminate the string - then the string at (p+i) contains the
		 * option name */
		p[j] = 0;
		j++;

		/* Trim additional spaces between option name and value - then
		 * (p+j) contains the option value */
		while (isspace((unsigned char)p[j])) {
			j++;
		}

		/* Set option */
		if (!set_option(options, p + i, p + j)) {
			fprintf(stderr,
			        "%s: line %d is invalid, ignoring it:\n %s",
			        config_file,
			        (int)line_no,
			        p);
		}
	}

	(void)fclose(fp);

	return 1;
}


static void
process_command_line_arguments(int argc, char *argv[], char **options)
{
	char *p;
	size_t i, cmd_line_opts_start = 1;
#if defined(CONFIG_FILE2)
	FILE *fp = NULL;
#endif

	/* Should we use a config file ? */
	if ((argc > 1) && (argv[1] != NULL) && (argv[1][0] != '-')
	    && (argv[1][0] != 0)) {
		/* The first command line parameter is a config file name. */
		snprintf(g_config_file_name,
		         sizeof(g_config_file_name) - 1,
		         "%s",
		         argv[1]);
		cmd_line_opts_start = 2;
	} else if ((p = strrchr(argv[0], DIRSEP)) == NULL) {
		/* No config file set. No path in arg[0] found.
		 * Use default file name in the current path. */
		snprintf(g_config_file_name,
		         sizeof(g_config_file_name) - 1,
		         "%s",
		         CONFIG_FILE);
	} else {
		/* No config file set. Path to exe found in arg[0].
		 * Use default file name next to the executable. */
		snprintf(g_config_file_name,
		         sizeof(g_config_file_name) - 1,
		         "%.*s%c%s",
		         (int)(p - argv[0]),
		         argv[0],
		         DIRSEP,
		         CONFIG_FILE);
	}
	g_config_file_name[sizeof(g_config_file_name) - 1] = 0;

#if defined(CONFIG_FILE2)
	fp = fopen(g_config_file_name, "r");

	/* try alternate config file */
	if (fp == NULL) {
		fp = fopen(CONFIG_FILE2, "r");
		if (fp != NULL) {
			strcpy(g_config_file_name, CONFIG_FILE2);
		}
	}
	if (fp != NULL) {
		fclose(fp);
	}
#endif

	/* read all configurations from a config file */
	if (0 == read_config_file(g_config_file_name, options)) {
		if (cmd_line_opts_start == 2) {
			/* If config file was set in command line and open failed, die. */
			/* Errno will still hold the error from fopen. */
			die("Cannot open config file %s: %s",
			    g_config_file_name,
			    strerror(errno));
		}
		/* Otherwise: CivetWeb can work without a config file */
	}

	/* If we're under MacOS and started by launchd, then the second
	   argument is process serial number, -psn_.....
	   In this case, don't process arguments at all. */
	if (argv[1] == NULL || memcmp(argv[1], "-psn_", 5) != 0) {
		/* Handle command line flags.
		   They override config file and default settings. */
		for (i = cmd_line_opts_start; argv[i] != NULL; i += 2) {
			if (argv[i][0] != '-' || argv[i + 1] == NULL) {
				show_usage_and_exit(argv[0]);
			}
			if (!set_option(options, &argv[i][1], argv[i + 1])) {
				fprintf(
				    stderr,
				    "command line option is invalid, ignoring it:\n %s %s\n",
				    argv[i],
				    argv[i + 1]);
			}
		}
	}
}


static void
init_system_info(void)
{
	int len = mg_get_system_info(NULL, 0);
	if (len > 0) {
		g_system_info = (char *)malloc((unsigned)len + 1);
		(void)mg_get_system_info(g_system_info, len + 1);
	} else {
		g_system_info = sdup("Not available");
	}
}


static void
init_server_name(void)
{
	DEBUG_ASSERT(sizeof(main_config_options) / sizeof(main_config_options[0])
	             == NUM_MAIN_OPTIONS + 1);
	DEBUG_ASSERT((strlen(mg_version()) + 12) < sizeof(g_server_base_name));
	snprintf(g_server_base_name,
	         sizeof(g_server_base_name),
	         "CivetWeb V%s",
	         mg_version());
	g_server_name = g_server_base_name;
	g_icon_name = NULL;
	g_website = "http://civetweb.github.io/civetweb/";
	g_num_add_domains = 0;
	g_add_domain = NULL;
}


static void
free_system_info(void)
{
	free(g_system_info);
}


static int
log_message(const struct mg_connection *conn, const char *message)
{
	const struct mg_context *ctx = mg_get_context(conn);
	struct tuser_data *ud = (struct tuser_data *)mg_get_user_data(ctx);

	fprintf(stderr, "%s\n", message);

	if (ud->first_message == NULL) {
		ud->first_message = sdup(message);
	}

	return 0;
}


static int
is_path_absolute(const char *path)
{
#if defined(_WIN32)
	return path != NULL
	       && ((path[0] == '\\' && path[1] == '\\') || /* UNC path, e.g.
	                                                      \\server\dir */
	           (isalpha((unsigned char)path[0]) && path[1] == ':'
	            && path[2] == '\\')); /* E.g. X:\dir */
#else
	return path != NULL && path[0] == '/';
#endif
}


static void
verify_existence(char **options, const char *option_name, int must_be_dir)
{
	struct stat st;
	const char *path = get_option(options, option_name);

#if defined(_WIN32)
	wchar_t wbuf[1024];
	char mbbuf[1024];
	int len;

	if (path) {
		memset(wbuf, 0, sizeof(wbuf));
		memset(mbbuf, 0, sizeof(mbbuf));
		len = MultiByteToWideChar(
		    CP_UTF8, 0, path, -1, wbuf, sizeof(wbuf) / sizeof(wbuf[0]) - 1);
		wcstombs(mbbuf, wbuf, sizeof(mbbuf) - 1);
		path = mbbuf;
		(void)len;
	}
#endif

	if (path != NULL
	    && (stat(path, &st) != 0
	        || ((S_ISDIR(st.st_mode) ? 1 : 0) != must_be_dir))) {
		die("Invalid path for %s: [%s]: (%s). Make sure that path is either "
		    "absolute, or it is relative to civetweb executable.",
		    option_name,
		    path,
		    strerror(errno));
	}
}


static void
set_absolute_path(char *options[],
                  const char *option_name,
                  const char *path_to_civetweb_exe)
{
	char path[PATH_MAX] = "", absolute[PATH_MAX] = "";
	const char *option_value;
	const char *p;

	/* Check whether option is already set */
	option_value = get_option(options, option_name);

	/* If option is already set and it is an absolute path,
	   leave it as it is -- it's already absolute. */
	if (option_value != NULL && !is_path_absolute(option_value)) {
		/* Not absolute. Use the directory where civetweb executable lives
		   be the relative directory for everything.
		   Extract civetweb executable directory into path. */
		if ((p = strrchr(path_to_civetweb_exe, DIRSEP)) == NULL) {
			IGNORE_UNUSED_RESULT(getcwd(path, sizeof(path)));
		} else {
			snprintf(path,
			         sizeof(path) - 1,
			         "%.*s",
			         (int)(p - path_to_civetweb_exe),
			         path_to_civetweb_exe);
			path[sizeof(path) - 1] = 0;
		}

		strncat(path, "/", sizeof(path) - strlen(path) - 1);
		strncat(path, option_value, sizeof(path) - strlen(path) - 1);

		/* Absolutize the path, and set the option */
		IGNORE_UNUSED_RESULT(abs_path(path, absolute, sizeof(absolute)));
		set_option(options, option_name, absolute);
	}
}


#if defined(USE_LUA)

#include "civetweb_private_lua.h"

#endif


#if defined(USE_DUKTAPE)

#include "duktape.h"

static int
run_duktape(const char *file_name)
{
	duk_context *ctx = NULL;

	ctx = duk_create_heap_default();
	if (!ctx) {
		fprintf(stderr, "Failed to create a Duktape heap.\n");
		goto finished;
	}

	if (duk_peval_file(ctx, file_name) != 0) {
		fprintf(stderr, "%s\n", duk_safe_to_string(ctx, -1));
		goto finished;
	}
	duk_pop(ctx); /* ignore result */

finished:
	duk_destroy_heap(ctx);

	return 0;
}
#endif


#if defined(__MINGW32__) || defined(__MINGW64__)
/* For __MINGW32/64_MAJOR/MINOR_VERSION define */
#include <_mingw.h>
#endif


static int
run_client(const char *url_arg)
{
	/* connection data */
	char *url = sdup(url_arg); /* OOM will cause program to exit */
	char *host;
	char *resource;
	int is_ssl = 0;
	unsigned long port = 0;
	size_t sep;
	char *endp = 0;
	char empty[] = "";

	/* connection object */
	struct mg_connection *conn;
	char ebuf[1024] = {0};

#if 0 /* Unreachable code, since sdup will never return NULL */
    /* Check out of memory */
    if (!url) {
        fprintf(stderr, "Out of memory\n");
        return 0;
    }
#endif

	/* Check parameter */
	if (!strncmp(url, "http://", 7)) {
		host = url + 7;
		port = 80;
	} else if (!strncmp(url, "https://", 8)) {
		host = url + 8;
		is_ssl = 1;
		port = 443;
	} else {
		fprintf(stderr, "URL must start with http:// or https://\n");
		free(url);
		return 0;
	}
	if ((host[0] <= 32) || (host[0] > 126) || (host[0] == '/')
	    || (host[0] == ':')) {
		fprintf(stderr, "Invalid host\n");
		free(url);
		return 0;
	}

	sep = strcspn(host, "/:");
	switch (host[sep]) {
	case 0:
		resource = empty;
		break;
	case '/':
		host[sep] = 0;
		resource = host + sep + 1;
		break;
	case ':':
		host[sep] = 0;
		port = strtoul(host + sep + 1, &endp, 10);
		if (!endp || (*endp != '/' && *endp != 0) || (port < 1)
		    || (port > 0xFFFF)) {
			fprintf(stderr, "Invalid port\n");
			free(url);
			return 0;
		}
		if (*endp) {
			*endp = 0;
			resource = endp + 1;
		} else {
			resource = empty;
		}
		break;
	default:
		fprintf(stderr, "Syntax error\n");
		free(url);
		return 0;
	}

	fprintf(stdout, "Protocol: %s\n", is_ssl ? "https" : "http");
	fprintf(stdout, "Host: %s\n", host);
	fprintf(stdout, "Port: %lu\n", port);
	fprintf(stdout, "Resource: %s\n", resource);

	/* Initialize library */
	if (is_ssl) {
		mg_init_library(MG_FEATURES_TLS);
	} else {
		mg_init_library(MG_FEATURES_DEFAULT);
	}

	/* Connect to host */
	conn = mg_connect_client(host, (int)port, is_ssl, ebuf, sizeof(ebuf));
	if (conn) {
		/* Connecting to server worked */
		char buf[1024] = {0};
		int ret;

		fprintf(stdout, "Connected to %s\n", host);

		/* Send GET request */
		mg_printf(conn,
		          "GET /%s HTTP/1.1\r\n"
		          "Host: %s\r\n"
		          "Connection: close\r\n"
		          "\r\n",
		          resource,
		          host);

		/* Wait for server to respond with a HTTP header */
		ret = mg_get_response(conn, ebuf, sizeof(ebuf), 10000);

		if (ret >= 0) {
			const struct mg_response_info *ri = mg_get_response_info(conn);

			fprintf(stdout,
			        "Response info: %i %s\n",
			        ri->status_code,
			        ri->status_text);

			/* Respond reader read. Read body (if any) */
			ret = mg_read(conn, buf, sizeof(buf));
			while (ret > 0) {
				fwrite(buf, 1, (unsigned)ret, stdout);
				ret = mg_read(conn, buf, sizeof(buf));
			}

			fprintf(stdout, "Closing connection to %s\n", host);

		} else {
			/* Server did not reply to HTTP request */
			fprintf(stderr, "Got no response from %s:\n%s\n", host, ebuf);
		}
		mg_close_connection(conn);

	} else {
		/* Connecting to server failed */
		fprintf(stderr, "Error connecting to %s:\n%s\n", host, ebuf);
	}

	/* Free memory and exit library */
	free(url);
	mg_exit_library();
	return 1;
}

static void
sanitize_options(char *options[] /* server options */,
                 const char *arg0 /* argv[0] */)
{
	/* Make sure we have absolute paths for files and directories */
	set_absolute_path(options, "document_root", arg0);
	set_absolute_path(options, "put_delete_auth_file", arg0);
	set_absolute_path(options, "cgi_interpreter", arg0);
	set_absolute_path(options, "access_log_file", arg0);
	set_absolute_path(options, "error_log_file", arg0);
	set_absolute_path(options, "global_auth_file", arg0);
#if defined(USE_LUA)
	set_absolute_path(options, "lua_preload_file", arg0);
#endif
	set_absolute_path(options, "ssl_certificate", arg0);

	/* Make extra verification for certain options */
	verify_existence(options, "document_root", 1);
	verify_existence(options, "cgi_interpreter", 0);
	verify_existence(options, "ssl_certificate", 0);
	verify_existence(options, "ssl_ca_path", 1);
	verify_existence(options, "ssl_ca_file", 0);
#if defined(USE_LUA)
	verify_existence(options, "lua_preload_file", 0);
#endif
}


static void
start_civetweb(int argc, char *argv[])
{
	struct mg_callbacks callbacks;
	char *options[2 * MAX_OPTIONS + 1];
	int i;

	/* Start option -I:
	 * Show system information and exit
	 * This is very useful for diagnosis. */
	if (argc > 1 && !strcmp(argv[1], "-I")) {

#if defined(WIN32)
		(void)MakeConsole();
#endif
		fprintf(stdout,
		        "\n%s (%s)\n%s\n",
		        g_server_base_name,
		        g_server_name,
		        g_system_info);

		exit(EXIT_SUCCESS);
	}

	/* Edit passwords file: Add user or change password, if -A option is
	 * specified */
	if (argc > 1 && !strcmp(argv[1], "-A")) {
		if (argc != 6) {
			show_usage_and_exit(argv[0]);
		}
		exit(mg_modify_passwords_file(argv[2], argv[3], argv[4], argv[5])
		         ? EXIT_SUCCESS
		         : EXIT_FAILURE);
	}

	/* Edit passwords file: Remove user, if -R option is specified */
	if (argc > 1 && !strcmp(argv[1], "-R")) {
		if (argc != 5) {
			show_usage_and_exit(argv[0]);
		}
		exit(mg_modify_passwords_file(argv[2], argv[3], argv[4], NULL)
		         ? EXIT_SUCCESS
		         : EXIT_FAILURE);
	}

	/* Client mode */
	if (argc > 1 && !strcmp(argv[1], "-C")) {
		if (argc != 3) {
			show_usage_and_exit(argv[0]);
		}

		exit(run_client(argv[2]) ? EXIT_SUCCESS : EXIT_FAILURE);
	}

	/* Call Lua with additional CivetWeb specific Lua functions, if -L option
	 * is specified */
	if (argc > 1 && !strcmp(argv[1], "-L")) {

#if defined(USE_LUA)
		if (argc != 3) {
			show_usage_and_exit(argv[0]);
		}
#if defined(WIN32)
		(void)MakeConsole();
#endif
		exit(run_lua(argv[2]));
#else
		show_server_name();
		fprintf(stderr, "\nError: Lua support not enabled\n");
		exit(EXIT_FAILURE);
#endif
	}

	/* Call Duktape, if -E option is specified */
	if (argc > 1 && !strcmp(argv[1], "-E")) {

#if defined(USE_DUKTAPE)
		if (argc != 3) {
			show_usage_and_exit(argv[0]);
		}
#if defined(WIN32)
		(void)MakeConsole();
#endif
		exit(run_duktape(argv[2]));
#else
		show_server_name();
		fprintf(stderr, "\nError: Ecmascript support not enabled\n");
		exit(EXIT_FAILURE);
#endif
	}

	/* Show usage if -h or --help options are specified */
	if (argc == 2
	    && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "-H")
	        || !strcmp(argv[1], "--help"))) {
		show_usage_and_exit(argv[0]);
	}

	/* Initialize options structure */
	memset(options, 0, sizeof(options));
	set_option(options, "document_root", ".");

	/* Update config based on command line arguments */
	process_command_line_arguments(argc, argv, options);

	sanitize_options(options, argv[0]);

	/* Setup signal handler: quit on Ctrl-C */
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);

#if defined(DAEMONIZE)
	/* Daemonize */
	for (i = 0; options[i] != NULL; i++) {
		if (strcmp(options[i], "daemonize") == 0) {
			if (options[i + 1] != NULL) {
				if (mg_strcasecmp(options[i + 1], "yes") == 0) {
					fprintf(stdout, "daemonize.\n");
					if (daemon(0, 0) != 0) {
						fprintf(stdout, "Faild to daemonize main process.\n");
						exit(EXIT_FAILURE);
					}
					FILE *fp;
					if ((fp = fopen(PID_FILE, "w")) == 0) {
						fprintf(stdout, "Can not open %s.\n", PID_FILE);
						exit(EXIT_FAILURE);
					}
					fprintf(fp, "%d", getpid());
					fclose(fp);
				}
			}
			break;
		}
	}
#endif

	/* Initialize user data */
	memset(&g_user_data, 0, sizeof(g_user_data));

	/* Start Civetweb */
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.log_message = &log_message;
	g_ctx = mg_start(&callbacks, &g_user_data, (const char **)options);

	/* mg_start copies all options to an internal buffer.
	 * The options data field here is not required anymore. */
	for (i = 0; options[i] != NULL; i++) {
		free(options[i]);
	}

	/* If mg_start fails, it returns NULL */
	if (g_ctx == NULL) {
		die("Failed to start %s:\n%s",
		    g_server_name,
		    ((g_user_data.first_message == NULL) ? "unknown reason"
		                                         : g_user_data.first_message));
	}

#if defined(MG_EXPERIMENTAL_INTERFACES)
	for (i = 0; i < g_num_add_domains; i++) {

		int j;
		memset(options, 0, sizeof(options));
		set_option(options, "document_root", ".");

		if (0 == read_config_file(g_add_domain[i], options)) {
			die("Cannot open config file %s: %s",
			    g_add_domain[i],
			    strerror(errno));
		}

		sanitize_options(options, argv[0]);

		j = mg_start_domain(g_ctx, (const char **)options);
		if (j < 0) {
			die("Error loading domain file %s: %i", g_add_domain[i], j);
		} else {
			fprintf(stdout, "Domain file %s loaded\n", g_add_domain[i]);
		}

		for (j = 0; options[j] != NULL; j++) {
			free(options[j]);
		}
	}
#endif
}


static void
stop_civetweb(void)
{
	mg_stop(g_ctx);
	free(g_user_data.first_message);
	g_user_data.first_message = NULL;
}


#if defined(_WIN32)
/* Win32 has a small GUI.
 * Define some GUI elements and Windows message handlers. */

enum {
	ID_ICON = 100,
	ID_QUIT,
	ID_SETTINGS,
	ID_SEPARATOR,
	ID_INSTALL_SERVICE,
	ID_REMOVE_SERVICE,
	ID_STATIC,
	ID_GROUP,
	ID_PASSWORD,
	ID_SAVE,
	ID_RESET_DEFAULTS,
	ID_RESET_FILE,
	ID_RESET_ACTIVE,
	ID_STATUS,
	ID_CONNECT,
	ID_ADD_USER,
	ID_ADD_USER_NAME,
	ID_ADD_USER_REALM,
	ID_INPUT_LINE,
	ID_SYSINFO,
	ID_WEBSITE,

	/* All dynamically created text boxes for options have IDs starting from
   ID_CONTROLS, incremented by one. */
	ID_CONTROLS = 200,

	/* Text boxes for files have "..." buttons to open file browser. These
   buttons have IDs that are ID_FILE_BUTTONS_DELTA higher than associated
   text box ID. */
	ID_FILE_BUTTONS_DELTA = 1000
};


static HICON hIcon;
static SERVICE_STATUS ss;
static SERVICE_STATUS_HANDLE hStatus;
static const char *service_magic_argument = "--";
static NOTIFYICONDATA TrayIcon;

static void WINAPI
ControlHandler(DWORD code)
{
	if (code == SERVICE_CONTROL_STOP || code == SERVICE_CONTROL_SHUTDOWN) {
		ss.dwWin32ExitCode = 0;
		ss.dwCurrentState = SERVICE_STOPPED;
	}
	SetServiceStatus(hStatus, &ss);
}


static void WINAPI
ServiceMain(void)
{
	ss.dwServiceType = SERVICE_WIN32;
	ss.dwCurrentState = SERVICE_RUNNING;
	ss.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

	hStatus = RegisterServiceCtrlHandler(g_server_name, ControlHandler);
	SetServiceStatus(hStatus, &ss);

	while (ss.dwCurrentState == SERVICE_RUNNING) {
		Sleep(1000);
	}
	stop_civetweb();

	ss.dwCurrentState = SERVICE_STOPPED;
	ss.dwWin32ExitCode = (DWORD)-1;
	SetServiceStatus(hStatus, &ss);
}


static void
show_error(void)
{
	char buf[256];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
	              NULL,
	              GetLastError(),
	              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	              buf,
	              sizeof(buf),
	              NULL);
	MessageBox(NULL, buf, "Error", MB_OK);
}


static void *
align(void *ptr, uintptr_t alig)
{
	uintptr_t ul = (uintptr_t)ptr;
	ul += alig;
	ul &= ~alig;
	return ((void *)ul);
}


static void
save_config(HWND hDlg, FILE *fp)
{
	char value[2000] = "";
	const char *default_value;
	const struct mg_option *options;
	int i, id;

	fprintf(fp, "%s", config_file_top_comment);
	options = mg_get_valid_options();
	for (i = 0; options[i].name != NULL; i++) {
		id = ID_CONTROLS + i;
		if (options[i].type == MG_CONFIG_TYPE_BOOLEAN) {
			snprintf(value,
			         sizeof(value) - 1,
			         "%s",
			         IsDlgButtonChecked(hDlg, id) ? "yes" : "no");
			value[sizeof(value) - 1] = 0;
		} else {
			GetDlgItemText(hDlg, id, value, sizeof(value));
		}
		default_value =
		    options[i].default_value == NULL ? "" : options[i].default_value;
		/* If value is the same as default, skip it */
		if (strcmp(value, default_value) != 0) {
			fprintf(fp, "%s %s\n", options[i].name, value);
		}
	}
}


/* LPARAM pointer passed to WM_INITDIALOG */
struct dlg_proc_param {
	int guard;
	HWND hWnd;
	const char *name;
	char *buffer;
	unsigned buflen;
	int idRetry;
	BOOL (*fRetry)(struct dlg_proc_param *data);
};

struct dlg_header_param {
	DLGTEMPLATE dlg_template; /* 18 bytes */
	WORD menu, dlg_class;
	wchar_t caption[1];
	WORD fontsiz;
	wchar_t fontface[7];
};

static struct dlg_header_param
GetDlgHeader(const short width)
{
#if defined(_MSC_VER)
/* disable MSVC warning C4204 (non-constant used to initialize structure) */
#pragma warning(push)
#pragma warning(disable : 4204)
#endif /* if defined(_MSC_VER) */
	struct dlg_header_param dialog_header = {{WS_CAPTION | WS_POPUP | WS_SYSMENU
	                                              | WS_VISIBLE | DS_SETFONT
	                                              | WS_DLGFRAME,
	                                          WS_EX_TOOLWINDOW,
	                                          0,
	                                          200,
	                                          200,
	                                          width,
	                                          0},
	                                         0,
	                                         0,
	                                         L"",
	                                         8,
	                                         L"Tahoma"};
#if defined(_MSC_VER)
#pragma warning(pop)
#endif /* if defined(_MSC_VER) */
	return dialog_header;
}

/* Dialog proc for settings dialog */
static INT_PTR CALLBACK
SettingsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	FILE *fp;
	int i, j;
	const char *name, *value;
	const struct mg_option *default_options = mg_get_valid_options();
	char *file_options[MAX_OPTIONS * 2 + 1] = {0};
	char *title;
	struct dlg_proc_param *pdlg_proc_param;

	switch (msg) {

	case WM_CLOSE:
		DestroyWindow(hDlg);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case ID_SAVE:
			EnableWindow(GetDlgItem(hDlg, ID_SAVE), FALSE);
			if ((fp = fopen(g_config_file_name, "w+")) != NULL) {
				save_config(hDlg, fp);
				fclose(fp);
				stop_civetweb();
				start_civetweb(__argc, __argv);
			}
			EnableWindow(GetDlgItem(hDlg, ID_SAVE), TRUE);
			break;

		case ID_RESET_DEFAULTS:
			for (i = 0; default_options[i].name != NULL; i++) {
				name = default_options[i].name;
				value = default_options[i].default_value == NULL
				            ? ""
				            : default_options[i].default_value;
				if (default_options[i].type == MG_CONFIG_TYPE_BOOLEAN) {
					CheckDlgButton(hDlg,
					               ID_CONTROLS + i,
					               !strcmp(value, "yes") ? BST_CHECKED
					                                     : BST_UNCHECKED);
				} else {
					SetWindowText(GetDlgItem(hDlg, ID_CONTROLS + i), value);
				}
			}
			break;

		case ID_RESET_FILE:
			read_config_file(g_config_file_name, file_options);
			for (i = 0; default_options[i].name != NULL; i++) {
				name = default_options[i].name;
				value = default_options[i].default_value;
				for (j = 0; file_options[j * 2] != NULL; j++) {
					if (!strcmp(name, file_options[j * 2])) {
						value = file_options[j * 2 + 1];
					}
				}
				if (value == NULL) {
					value = "";
				}
				if (default_options[i].type == MG_CONFIG_TYPE_BOOLEAN) {
					CheckDlgButton(hDlg,
					               ID_CONTROLS + i,
					               !strcmp(value, "yes") ? BST_CHECKED
					                                     : BST_UNCHECKED);
				} else {
					SetWindowText(GetDlgItem(hDlg, ID_CONTROLS + i), value);
				}
			}
			for (i = 0; i < MAX_OPTIONS; i++) {
				free(file_options[2 * i]);
				free(file_options[2 * i + 1]);
			}
			break;

		case ID_RESET_ACTIVE:
			for (i = 0; default_options[i].name != NULL; i++) {
				name = default_options[i].name;
				value = mg_get_option(g_ctx, name);
				if (default_options[i].type == MG_CONFIG_TYPE_BOOLEAN) {
					CheckDlgButton(hDlg,
					               ID_CONTROLS + i,
					               !strcmp(value, "yes") ? BST_CHECKED
					                                     : BST_UNCHECKED);
				} else {
					SetDlgItemText(hDlg,
					               ID_CONTROLS + i,
					               value == NULL ? "" : value);
				}
			}
			break;
		}

		for (i = 0; default_options[i].name != NULL; i++) {
			name = default_options[i].name;
			if (((default_options[i].type == MG_CONFIG_TYPE_FILE)
			     || (default_options[i].type == MG_CONFIG_TYPE_DIRECTORY))
			    && LOWORD(wParam) == ID_CONTROLS + i + ID_FILE_BUTTONS_DELTA) {
				OPENFILENAME of;
				BROWSEINFO bi;
				char path[PATH_MAX] = "";

				memset(&of, 0, sizeof(of));
				of.lStructSize = sizeof(of);
				of.hwndOwner = (HWND)hDlg;
				of.lpstrFile = path;
				of.nMaxFile = sizeof(path);
				of.lpstrInitialDir = mg_get_option(g_ctx, "document_root");
				of.Flags =
				    OFN_CREATEPROMPT | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;

				memset(&bi, 0, sizeof(bi));
				bi.hwndOwner = (HWND)hDlg;
				bi.lpszTitle = "Choose WWW root directory:";
				bi.ulFlags = BIF_RETURNONLYFSDIRS;

				if (default_options[i].type == MG_CONFIG_TYPE_DIRECTORY) {
					SHGetPathFromIDList(SHBrowseForFolder(&bi), path);
				} else {
					GetOpenFileName(&of);
				}

				if (path[0] != '\0') {
					SetWindowText(GetDlgItem(hDlg, ID_CONTROLS + i), path);
				}
			}
		}
		break;

	case WM_INITDIALOG:
		/* Store hWnd in a parameter accessible by the parent, so we can
		 * bring this window to front if required. */
		pdlg_proc_param = (struct dlg_proc_param *)lParam;
		pdlg_proc_param->hWnd = hDlg;

		/* Initialize the dialog elements */
		SendMessage(hDlg, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon);
		SendMessage(hDlg, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
		title = (char *)malloc(strlen(g_server_name) + 16);
		if (title) {
			strcpy(title, g_server_name);
			strcat(title, " settings");
			SetWindowText(hDlg, title);
			free(title);
		}
		SetFocus(GetDlgItem(hDlg, ID_SAVE));

		/* Init dialog with active settings */
		SendMessage(hDlg, WM_COMMAND, ID_RESET_ACTIVE, 0);
		/* alternative: SendMessage(hDlg, WM_COMMAND, ID_RESET_FILE, 0); */
		break;

	default:
		break;
	}

	return FALSE;
}


/* Dialog proc for input dialog */
static INT_PTR CALLBACK
InputDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static struct dlg_proc_param *inBuf = 0;
	WORD ctrlId;
	HWND hIn;

	switch (msg) {
	case WM_CLOSE:
		inBuf = 0;
		DestroyWindow(hDlg);
		break;

	case WM_COMMAND:
		ctrlId = LOWORD(wParam);
		if (ctrlId == IDOK) {
			/* Get handle of input line */
			hIn = GetDlgItem(hDlg, ID_INPUT_LINE);

			if (hIn) {
				/* Get content of input line */
				GetWindowText(hIn, inBuf->buffer, (int)inBuf->buflen);
				if (inBuf->buffer[0] != 0) {
					/* Input dialog is not empty. */
					EndDialog(hDlg, IDOK);
				}
			} else {
				/* There is no input line in this dialog. */
				EndDialog(hDlg, IDOK);
			}

		} else if (ctrlId == IDRETRY) {

			/* Get handle of input line */
			hIn = GetDlgItem(hDlg, inBuf->idRetry);

			if (hIn) {
				/* Load current string */
				GetWindowText(hIn, inBuf->buffer, (int)inBuf->buflen);
				if (inBuf->fRetry) {
					if (inBuf->fRetry(inBuf)) {
						SetWindowText(hIn, inBuf->buffer);
					}
				}
			}

		} else if (ctrlId == IDCANCEL) {
			EndDialog(hDlg, IDCANCEL);
		}
		break;

	case WM_INITDIALOG:
		/* Get handle of input line */
		hIn = GetDlgItem(hDlg, ID_INPUT_LINE);

		/* Get dialog parameters */
		inBuf = (struct dlg_proc_param *)lParam;

		/* Set dialog handle for the caller */
		inBuf->hWnd = hDlg;

		/* Set dialog name */
		SetWindowText(hDlg, inBuf->name);

		if (hIn) {
			/* This is an input dialog */
			DEBUG_ASSERT(inBuf != NULL);
			DEBUG_ASSERT((inBuf->buffer != NULL) && (inBuf->buflen != 0));
			DEBUG_ASSERT(strlen(inBuf->buffer) < inBuf->buflen);
			SendMessage(hDlg, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon);
			SendMessage(hDlg, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
			SendMessage(hIn, EM_LIMITTEXT, inBuf->buflen - 1, 0);
			SetWindowText(hIn, inBuf->buffer);
			SetFocus(hIn);
		}

		break;

	default:
		break;
	}

	return FALSE;
}


static void
suggest_passwd(char *passwd)
{
	unsigned u;
	char *p;
	union {
		FILETIME ft;
		LARGE_INTEGER li;
	} num;

	/* valid characters are 32 to 126 */
	GetSystemTimeAsFileTime(&num.ft);
	num.li.HighPart |= (LONG)GetCurrentProcessId();
	p = passwd;
	while (num.li.QuadPart) {
		u = (unsigned)(num.li.QuadPart % 95);
		num.li.QuadPart -= u;
		num.li.QuadPart /= 95;
		*p = (char)(u + 32);
		p++;
	}
}


static void add_control(unsigned char **mem,
                        DLGTEMPLATE *dia,
                        WORD type,
                        WORD id,
                        DWORD style,
                        short x,
                        short y,
                        short cx,
                        short cy,
                        const char *caption);


static int
get_password(const char *user,
             const char *realm,
             char *passwd,
             unsigned passwd_len)
{
#define HEIGHT (15)
#define WIDTH (280)
#define LABEL_WIDTH (90)

	unsigned char mem[4096], *p;
	DLGTEMPLATE *dia = (DLGTEMPLATE *)mem;
	int ok;
	short y;
	static struct dlg_proc_param s_dlg_proc_param;

	const struct dlg_header_param dialog_header = GetDlgHeader(WIDTH);

	DEBUG_ASSERT((user != NULL) && (realm != NULL) && (passwd != NULL));

	/* Only allow one instance of this dialog to be open. */
	if (s_dlg_proc_param.guard == 0) {
		memset(&s_dlg_proc_param, 0, sizeof(s_dlg_proc_param));
		s_dlg_proc_param.guard = 1;
	} else {
		SetForegroundWindow(s_dlg_proc_param.hWnd);
		return 0;
	}

	/* Do not open a password dialog, if the username is empty */
	if (user[0] == 0) {
		s_dlg_proc_param.guard = 0;
		return 0;
	}

	/* Create a password suggestion */
	memset(passwd, 0, passwd_len);
	suggest_passwd(passwd);

	/* Make buffer available for input dialog */
	s_dlg_proc_param.buffer = passwd;
	s_dlg_proc_param.buflen = passwd_len;

	/* Create the dialog */
	(void)memset(mem, 0, sizeof(mem));
	p = mem;
	(void)memcpy(p, &dialog_header, sizeof(dialog_header));
	p = mem + sizeof(dialog_header);

	y = HEIGHT;
	add_control(&p,
	            dia,
	            0x82,
	            ID_STATIC,
	            WS_VISIBLE | WS_CHILD,
	            10,
	            y,
	            LABEL_WIDTH,
	            HEIGHT,
	            "User:");
	add_control(&p,
	            dia,
	            0x81,
	            ID_CONTROLS + 1,
	            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL
	                | ES_READONLY,
	            15 + LABEL_WIDTH,
	            y,
	            WIDTH - LABEL_WIDTH - 25,
	            HEIGHT,
	            user);

	y += HEIGHT;
	add_control(&p,
	            dia,
	            0x82,
	            ID_STATIC,
	            WS_VISIBLE | WS_CHILD,
	            10,
	            y,
	            LABEL_WIDTH,
	            HEIGHT,
	            "Realm:");
	add_control(&p,
	            dia,
	            0x81,
	            ID_CONTROLS + 2,
	            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL
	                | ES_READONLY,
	            15 + LABEL_WIDTH,
	            y,
	            WIDTH - LABEL_WIDTH - 25,
	            HEIGHT,
	            realm);

	y += HEIGHT;
	add_control(&p,
	            dia,
	            0x82,
	            ID_STATIC,
	            WS_VISIBLE | WS_CHILD,
	            10,
	            y,
	            LABEL_WIDTH,
	            HEIGHT,
	            "Password:");
	add_control(&p,
	            dia,
	            0x81,
	            ID_INPUT_LINE,
	            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
	            15 + LABEL_WIDTH,
	            y,
	            WIDTH - LABEL_WIDTH - 25,
	            HEIGHT,
	            "");

	y += (WORD)(HEIGHT * 2);
	add_control(&p,
	            dia,
	            0x80,
	            IDOK,
	            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
	            80,
	            y,
	            55,
	            12,
	            "Ok");
	add_control(&p,
	            dia,
	            0x80,
	            IDCANCEL,
	            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
	            140,
	            y,
	            55,
	            12,
	            "Cancel");

	DEBUG_ASSERT((intptr_t)p - (intptr_t)mem < (intptr_t)sizeof(mem));

	dia->cy = y + (WORD)(HEIGHT * 1.5);

	s_dlg_proc_param.name = "Modify password";
	s_dlg_proc_param.fRetry = NULL;

	ok = (IDOK
	      == DialogBoxIndirectParam(
	             NULL, dia, NULL, InputDlgProc, (LPARAM)&s_dlg_proc_param));

	s_dlg_proc_param.hWnd = NULL;
	s_dlg_proc_param.guard = 0;

	return ok;

#undef HEIGHT
#undef WIDTH
#undef LABEL_WIDTH
}


/* Dialog proc for password dialog */
static INT_PTR CALLBACK
PasswordDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static const char *passfile = 0;
	char domain[256], user[256], password[256];
	WORD ctrlId;
	struct dlg_proc_param *pdlg_proc_param;

	switch (msg) {
	case WM_CLOSE:
		passfile = 0;
		DestroyWindow(hDlg);
		break;

	case WM_COMMAND:
		ctrlId = LOWORD(wParam);
		if (ctrlId == ID_ADD_USER) {
			/* Add user */
			GetWindowText(GetDlgItem(hDlg, ID_ADD_USER_NAME),
			              user,
			              sizeof(user));
			GetWindowText(GetDlgItem(hDlg, ID_ADD_USER_REALM),
			              domain,
			              sizeof(domain));
			if (get_password(user, domain, password, sizeof(password))) {
				mg_modify_passwords_file(passfile, domain, user, password);
				EndDialog(hDlg, IDOK);
			}
		} else if ((ctrlId >= (ID_CONTROLS + ID_FILE_BUTTONS_DELTA * 3))
		           && (ctrlId < (ID_CONTROLS + ID_FILE_BUTTONS_DELTA * 4))) {
			/* Modify password */
			GetWindowText(GetDlgItem(hDlg, ctrlId - ID_FILE_BUTTONS_DELTA * 3),
			              user,
			              sizeof(user));
			GetWindowText(GetDlgItem(hDlg, ctrlId - ID_FILE_BUTTONS_DELTA * 2),
			              domain,
			              sizeof(domain));
			if (get_password(user, domain, password, sizeof(password))) {
				mg_modify_passwords_file(passfile, domain, user, password);
				EndDialog(hDlg, IDOK);
			}
		} else if ((ctrlId >= (ID_CONTROLS + ID_FILE_BUTTONS_DELTA * 2))
		           && (ctrlId < (ID_CONTROLS + ID_FILE_BUTTONS_DELTA * 3))) {
			/* Remove user */
			GetWindowText(GetDlgItem(hDlg, ctrlId - ID_FILE_BUTTONS_DELTA * 2),
			              user,
			              sizeof(user));
			GetWindowText(GetDlgItem(hDlg, ctrlId - ID_FILE_BUTTONS_DELTA),
			              domain,
			              sizeof(domain));
			mg_modify_passwords_file(passfile, domain, user, NULL);
			EndDialog(hDlg, IDOK);
		}
		break;

	case WM_INITDIALOG:
		pdlg_proc_param = (struct dlg_proc_param *)lParam;
		pdlg_proc_param->hWnd = hDlg;
		passfile = pdlg_proc_param->name;
		SendMessage(hDlg, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon);
		SendMessage(hDlg, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
		SetWindowText(hDlg, passfile);
		SetFocus(GetDlgItem(hDlg, ID_ADD_USER_NAME));
		break;

	default:
		break;
	}

	return FALSE;
}


static void
add_control(unsigned char **mem,
            DLGTEMPLATE *dia,
            WORD type,
            WORD id,
            DWORD style,
            short x,
            short y,
            short cx,
            short cy,
            const char *caption)
{
	DLGITEMTEMPLATE *tp;
	LPWORD p;

	dia->cdit++;

	*mem = (unsigned char *)align(*mem, 3);
	tp = (DLGITEMTEMPLATE *)*mem;

	tp->id = id;
	tp->style = style;
	tp->dwExtendedStyle = 0;
	tp->x = x;
	tp->y = y;
	tp->cx = cx;
	tp->cy = cy;

	p = (LPWORD)align(*mem + sizeof(*tp), 1);
	*p++ = 0xffff;
	*p++ = type;

	while (*caption != '\0') {
		*p++ = (WCHAR)*caption++;
	}
	*p++ = 0;
	p = (LPWORD)align(p, 1);

	*p++ = 0;
	*mem = (unsigned char *)p;
}


static void
show_settings_dialog()
{
#define HEIGHT (15)
#define WIDTH (460)
#define LABEL_WIDTH (90)

	unsigned char mem[16 * 1024], *p;
	const struct mg_option *options;
	DWORD style;
	DLGTEMPLATE *dia = (DLGTEMPLATE *)mem;
	WORD i, cl, nelems = 0;
	short width, x, y;
	static struct dlg_proc_param s_dlg_proc_param;

	const struct dlg_header_param dialog_header = GetDlgHeader(WIDTH);

	if (s_dlg_proc_param.guard == 0) {
		memset(&s_dlg_proc_param, 0, sizeof(s_dlg_proc_param));
		s_dlg_proc_param.guard = 1;
	} else {
		SetForegroundWindow(s_dlg_proc_param.hWnd);
		return;
	}

	(void)memset(mem, 0, sizeof(mem));
	p = mem;
	(void)memcpy(p, &dialog_header, sizeof(dialog_header));
	p = mem + sizeof(dialog_header);

	options = mg_get_valid_options();
	for (i = 0; options[i].name != NULL; i++) {
		style = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
		x = 10 + (WIDTH / 2) * (nelems % 2);
		y = (nelems / 2 + 1) * HEIGHT + 5;
		width = WIDTH / 2 - 20 - LABEL_WIDTH;
		if (options[i].type == MG_CONFIG_TYPE_NUMBER) {
			style |= ES_NUMBER;
			cl = 0x81;
			style |= WS_BORDER | ES_AUTOHSCROLL;
		} else if (options[i].type == MG_CONFIG_TYPE_BOOLEAN) {
			cl = 0x80;
			style |= BS_AUTOCHECKBOX;
		} else if ((options[i].type == MG_CONFIG_TYPE_FILE)
		           || (options[i].type == MG_CONFIG_TYPE_DIRECTORY)) {
			style |= WS_BORDER | ES_AUTOHSCROLL;
			width -= 20;
			cl = 0x81;
			add_control(&p,
			            dia,
			            0x80,
			            ID_CONTROLS + i + ID_FILE_BUTTONS_DELTA,
			            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			            x + width + LABEL_WIDTH + 5,
			            y,
			            15,
			            12,
			            "...");
		} else if (options[i].type == MG_CONFIG_TYPE_STRING_MULTILINE) {
			/* TODO: This is not really uer friendly */
			cl = 0x81;
			style |= WS_BORDER | ES_AUTOHSCROLL | ES_MULTILINE | ES_WANTRETURN
			         | ES_AUTOVSCROLL;
		} else {
			cl = 0x81;
			style |= WS_BORDER | ES_AUTOHSCROLL;
		}
		add_control(&p,
		            dia,
		            0x82,
		            ID_STATIC,
		            WS_VISIBLE | WS_CHILD,
		            x,
		            y,
		            LABEL_WIDTH,
		            HEIGHT,
		            options[i].name);
		add_control(&p,
		            dia,
		            cl,
		            ID_CONTROLS + i,
		            style,
		            x + LABEL_WIDTH,
		            y,
		            width,
		            12,
		            "");
		nelems++;

		DEBUG_ASSERT(((intptr_t)p - (intptr_t)mem) < (intptr_t)sizeof(mem));
	}

	y = (((nelems + 1) / 2 + 1) * HEIGHT + 5);
	add_control(&p,
	            dia,
	            0x80,
	            ID_GROUP,
	            WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
	            5,
	            5,
	            WIDTH - 10,
	            y,
	            " Settings ");
	y += 10;
	add_control(&p,
	            dia,
	            0x80,
	            ID_SAVE,
	            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
	            WIDTH - 70,
	            y,
	            65,
	            12,
	            "Save Settings");
	add_control(&p,
	            dia,
	            0x80,
	            ID_RESET_DEFAULTS,
	            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
	            WIDTH - 140,
	            y,
	            65,
	            12,
	            "Reset to defaults");
	add_control(&p,
	            dia,
	            0x80,
	            ID_RESET_FILE,
	            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
	            WIDTH - 210,
	            y,
	            65,
	            12,
	            "Reload from file");
	add_control(&p,
	            dia,
	            0x80,
	            ID_RESET_ACTIVE,
	            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
	            WIDTH - 280,
	            y,
	            65,
	            12,
	            "Reload active");
	add_control(&p,
	            dia,
	            0x82,
	            ID_STATIC,
	            WS_CHILD | WS_VISIBLE | WS_DISABLED,
	            5,
	            y,
	            100,
	            12,
	            g_server_base_name);

	DEBUG_ASSERT(((intptr_t)p - (intptr_t)mem) < (intptr_t)sizeof(mem));

	dia->cy = ((nelems + 1) / 2 + 1) * HEIGHT + 30;

	s_dlg_proc_param.fRetry = NULL;

	DialogBoxIndirectParam(
	    NULL, dia, NULL, SettingsDlgProc, (LPARAM)&s_dlg_proc_param);

	s_dlg_proc_param.hWnd = NULL;
	s_dlg_proc_param.guard = 0;

#undef HEIGHT
#undef WIDTH
#undef LABEL_WIDTH
}


static void
change_password_file()
{
#define HEIGHT (15)
#define WIDTH (320)
#define LABEL_WIDTH (90)

	OPENFILENAME of;
	char path[PATH_MAX] = PASSWORDS_FILE_NAME;
	char strbuf[256], u[256], d[256];
	HWND hDlg = NULL;
	FILE *f;
	short y, nelems;
	unsigned char mem[4096], *p;
	DLGTEMPLATE *dia = (DLGTEMPLATE *)mem;
	const char *domain = mg_get_option(g_ctx, "authentication_domain");
	static struct dlg_proc_param s_dlg_proc_param;

	const struct dlg_header_param dialog_header = GetDlgHeader(WIDTH);

	if (s_dlg_proc_param.guard == 0) {
		memset(&s_dlg_proc_param, 0, sizeof(s_dlg_proc_param));
		s_dlg_proc_param.guard = 1;
	} else {
		SetForegroundWindow(s_dlg_proc_param.hWnd);
		return;
	}

	memset(&of, 0, sizeof(of));
	of.lStructSize = sizeof(of);
	of.hwndOwner = (HWND)hDlg;
	of.lpstrFile = path;
	of.nMaxFile = sizeof(path);
	of.lpstrInitialDir = mg_get_option(g_ctx, "document_root");
	of.Flags = OFN_CREATEPROMPT | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;

	if (!GetSaveFileName(&of)) {
		/* Cancel/Close by user */
		s_dlg_proc_param.guard = 0;
		return;
	}

	f = fopen(path, "a+");
	if (f) {
		fclose(f);
	} else {
		MessageBox(NULL, path, "Can not open file", MB_ICONERROR);
		s_dlg_proc_param.guard = 0;
		return;
	}

	do {
		s_dlg_proc_param.hWnd = NULL;
		(void)memset(mem, 0, sizeof(mem));
		p = mem;
		(void)memcpy(p, &dialog_header, sizeof(dialog_header));
		p = mem + sizeof(dialog_header);

		f = fopen(path, "r+");
		if (!f) {
			MessageBox(NULL, path, "Can not open file", MB_ICONERROR);
			s_dlg_proc_param.guard = 0;
			return;
		}

		nelems = 0;
		while (fgets(strbuf, sizeof(strbuf), f)) {
			if (sscanf(strbuf, "%255[^:]:%255[^:]:%*s", u, d) != 2) {
				continue;
			}
			u[255] = 0;
			d[255] = 0;
			y = (nelems + 1) * HEIGHT + 5;
			add_control(&p,
			            dia,
			            0x80,
			            ID_CONTROLS + nelems + ID_FILE_BUTTONS_DELTA * 3,
			            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
			            10,
			            y,
			            65,
			            12,
			            "Modify password");
			add_control(&p,
			            dia,
			            0x80,
			            ID_CONTROLS + nelems + ID_FILE_BUTTONS_DELTA * 2,
			            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
			            80,
			            y,
			            55,
			            12,
			            "Remove user");
			add_control(&p,
			            dia,
			            0x81,
			            ID_CONTROLS + nelems + ID_FILE_BUTTONS_DELTA,
			            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL
			                | ES_READONLY,
			            245,
			            y,
			            60,
			            12,
			            d);
			add_control(&p,
			            dia,
			            0x81,
			            ID_CONTROLS + nelems,
			            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL
			                | ES_READONLY,
			            140,
			            y,
			            100,
			            12,
			            u);

			nelems++;
			DEBUG_ASSERT(((intptr_t)p - (intptr_t)mem) < (intptr_t)sizeof(mem));
		}
		fclose(f);

		y = (nelems + 1) * HEIGHT + 10;
		add_control(&p,
		            dia,
		            0x80,
		            ID_ADD_USER,
		            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		            80,
		            y,
		            55,
		            12,
		            "Add user");
		add_control(&p,
		            dia,
		            0x81,
		            ID_ADD_USER_NAME,
		            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL
		                | WS_TABSTOP,
		            140,
		            y,
		            100,
		            12,
		            "");
		add_control(&p,
		            dia,
		            0x81,
		            ID_ADD_USER_REALM,
		            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL
		                | WS_TABSTOP,
		            245,
		            y,
		            60,
		            12,
		            domain);

		y = (nelems + 2) * HEIGHT + 10;
		add_control(&p,
		            dia,
		            0x80,
		            ID_GROUP,
		            WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
		            5,
		            5,
		            WIDTH - 10,
		            y,
		            " Users ");

		y += HEIGHT;
		add_control(&p,
		            dia,
		            0x82,
		            ID_STATIC,
		            WS_CHILD | WS_VISIBLE | WS_DISABLED,
		            5,
		            y,
		            100,
		            12,
		            g_server_base_name);

		DEBUG_ASSERT(((intptr_t)p - (intptr_t)mem) < (intptr_t)sizeof(mem));

		dia->cy = y + 20;

		s_dlg_proc_param.name = path;
		s_dlg_proc_param.fRetry = NULL;

	} while (
	    (IDOK
	     == DialogBoxIndirectParam(
	            NULL, dia, NULL, PasswordDlgProc, (LPARAM)&s_dlg_proc_param))
	    && (!g_exit_flag));

	s_dlg_proc_param.hWnd = NULL;
	s_dlg_proc_param.guard = 0;

#undef HEIGHT
#undef WIDTH
#undef LABEL_WIDTH
}


static BOOL
sysinfo_reload(struct dlg_proc_param *prm)
{
	static char *buf = 0;
	int cl, rl;

	cl = mg_get_context_info(g_ctx, NULL, 0);
	free(buf);
	cl += 510;
	buf = (char *)malloc(cl + 1);
	rl = mg_get_context_info(g_ctx, buf, cl);
	if ((rl > cl) || (rl <= 0)) {
		if (g_ctx == NULL) {
			prm->buffer = "Server not running";
		} else if (rl <= 0) {
			prm->buffer = "No server statistics available";
		} else {
			prm->buffer = "Please retry";
		}
	} else {
		prm->buffer = buf;
	}

	return TRUE;
}


int
show_system_info()
{
#define HEIGHT (15)
#define WIDTH (320)
#define LABEL_WIDTH (50)

	unsigned char mem[4096], *p;
	DLGTEMPLATE *dia = (DLGTEMPLATE *)mem;
	int ok;
	short y;
	static struct dlg_proc_param s_dlg_proc_param;

	const struct dlg_header_param dialog_header = GetDlgHeader(WIDTH);

	/* Only allow one instance of this dialog to be open. */
	if (s_dlg_proc_param.guard == 0) {
		memset(&s_dlg_proc_param, 0, sizeof(s_dlg_proc_param));
		s_dlg_proc_param.guard = 1;
	} else {
		SetForegroundWindow(s_dlg_proc_param.hWnd);
		return 0;
	}

	/* Create the dialog */
	(void)memset(mem, 0, sizeof(mem));
	p = mem;
	(void)memcpy(p, &dialog_header, sizeof(dialog_header));
	p = mem + sizeof(dialog_header);

	y = HEIGHT;
	add_control(&p,
	            dia,
	            0x82,
	            ID_STATIC,
	            WS_VISIBLE | WS_CHILD,
	            10,
	            y,
	            LABEL_WIDTH,
	            HEIGHT,
	            "System Information:");
	add_control(&p,
	            dia,
	            0x81,
	            ID_CONTROLS + 1,
	            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL
	                | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY,
	            15 + LABEL_WIDTH,
	            y,
	            WIDTH - LABEL_WIDTH - 25,
	            HEIGHT * 7,
	            g_system_info);

	y += (WORD)(HEIGHT * 8);

	add_control(&p,
	            dia,
	            0x80,
	            IDRETRY,
	            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
	            WIDTH - 10 - 55 - 10 - 55,
	            y,
	            55,
	            12,
	            "Reload");

	add_control(&p,
	            dia,
	            0x80,
	            IDOK,
	            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
	            WIDTH - 10 - 55,
	            y,
	            55,
	            12,
	            "Close");

	DEBUG_ASSERT((intptr_t)p - (intptr_t)mem < (intptr_t)sizeof(mem));

	dia->cy = y + (WORD)(HEIGHT * 1.5);

	s_dlg_proc_param.name = "System information";
	s_dlg_proc_param.fRetry = sysinfo_reload;
	s_dlg_proc_param.idRetry = ID_CONTROLS + 1; /* Reload field with this ID */

	ok = (IDOK
	      == DialogBoxIndirectParam(
	             NULL, dia, NULL, InputDlgProc, (LPARAM)&s_dlg_proc_param));

	s_dlg_proc_param.hWnd = NULL;
	s_dlg_proc_param.guard = 0;

	return ok;

#undef HEIGHT
#undef WIDTH
#undef LABEL_WIDTH
}


static int
manage_service(int action)
{
	const char *service_name = g_server_name;
	SC_HANDLE hSCM = NULL, hService = NULL;
	SERVICE_DESCRIPTION descr;
	char path[PATH_MAX + 20] = ""; /* Path to executable plus magic argument */
	int success = 1;

	descr.lpDescription = (LPSTR)g_server_name;

	if ((hSCM = OpenSCManager(NULL,
	                          NULL,
	                          action == ID_INSTALL_SERVICE ? GENERIC_WRITE
	                                                       : GENERIC_READ))
	    == NULL) {
		success = 0;
		show_error();
	} else if (action == ID_INSTALL_SERVICE) {
		path[sizeof(path) - 1] = 0;
		GetModuleFileName(NULL, path, sizeof(path) - 1);
		strncat(path, " ", sizeof(path) - 1 - strlen(path));
		strncat(path, service_magic_argument, sizeof(path) - 1 - strlen(path));
		hService = CreateService(hSCM,
		                         service_name,
		                         service_name,
		                         SERVICE_ALL_ACCESS,
		                         SERVICE_WIN32_OWN_PROCESS,
		                         SERVICE_AUTO_START,
		                         SERVICE_ERROR_NORMAL,
		                         path,
		                         NULL,
		                         NULL,
		                         NULL,
		                         NULL,
		                         NULL);
		if (hService) {
			ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &descr);
		} else {
			show_error();
		}
	} else if (action == ID_REMOVE_SERVICE) {
		if ((hService = OpenService(hSCM, service_name, DELETE)) == NULL
		    || !DeleteService(hService)) {
			show_error();
		}
	} else if ((hService =
	                OpenService(hSCM, service_name, SERVICE_QUERY_STATUS))
	           == NULL) {
		success = 0;
	}

	if (hService)
		CloseServiceHandle(hService);
	if (hSCM)
		CloseServiceHandle(hSCM);

	return success;
}


/* Window proc for taskbar icon */
static LRESULT CALLBACK
WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	int service_installed;
	char buf[200];
	POINT pt;
	HMENU hMenu;
	static UINT s_uTaskbarRestart; /* for taskbar creation */

	switch (msg) {

	case WM_CREATE:
		if ((__argv[1] != NULL) && !strcmp(__argv[1], service_magic_argument)) {
			static SERVICE_TABLE_ENTRY service_table[2];
			char *service_argv[2];

			service_argv[0] = __argv[0];
			service_argv[1] = NULL;

			start_civetweb(1, service_argv);

			memset(service_table, 0, sizeof(service_table));
			service_table[0].lpServiceName = (LPSTR)g_server_name;
			service_table[0].lpServiceProc =
			    (LPSERVICE_MAIN_FUNCTION)ServiceMain;

			StartServiceCtrlDispatcher(service_table);
			exit(EXIT_SUCCESS);
		} else {
			start_civetweb(__argc, __argv);
			s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_QUIT:
			stop_civetweb();
			Shell_NotifyIcon(NIM_DELETE, &TrayIcon);
			g_exit_flag = 1;
			PostQuitMessage(0);
			return 0;
		case ID_SETTINGS:
			show_settings_dialog();
			break;
		case ID_PASSWORD:
			change_password_file();
			break;
		case ID_SYSINFO:
			show_system_info();
			break;
		case ID_INSTALL_SERVICE:
		case ID_REMOVE_SERVICE:
			manage_service(LOWORD(wParam));
			break;
		case ID_CONNECT:
			fprintf(stdout, "[%s]\n", get_url_to_first_open_port(g_ctx));
			ShellExecute(NULL,
			             "open",
			             get_url_to_first_open_port(g_ctx),
			             NULL,
			             NULL,
			             SW_SHOW);
			break;
		case ID_WEBSITE:
			fprintf(stdout, "[%s]\n", g_website);
			ShellExecute(NULL, "open", g_website, NULL, NULL, SW_SHOW);
			break;
		}
		break;

	case WM_USER:
		switch (lParam) {
		case WM_RBUTTONUP:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
			hMenu = CreatePopupMenu();
			AppendMenu(hMenu,
			           MF_STRING | MF_GRAYED,
			           ID_SEPARATOR,
			           g_server_name);
			AppendMenu(hMenu, MF_SEPARATOR, ID_SEPARATOR, "");
			service_installed = manage_service(0);
			snprintf(buf,
			         sizeof(buf) - 1,
			         "NT service: %s installed",
			         service_installed ? "" : "not");
			buf[sizeof(buf) - 1] = 0;
			AppendMenu(hMenu, MF_STRING | MF_GRAYED, ID_SEPARATOR, buf);
			AppendMenu(hMenu,
			           MF_STRING | (service_installed ? MF_GRAYED : 0),
			           ID_INSTALL_SERVICE,
			           "Install service");
			AppendMenu(hMenu,
			           MF_STRING | (!service_installed ? MF_GRAYED : 0),
			           ID_REMOVE_SERVICE,
			           "Deinstall service");
			AppendMenu(hMenu, MF_SEPARATOR, ID_SEPARATOR, "");
			AppendMenu(hMenu, MF_STRING, ID_CONNECT, "Start browser");
			AppendMenu(hMenu, MF_STRING, ID_SETTINGS, "Edit settings");
			AppendMenu(hMenu, MF_STRING, ID_PASSWORD, "Modify password file");
			AppendMenu(hMenu, MF_STRING, ID_SYSINFO, "Show system info");
			AppendMenu(hMenu, MF_STRING, ID_WEBSITE, "Visit website");
			AppendMenu(hMenu, MF_SEPARATOR, ID_SEPARATOR, "");
			AppendMenu(hMenu, MF_STRING, ID_QUIT, "Exit");
			GetCursorPos(&pt);
			SetForegroundWindow(hWnd);
			TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, hWnd, NULL);
			PostMessage(hWnd, WM_NULL, 0, 0);
			DestroyMenu(hMenu);
			break;
		}
		break;

	case WM_CLOSE:
		stop_civetweb();
		Shell_NotifyIcon(NIM_DELETE, &TrayIcon);
		g_exit_flag = 1;
		PostQuitMessage(0);
		return 0; /* We've just sent our own quit message, with proper hwnd. */

	default:
		if (msg == s_uTaskbarRestart)
			Shell_NotifyIcon(NIM_ADD, &TrayIcon);
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


static int
MakeConsole(void)
{
	DWORD err;
	HANDLE hConWnd = GetConsoleWindow();

	if (hConWnd == NULL) {
		if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
			FreeConsole();
			if (!AllocConsole()) {
				err = GetLastError();
				if (err == ERROR_ACCESS_DENIED) {
					MessageBox(NULL,
					           "Insufficient rights to create a console window",
					           "Error",
					           MB_ICONERROR);
				}
			}
			AttachConsole(GetCurrentProcessId());
		}

		/* Retry to get a console handle */
		hConWnd = GetConsoleWindow();

		if (hConWnd != NULL) {
			/* Reopen console handles according to
			 * https://stackoverflow.com/questions/9020790/using-stdin-with-an-allocconsole
			 */
			freopen("CONIN$", "r", stdin);
			freopen("CONOUT$", "w", stdout);
			freopen("CONOUT$", "w", stderr);
		}
	}

	if (hConWnd != NULL) {
		SetConsoleTitle(g_server_name);
	}

	return (hConWnd != NULL);
}


int WINAPI
WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmdline, int show)
{
	WNDCLASS cls;
	HWND hWnd;
	MSG msg;

#if defined(DEBUG)
	(void)MakeConsole();
#endif

	(void)hInst;
	(void)hPrev;
	(void)cmdline;
	(void)show;

	init_server_name();
	init_system_info();
	memset(&cls, 0, sizeof(cls));
	cls.lpfnWndProc = (WNDPROC)WindowProc;
	cls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	cls.lpszClassName = g_server_base_name;

	RegisterClass(&cls);
	hWnd = CreateWindow(cls.lpszClassName,
	                    g_server_name,
	                    WS_OVERLAPPEDWINDOW,
	                    0,
	                    0,
	                    0,
	                    0,
	                    NULL,
	                    NULL,
	                    NULL,
	                    NULL);
	ShowWindow(hWnd, SW_HIDE);

	if (g_icon_name) {
		hIcon = (HICON)
		    LoadImage(NULL, g_icon_name, IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
	} else {
		hIcon = (HICON)LoadImage(GetModuleHandle(NULL),
		                         MAKEINTRESOURCE(ID_ICON),
		                         IMAGE_ICON,
		                         16,
		                         16,
		                         0);
	}

	TrayIcon.cbSize = sizeof(TrayIcon);
	TrayIcon.uID = ID_ICON;
	TrayIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	TrayIcon.hIcon = hIcon;
	TrayIcon.hWnd = hWnd;
	snprintf(TrayIcon.szTip, sizeof(TrayIcon.szTip), "%s", g_server_name);
	TrayIcon.uCallbackMessage = WM_USER;
	Shell_NotifyIcon(NIM_ADD, &TrayIcon);

	while (GetMessage(&msg, hWnd, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	free_system_info();

	/* Return the WM_QUIT value. */
	return (int)msg.wParam;
}


int
main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	return WinMain(0, 0, 0, 0);
}


#elif defined(USE_COCOA)
#import <Cocoa/Cocoa.h>

@interface Civetweb : NSObject <NSApplicationDelegate>
- (void)openBrowser;
- (void)shutDown;
@end

@implementation Civetweb
- (void)openBrowser {
	[[NSWorkspace sharedWorkspace]
	    openURL:[NSURL URLWithString:[NSString stringWithUTF8String:
	                                               get_url_to_first_open_port(
	                                                   g_ctx)]]];
}
- (void)editConfig {
	create_config_file(g_ctx, g_config_file_name);
	NSString *path = [NSString stringWithUTF8String:g_config_file_name];
	if (![[NSWorkspace sharedWorkspace] openFile:path
	                             withApplication:@"TextEdit"]) {
		NSAlert *alert = [[[NSAlert alloc] init] autorelease];
		[alert setAlertStyle:NSWarningAlertStyle];
		[alert setMessageText:NSLocalizedString(@"Unable to open config file.",
		                                        "")];
		[alert setInformativeText:path];
		(void)[alert runModal];
	}
}
- (void)shutDown {
	[NSApp terminate:nil];
}
@end

int
main(int argc, char *argv[])
{
	init_server_name();
	init_system_info();
	start_civetweb(argc, argv);

	[NSAutoreleasePool new];
	[NSApplication sharedApplication];

	/* Add delegate to process menu item actions */
	Civetweb *myDelegate = [[Civetweb alloc] autorelease];
	[NSApp setDelegate:myDelegate];

	/* Run this app as agent */
	ProcessSerialNumber psn = {0, kCurrentProcess};
	TransformProcessType(&psn, kProcessTransformToBackgroundApplication);
	SetFrontProcess(&psn);

	/* Add status bar menu */
	id menu = [[NSMenu new] autorelease];

	/* Add version menu item */
	[menu
	    addItem:[[[NSMenuItem alloc]
	                /*initWithTitle:[NSString stringWithFormat:@"%s",
	                   server_name]*/
	                initWithTitle:[NSString stringWithUTF8String:g_server_name]
	                       action:@selector(noexist)
	                keyEquivalent:@""] autorelease]];

	/* Add configuration menu item */
	[menu addItem:[[[NSMenuItem alloc] initWithTitle:@"Edit configuration"
	                                          action:@selector(editConfig)
	                                   keyEquivalent:@""] autorelease]];

	/* Add connect menu item */
	[menu
	    addItem:[[[NSMenuItem alloc] initWithTitle:@"Open web root in a browser"
	                                        action:@selector(openBrowser)
	                                 keyEquivalent:@""] autorelease]];

	/* Separator */
	[menu addItem:[NSMenuItem separatorItem]];

	/* Add quit menu item */
	[menu addItem:[[[NSMenuItem alloc] initWithTitle:@"Quit"
	                                          action:@selector(shutDown)
	                                   keyEquivalent:@"q"] autorelease]];

	/* Attach menu to the status bar */
	id item = [[[NSStatusBar systemStatusBar]
	    statusItemWithLength:NSVariableStatusItemLength] retain];
	[item setHighlightMode:YES];
	[item setImage:[NSImage imageNamed:@"civetweb_22x22.png"]];
	[item setMenu:menu];

	/* Run the app */
	[NSApp activateIgnoringOtherApps:YES];
	[NSApp run];

	stop_civetweb();
	free_system_info();

	return EXIT_SUCCESS;
}

#else

int
main(int argc, char *argv[])
{
	init_server_name();
	init_system_info();
	start_civetweb(argc, argv);
	fprintf(stdout,
	        "%s started on port(s) %s with web root [%s]\n",
	        g_server_name,
	        mg_get_option(g_ctx, "listening_ports"),
	        mg_get_option(g_ctx, "document_root"));

	while (g_exit_flag == 0) {
		sleep(1);
	}

	fprintf(stdout,
	        "Exiting on signal %d, waiting for all threads to finish...",
	        g_exit_flag);
	fflush(stdout);
	stop_civetweb();
	fprintf(stdout, "%s", " done.\n");

	free_system_info();

	return EXIT_SUCCESS;
}
#endif /* _WIN32 */
