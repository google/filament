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

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS /* Microsoft nonsense */
#endif

#include "civetweb_check.h"
#include "private.h"
#include "private_exe.h"
#include "public_func.h"
#include "public_server.h"
#include "shared.h"
#include "timertest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* This unit test file uses the excellent Check unit testing library.
 * The API documentation is available here:
 * http://check.sourceforge.net/doc/check_html/index.html
 *
 * Note: CivetWeb is tested using it's own fork of check:
 * https://github.com/civetweb/check
 * Required fixes from this fork are already available
 * in the main repository:
 * https://github.com/libcheck/check
 */

#define FILENAME_LEN (128)

int
main(const int argc, char *argv[])
{
	/* Supported command line arguments */
	const char *const suite_arg = "--suite=";
	const size_t suite_arg_size = strlen(suite_arg);
	const char *const test_case_arg = "--test-case=";
	const size_t test_case_arg_size = strlen(test_case_arg);
	const char *const test_dir_arg = "--test-dir=";
	const size_t test_dir_arg_size = strlen(test_dir_arg);
	const char *const test_log_arg = "--test-log=";
	const size_t test_log_arg_size = strlen(test_log_arg);
	const char *const help_arg = "--help";

	/* Test variables */
	const char *suite = NULL;
	const char *test_case = NULL;
	SRunner *srunner;
	int number_run = 0;
	int number_failed = 0;
	const char *test_log_prefix = NULL;
	char test_log_name[FILENAME_LEN];
	char test_xml_name[FILENAME_LEN];

	int i;

	/* Check command line parameters for tests */
	for (i = 1; i < argc; ++i) {
		if (0 == strncmp(suite_arg, argv[i], suite_arg_size)
		    && (strlen(argv[i]) > suite_arg_size)) {
			suite = &argv[i][suite_arg_size];
		} else if (0 == strncmp(test_case_arg, argv[i], test_case_arg_size)
		           && (strlen(argv[i]) > test_case_arg_size)) {
			test_case = &argv[i][test_case_arg_size];
		} else if (0 == strncmp(test_dir_arg, argv[i], test_dir_arg_size)
		           && (strlen(argv[i]) > test_dir_arg_size)) {
			set_test_directory(&argv[i][test_dir_arg_size]);
		} else if (0 == strncmp(test_log_arg, argv[i], test_log_arg_size)
		           && (strlen(argv[i]) > test_log_arg_size)) {
			test_log_prefix = &argv[i][test_log_arg_size];
			if ((strlen(test_log_prefix) + 16) > FILENAME_LEN) {
				fprintf(stderr, "Argument too long: %s\n", argv[i]);
				exit(EXIT_FAILURE);
			}
		} else if (0 == strcmp(help_arg, argv[i])) {
			printf(
			    "Usage: %s [options]\n"
			    "  --suite=Suite            Determines the suite to run\n"
			    "  --test-case='Test Case'  Determines the test case to run\n"
			    "  --test-dir='folder/path' The location of the test directory "
			    "with the \n"
			    "                           'fixtures' and 'expected\n",
			    argv[0]);
			exit(EXIT_SUCCESS);
		} else {
			fprintf(stderr, "Invalid argument: %s\n", argv[i]);
			exit(EXIT_FAILURE);
		}
	}

	/* Register all tests to run them later */
	srunner = srunner_create(make_public_func_suite());
	srunner_add_suite(srunner, make_public_server_suite());
	srunner_add_suite(srunner, make_private_suite());
	srunner_add_suite(srunner, make_private_exe_suite());
	srunner_add_suite(srunner, make_timertest_suite());

	/* Write test logs to a file */
	if (test_log_prefix == NULL) {
		/* Find the next free log name */
		FILE *f;
		for (i = 1;; i++) {
			/* enumerate all log files (8.3 filename using 3 digits) */
			sprintf(test_log_name, "test-%03i.log", i);
			f = fopen(test_log_name, "r");
			if (f) {
				/* file already exists */
				fclose(f);
				/* try next name */
				continue;
			}
			/* file does not exist - use this name */
			srunner_set_log(srunner, test_log_name);
			/* use the same index for xml as for log */
			sprintf(test_xml_name, "test-%03i.xml", i);
			srunner_set_xml(srunner, test_xml_name);
			break;
		}
	} else {
		/* We got a test log name from the command line */
		sprintf(test_log_name, "%s.log", test_log_prefix);
		srunner_set_log(srunner, test_log_name);
		sprintf(test_xml_name, "%s.xml", test_log_prefix);
		srunner_set_xml(srunner, test_xml_name);
	}

	/* Run tests, using log level CK_VERBOSE, since CK_NORMAL
	 * offers not enough diagnosis to analyze failed tests.
	 * see http://check.sourceforge.net/doc/check_html/check_3.html */
	srunner_run(srunner, suite, test_case, CK_VERBOSE);

	/* Check passed / failed */
	number_run = srunner_ntests_run(srunner);
	number_failed = srunner_ntests_failed(srunner);
	srunner_free(srunner);
	return ((number_failed == 0) && (number_run != 0)) ? EXIT_SUCCESS
	                                                   : EXIT_FAILURE;
}
