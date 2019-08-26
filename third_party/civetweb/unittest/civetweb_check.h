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
#ifndef TEST_CIVETWEB_CHECK_H_
#define TEST_CIVETWEB_CHECK_H_

#ifdef __clang__
#pragma clang diagnostic push
// FIXME: check uses GCC specific variadic macros that are non-standard
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

#if defined(__MINGW__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
/* Disable warnings for defining _CRT_SECURE_NO_* (here) and
 * _CHECK_CHECK_STDINT_H (in check.h)
 */
/* Disable until Warning returns to Travis CI or AppVeyor
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wno-variadic-macros"
#pragma GCC diagnostic ignored "-Wreserved-id-macro"
*/
#endif

#ifdef _MSC_VER
#undef pid_t
#define pid_t int
/* Unreferenced formal parameter. START_TEST has _i */
#pragma warning(disable : 4100)
/* conditional expression is constant . asserts use while(0) */
#pragma warning(disable : 4127)
#endif
#include <stdint.h>

/* All unit tests use the "check" framework.
 * Download from https://libcheck.github.io/check/
 */
#include "check.h"

#if (CHECK_MINOR_VERSION < 11)
#ifndef LOCAL_TEST
#error "CivetWeb unit tests require at least check 0.11.0"
#endif
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE
#endif

#if defined(__MINGW__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#ifdef __clang__
/* When using -Weverything, clang does not accept it's own headers
 * in a release build configuration. Disable what is too much in
 * -Weverything. */
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif

/* A minimal timeout used for all tests with the "check" framework. */
#define civetweb_min_test_timeout (30)

/* A minimal timeout for all tests starting a server. */
#define civetweb_min_server_test_timeout (civetweb_min_test_timeout + 30)

/* A default timeout for all tests running multiple requests to a server. */
#define civetweb_mid_server_test_timeout                                       \
	(civetweb_min_server_test_timeout + 180)

#endif /* TEST_CIVETWEB_CHECK_H_ */
