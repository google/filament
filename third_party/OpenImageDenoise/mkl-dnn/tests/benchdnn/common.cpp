/*******************************************************************************
* Copyright 2017-2018 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include <stdint.h>
#include <limits.h>
#include <assert.h>

#include "mkldnn.h"

#include "common.hpp"
const char *bench_mode2str(bench_mode_t mode) {
    const char *modes[] = {
        "MODE_UNDEF", "CORR", "PERF", "CORR+PERF"
    };
    assert((int)mode < 4);
    return modes[(int)mode];
}

bench_mode_t str2bench_mode(const char *str) {
    bench_mode_t mode = MODE_UNDEF;
    if (strchr(str, 'c') || strchr(str, 'C'))
        mode = (bench_mode_t)((int)mode | (int)CORR);
    if (strchr(str, 'p') || strchr(str, 'P'))
        mode = (bench_mode_t)((int)mode | (int)PERF);
    if (mode == MODE_UNDEF)
        []() { SAFE(FAIL, CRIT); return 0; }();
    return mode;
}

/* perf */
#include <chrono>

static inline double ms_now() {
    auto timePointTmp
        = std::chrono::high_resolution_clock::now().time_since_epoch();
    return std::chrono::duration<double, std::milli>(timePointTmp).count();
}

#if !defined(BENCHDNN_USE_RDPMC) || defined(_WIN32)
unsigned long long ticks_now() {
    return (unsigned long long)0;
}
#else
unsigned long long ticks_now() {
    unsigned eax, edx, ecx;

    ecx = (1 << 30) + 1;
    __asm__ volatile("rdpmc" : "=a" (eax), "=d" (edx) : "c" (ecx));

    return (unsigned long long)eax | (unsigned long long)edx << 32;
}
#endif

void benchdnn_timer_t::reset() {
    times_ = 0;
    for (int i = 0; i < n_modes; ++i) ticks_[i] = 0;
    ticks_start_ = 0;
    for (int i = 0; i < n_modes; ++i) ms_[i] = 0;
    ms_start_ = 0;

    start();
}

void benchdnn_timer_t::start() {
    ticks_start_ = ticks_now();
    ms_start_ = ms_now();
}

void benchdnn_timer_t::stop() {
    long long d_ticks = ticks_now() - ticks_start_; /* FIXME: overflow? */
    double d_ms = ms_now() - ms_start_;

    ticks_start_ += d_ticks;
    ms_start_ += d_ms;

    ms_[benchdnn_timer_t::min] = times_
        ? MIN2(ms_[benchdnn_timer_t::min], d_ms) : d_ms;
    ms_[benchdnn_timer_t::avg] += d_ms;
    ms_[benchdnn_timer_t::max] = times_
        ? MAX2(ms_[benchdnn_timer_t::max], d_ms) : d_ms;

    ticks_[benchdnn_timer_t::min] = times_
        ? MIN2(ticks_[benchdnn_timer_t::min], d_ticks) : d_ticks;
    ticks_[benchdnn_timer_t::avg] += d_ticks;
    ticks_[benchdnn_timer_t::max] = times_
        ? MAX2(ticks_[benchdnn_timer_t::max], d_ticks) : d_ticks;

    times_++;
}

benchdnn_timer_t &benchdnn_timer_t::operator=(const benchdnn_timer_t &rhs) {
    if (this == &rhs) return *this;
    times_ = rhs.times_;
    for (int i = 0; i < n_modes; ++i) ticks_[i] = rhs.ticks_[i];
    ticks_start_ = rhs.ticks_start_;
    for (int i = 0; i < n_modes; ++i) ms_[i] = rhs.ms_[i];
    ms_start_ = rhs.ms_start_;
    return *this;
}

/* result structure */
const char *state2str(res_state_t state) {
#define CASE(x) if (state == x) return STRINGIFY(x)
    CASE(UNTESTED);
    CASE(PASSED);
    CASE(SKIPPED);
    CASE(MISTRUSTED);
    CASE(UNIMPLEMENTED);
    CASE(FAILED);
#undef CASE
    assert(!"unknown res state");
    return "STATE_UNDEF";
}

