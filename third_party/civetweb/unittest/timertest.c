/* Copyright (c) 2016-2018 the Civetweb developers
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

/**
 * We include the source file so that we have access to the internal private
 * static functions
 */
#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#define CIVETWEB_API static
#define USE_TIMERS

#include "../src/civetweb.c"

#include <stdlib.h>
#include <time.h>

#include "timertest.h"

static int action_dec_ret;

static int
action_dec(void *arg)
{
	int *p = (int *)arg;
	(*p)--;

	if (*p < -1) {
		ck_abort_msg("Periodic timer called too often");
		/* return 0 here would be unreachable code */
	}

	return (*p >= -3) ? action_dec_ret : 0;
}


static int
action_dec_to_0(void *arg)
{
	int *p = (int *)arg;
	(*p)--;

	if (*p <= -1) {
		ck_abort_msg("Periodic timer called too often");
		/* return 0 here would be unreachable code */
	}

	return (*p > 0);
}


START_TEST(test_timer_cyclic)
{
	struct mg_context ctx;
	int c[10];
	memset(&ctx, 0, sizeof(ctx));
	memset(c, 0, sizeof(c));

	action_dec_ret = 1;

	mark_point();
	timers_init(&ctx);
	mg_sleep(100);
	mark_point();

	c[0] = 100;
	timer_add(&ctx, 0.05, 0.1, 1, action_dec, c + 0);
	c[2] = 20;
	timer_add(&ctx, 0.25, 0.5, 1, action_dec, c + 2);
	c[1] = 50;
	timer_add(&ctx, 0.1, 0.2, 1, action_dec, c + 1);

	mark_point();

	mg_sleep(10000); /* Sleep 10 second - timers will run */

	mark_point();
	ctx.stop_flag = 99; /* End timer thread */
	mark_point();

	mg_sleep(2000); /* Sleep 2 second - timers will not run */

	mark_point();

	timers_exit(&ctx);

	mark_point();

	/* If this test runs in a virtual environment, like the CI unit test
	 * containers, there might be some timing deviations, so check the
	 * counter with some tolerance. */

	ck_assert_int_ge(c[0], -1);
	ck_assert_int_le(c[0], +1);
	ck_assert_int_ge(c[1], -1);
	ck_assert_int_le(c[1], +1);
	ck_assert_int_ge(c[2], -1);
	ck_assert_int_le(c[2], +1);
}
END_TEST


START_TEST(test_timer_oneshot_by_callback_retval)
{
	struct mg_context ctx;
	int c[10];
	memset(&ctx, 0, sizeof(ctx));
	memset(c, 0, sizeof(c));

	action_dec_ret = 0;

	mark_point();
	timers_init(&ctx);
	mg_sleep(100);
	mark_point();

	c[0] = 10;
	timer_add(&ctx, 0, 0.1, 1, action_dec, c + 0);
	c[2] = 2;
	timer_add(&ctx, 0, 0.5, 1, action_dec, c + 2);
	c[1] = 5;
	timer_add(&ctx, 0, 0.2, 1, action_dec, c + 1);

	mark_point();

	mg_sleep(1000); /* Sleep 1 second - timer will run */

	mark_point();
	ctx.stop_flag = 99; /* End timer thread */
	mark_point();

	mg_sleep(1000); /* Sleep 1 second - timer will not run */

	mark_point();

	timers_exit(&ctx);

	mark_point();
	mg_sleep(100);

	ck_assert_int_eq(c[0], 9);
	ck_assert_int_eq(c[1], 4);
	ck_assert_int_eq(c[2], 1);
}
END_TEST


START_TEST(test_timer_oneshot_by_timer_add)
{
	struct mg_context ctx;
	int c[10];
	memset(&ctx, 0, sizeof(ctx));
	memset(c, 0, sizeof(c));

	action_dec_ret = 1;

	mark_point();
	timers_init(&ctx);
	mg_sleep(100);
	mark_point();

	c[0] = 10;
	timer_add(&ctx, 0, 0, 1, action_dec, c + 0);
	c[2] = 2;
	timer_add(&ctx, 0, 0, 1, action_dec, c + 2);
	c[1] = 5;
	timer_add(&ctx, 0, 0, 1, action_dec, c + 1);

	mark_point();

	mg_sleep(1000); /* Sleep 1 second - timer will run */

	mark_point();
	ctx.stop_flag = 99; /* End timer thread */
	mark_point();

	mg_sleep(1000); /* Sleep 1 second - timer will not run */

	mark_point();

	timers_exit(&ctx);

	mark_point();
	mg_sleep(100);

	ck_assert_int_eq(c[0], 9);
	ck_assert_int_eq(c[1], 4);
	ck_assert_int_eq(c[2], 1);
}
END_TEST


