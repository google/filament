/*******************************************************************************
* Copyright 2018 Intel Corporation
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

#include "mkldnn.h"

#include "mkldnn_common.hpp"
#include "mkldnn_memory.hpp"

#include "conv/deconv.hpp"
using namespace conv;

namespace deconv {

/* global driver parameters */
const dt_conf_t *cfg = conf_f32;
const char *pattern = NULL;
dir_t dir = FWD_B;
int64_t mb = 0;
alg_t alg = DIRECT;
attr_t attr;
const char *skip_impl = "";
bool allow_unimpl = false;
const char *perf_template = "perf,%n,%d,%GO,%GF,%-t,%-Gp,%0t,%0Gp";

void reset_parameters() {
    cfg = conf_f32;
    pattern = NULL;
    dir = FWD_B;
    mb = 0;
    alg = DIRECT;
    attr = attr_t();
    skip_impl = "";
    allow_unimpl = false;
}

void check_correctness(const desc_t *c) {
    const prb_t p(*c, dir, cfg, alg, attr, mb, true);
    char pstr[max_prb_len];
    prb2str(&p, pstr);

    if (pattern && !match_regex(pstr, pattern))
        return;
    print(1, "run: %s\n", pstr);

    res_t res{};
    const int status = doit(&p, &res);
    (void)status;

    bool want_perf_report = false;

    parse_result(res, want_perf_report, allow_unimpl, status, pstr);

    if (want_perf_report && bench_mode & PERF)
        perf_report(&p, &res, pstr);

    benchdnn_stat.tests++;
}

int bench(int argc, char **argv, bool main_bench) {
    for (int arg = 0; arg < argc; ++arg) {
        if (!strncmp("--batch=", argv[arg], 8))
            SAFE(batch(argv[arg] + 8, bench), CRIT);
        else if (!strncmp("--cfg=", argv[arg], 6))
            cfg = str2cfg(argv[arg] + 6);
        else if (!strncmp("--match=", argv[arg], 8))
            pattern = argv[arg] + 8;
        else if (!strncmp("--mb=", argv[arg], 5))
            mb = atoi(argv[arg] + 5);
        else if (!strncmp("--dir=", argv[arg], 6))
            dir = str2dir(argv[arg] + 6);
        else if (!strncmp("--alg=", argv[arg], 6))
            alg = str2alg(argv[arg] + 6);
        else if (!strncmp("--attr=", argv[arg], 7))
            SAFE(str2attr(&attr, argv[arg] + 7), CRIT);
        else if (!strncmp("--skip-impl=", argv[arg], 12))
            skip_impl = argv[arg] + 12;
        else if (!strncmp("--allow-unimpl=", argv[arg], 15))
            allow_unimpl = str2bool(argv[arg] + 15);
        else if (!strncmp("--perf-template=", argv[arg], 16))
            perf_template = argv[arg] + 16;
        else if (!strcmp("--reset", argv[arg]))
            reset_parameters();
        else if (!strncmp("--mode=", argv[arg], 7))
            bench_mode = str2bench_mode(argv[arg] + 7);
        else if (!strncmp("-v", argv[arg], 2))
            verbose = atoi(argv[arg] + 2);
        else if (!strncmp("--verbose=", argv[arg], 10))
            verbose = atoi(argv[arg] + 10);
        else {
            desc_t c;
            bool is_deconv = 1;
            if (str2desc(&c, argv[arg], is_deconv) == FAIL) {
                fprintf(stderr, "driver: unknown option: `%s`, exiting...\n",
                        argv[arg]);
                exit(2);
            }
            check_correctness(&c);
        }
    }
    return OK;
}
}