void parse_result(res_t &res, bool &want_perf_report, bool allow_unimpl,
        int status, char *pstr) {
    auto &bs = benchdnn_stat;
    const char *state = state2str(res.state);

    switch (res.state) {
    case UNTESTED:
        if (!(bench_mode & CORR)) {
            want_perf_report = true;
            break;
        }
    case FAILED:
        assert(status == FAIL);
        bs.failed++;
        print(0, "%d:%s (errors:%lu total:%lu) __REPRO: %s\n", bs.tests, state,
                (unsigned long)res.errors,
                (unsigned long)res.total, pstr);
        break;
    case SKIPPED:
        assert(status == OK);
        print(0, "%d:%s __REPRO: %s\n", bs.tests, state, pstr);
        bs.skipped++;
        break;
    case UNIMPLEMENTED:
        assert(status == OK);
        print(0, "%d:%s __REPRO: %s\n", bs.tests, state, pstr);
        bs.unimplemented++;
        bs.failed += !allow_unimpl;
        break;
    case MISTRUSTED:
        assert(status == OK);
        bs.mistrusted++;
        print(0, "%d:%s __REPRO: %s\n", bs.tests, state, pstr);
        // bs.failed++; /* temporal workaround for some tests */
        break;
    case PASSED:
        assert(status == OK);
        print(0, "%d:%s __REPRO: %s\n", bs.tests, state, pstr);
        want_perf_report = true;
        bs.passed++;
        break;
    default:
        assert(!"unknown state");
        { []() { SAFE(FAIL, CRIT); return 0; }(); }
    }

    if (bench_mode & PERF) {
        using bt = benchdnn_timer_t;
        for (int mode = 0; mode < (int)bt::n_modes; ++mode)
            bs.ms[mode] += res.timer.ms((bt::mode_t)mode);
    }
}

/* misc */
void *zmalloc(size_t size, size_t align) {
    void *ptr;
#ifdef _WIN32
    ptr = _aligned_malloc(size, align);
    int rc = ((ptr) ? 0 : errno);
#else
    // TODO. Heuristics: Increasing the size to alignment increases
    // the stability of performance results.
    if (size < align)
        size = align;
    int rc = ::posix_memalign(&ptr, align, size);
#endif /* _WIN32 */
    return rc == 0 ? ptr : 0;
}

void zfree(void *ptr) {
#ifdef _WIN32
    _aligned_free(ptr);
#else
    return ::free(ptr);
#endif /* _WIN32 */
}

bool str2bool(const char *str) {
    return !strcasecmp("true", str) || !strcasecmp("1", str);
}

const char *bool2str(bool value) {
    return value ? "true" : "false";
}

#ifdef _WIN32
/* NOTE: this should be supported on linux as well, but currently
 * having issues for ICC170 and Clang*/
#include <regex>

bool match_regex(const char *str, const char *pattern) {
    std::regex re(pattern);
    return std::regex_search(str, re);
}
#else
#include <sys/types.h>
#include <regex.h>

bool match_regex(const char *str, const char *pattern) {
    static regex_t regex;
    static const char *prev_pattern = NULL;
    if (pattern != prev_pattern) {
        if (prev_pattern)
            regfree(&regex);

        if (regcomp(&regex, pattern, 0)) {
            fprintf(stderr, "could not create regex\n");
            return true;
        }

        prev_pattern = pattern;
    }

    return !regexec(&regex, str, 0, NULL, 0);
}
#endif /* _WIN32 */

bool maybe_skip(const char *skip_impl, const char *impl_str) {
    if (skip_impl == NULL || *skip_impl == '\0')
        return false;

    const size_t max_len = 128;
    char what[max_len] = {0};

    const char *s_start = skip_impl;
    while (1) {
        if (*s_start == '"' || *s_start == '\'')
            ++s_start;

        const char *s_end = strchr(s_start, ':');
        size_t len = s_end ? s_end - s_start : strlen(s_start);

        if (s_start[len - 1] == '"' || s_start[len - 1] == '\'')
            --len;

        SAFE(len < max_len ? OK : FAIL, CRIT);
        len = MIN2(len, max_len - 1);
        strncpy(what, s_start, len);
        what[len] = '\0';

        if (strstr(impl_str, what))
            return true;

        if (s_end == NULL)
            break;

        s_start = s_end + 1;
        if (*s_start == '\0')
            break;
    }

    return false;
}