START_TEST(test_timer_mixed)
{
	struct mg_context ctx;
	int c[10];
	memset(&ctx, 0, sizeof(ctx));
	memset(c, 0, sizeof(c));

	mark_point();
	timers_init(&ctx);
	mg_sleep(100);
	mark_point();

	/* 3 --> 2, because it is a single shot timer */
	c[0] = 3;
	timer_add(&ctx, 0, 0, 1, action_dec_to_0, &c[0]);

	/* 3 --> 0, because it will run until c[1] = 0 and then stop */
	c[1] = 3;
	timer_add(&ctx, 0, 0.2, 1, action_dec_to_0, &c[1]);

	/* 3 --> 1, with 750 ms period, it will run once at start,
	 * then once 750 ms later, but not 1500 ms later, since the
	 * timer is already stopped then. */
	c[2] = 3;
	timer_add(&ctx, 0, 0.75, 1, action_dec_to_0, &c[2]);

	/* 3 --> 2, will run at start, but no cyclic in 1 second */
	c[3] = 3;
	timer_add(&ctx, 0, 2.5, 1, action_dec_to_0, &c[3]);

	/* 3 --> 3, will not run at start */
	c[4] = 3;
	timer_add(&ctx, 2.5, 0.1, 1, action_dec_to_0, &c[4]);

	/* 3 --> 2, an absolute timer in the past (-123.456) will still
	 * run once at start, and then with the period */
	c[5] = 3;
	timer_add(&ctx, -123.456, 2.5, 0, action_dec_to_0, &c[5]);

	/* 3 --> 1, an absolute timer in the past (-123.456) will still
	 * run once at start, and then with the period */
	c[6] = 3;
	timer_add(&ctx, -123.456, 0.75, 0, action_dec_to_0, &c[6]);

	mark_point();

	mg_sleep(1000); /* Sleep 1 second - timer will run */

	mark_point();
	ctx.stop_flag = 99; /* End timer thread */
	mark_point();

	mg_sleep(1000); /* Sleep 1 second - timer will not run */

	mark_point();

	timers_exit(&ctx);

	mark_point();
	mg_sleep(100);

	ck_assert_int_eq(c[0], 2);
	ck_assert_int_eq(c[1], 0);
	ck_assert_int_eq(c[2], 1);
	ck_assert_int_eq(c[3], 2);
	ck_assert_int_eq(c[4], 3);
	ck_assert_int_eq(c[5], 2);
	ck_assert_int_eq(c[6], 1);
}
END_TEST


#if !defined(REPLACE_CHECK_FOR_LOCAL_DEBUGGING)
Suite *
make_timertest_suite(void)
{
	Suite *const suite = suite_create("Timer");

	TCase *const tcase_timer_cyclic = tcase_create("Timer Periodic");
	TCase *const tcase_timer_oneshot = tcase_create("Timer Single Shot");
	TCase *const tcase_timer_mixed = tcase_create("Timer Mixed");

	tcase_add_test(tcase_timer_cyclic, test_timer_cyclic);
	tcase_set_timeout(tcase_timer_cyclic, 30);
	suite_add_tcase(suite, tcase_timer_cyclic);

	tcase_add_test(tcase_timer_oneshot, test_timer_oneshot_by_timer_add);
	tcase_add_test(tcase_timer_oneshot, test_timer_oneshot_by_callback_retval);
	tcase_set_timeout(tcase_timer_oneshot, 30);
	suite_add_tcase(suite, tcase_timer_oneshot);

	tcase_add_test(tcase_timer_mixed, test_timer_mixed);
	tcase_set_timeout(tcase_timer_mixed, 30);
	suite_add_tcase(suite, tcase_timer_mixed);

	return suite;
}
#endif


#ifdef REPLACE_CHECK_FOR_LOCAL_DEBUGGING
/* Used to debug test cases without using the check framework */

void
TIMER_PRIVATE(void)
{
	unsigned f_avail;
	unsigned f_ret;

#if defined(_WIN32)
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);
#endif

	f_avail = mg_check_feature(0xFF);
	f_ret = mg_init_library(f_avail);
	ck_assert_uint_eq(f_ret, f_avail);

	test_timer_cyclic(0);
	test_timer_oneshot_by_timer_add(0);
	test_timer_oneshot_by_callback_retval(0);
	test_timer_mixed(0);

	mg_exit_library();

#if defined(_WIN32)
	WSACleanup();
#endif
}

#endif