#if defined(_WIN32) && !defined(__GNUC__)
#include <windows.h>
#define PATH_MAX MAX_PATH
static char *dirname(char *path) {
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    _splitpath(path, drive, dir, NULL, NULL);
    path[0] = '\0';
    if (drive != NULL) strncat(path, drive, _MAX_DRIVE);
    if (dir != NULL) strncat(path, dir, MAX_PATH);
    if (path[0] == '\0') strcat(path, ".");
    return path;
}
#else
#include <libgen.h>
#endif /* _WIN32 */

FILE *open_batch_file(const char *fname) {
    const int max_paths = 4;

    static int n_paths = 0;
    static char search_paths[max_paths][PATH_MAX] = {{0}};

    char *fdir = NULL;
    char fname_copy[PATH_MAX];
    {
        strncpy(fname_copy, fname, PATH_MAX - 1);
        fname_copy[PATH_MAX - 1] = '\0';
        fdir = dirname(fname_copy);
    }

    bool dir_found = false;
    for (int n = 0; n_paths < max_paths && n < n_paths; ++n)
        if (!strcmp(fdir, search_paths[n])) {
            dir_found = true;
            break;
        }
    if (!dir_found) {
        SAFE_V(n_paths < max_paths ? OK : FAIL);
        strcpy(search_paths[n_paths++], fdir);
    }

    FILE *fp = fopen(fname, "r");
    if (fp) return fp;

    for (int n = 0; n < n_paths; ++n) {
        char fullname[PATH_MAX + 2];
        snprintf(fullname, PATH_MAX + 2, "%s/%s", search_paths[n], fname);
        fp = fopen(fullname, "r");
        print(50, "batch file used: %s\n", fullname);
        if (fp) break;
    }

    return fp;
}

int batch(const char *fname, bench_f bench) {
    FILE *fp = open_batch_file(fname);
    SAFE(fp ? OK : FAIL, CRIT);

    const size_t maxlen = 1024;
    char *opts[8*1024] = {0}, buf[maxlen + 1];
    char line[1024];
    int n_opts = 0;
    while (fgets(line, sizeof(line), fp)) {
        int offset = 0;
        const char *l = line;
        while (sscanf(l, "%s%n", buf, &offset) == 1) {
            if (buf[0] == '#')
                break; /* stop reading till eol */

            const size_t len = strnlen(buf, maxlen) + 1;
            opts[n_opts] = (char *)malloc(len);
            SAFE(opts[n_opts] ? OK : FAIL, CRIT);
            strncpy(opts[n_opts], buf, len);
            ++n_opts;

            l += offset;
        }
    }
    bench(n_opts, opts, false);

    for (int n = 0; n < n_opts; ++n)
        free(opts[n]);

    fclose(fp);

    return OK;
}

int flip_coin(ptrdiff_t seed, float probability) {
    const ptrdiff_t big_prime = 1000003;
    const ptrdiff_t prime = 753737;
    seed *= prime;
    return (seed % big_prime) < (probability * big_prime);
}

int div_up(const int a, const int b){
    SAFE_V(b != 0 ? OK : FAIL);
    return (a + b - 1) / b;
}

#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
int mxcsr_round(float f) { return _mm_cvtss_si32(_mm_load_ss(&f)); }
#else
int mxcsr_round(float f) { return (int)nearbyintf(f); }
#endif

void array_set(char *arr, size_t size) {
    for (size_t i = 0; i < size; ++i)
        arr[i] = 0;
}

void gemm(const char *layout, const char *transa, const char *transb,
        int64_t m, int64_t n, int64_t k,
        const float alpha, const float *a, const int64_t lda,
        const float *b, const int64_t ldb,
        const float beta, float *c, const int64_t ldc) {
    if (*layout == 'F') {
        mkldnn_sgemm(transa, transb, &m, &n, &k, &alpha, a, &lda, b, &ldb,
                &beta, c, &ldc);
    } else {
        mkldnn_sgemm(transb, transa, &n, &m, &k, &alpha, b, &ldb, a, &lda,
                &beta, c, &ldc);
    }
}
