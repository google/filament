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
#include "mkldnn_debug.hpp"

#include "shuffle/shuffle.hpp"

namespace shuffle {

/* global driver parameters */
int64_t mb = 0;
dir_t dir = FWD_D;
mkldnn_data_type_t dt = mkldnn_f32;
mkldnn_format_tag_t tag = mkldnn_nchw;
dims_t dims;
int axis = 1;
int64_t group = 1;
const char *pattern = NULL;
bool allow_unimpl = false;
const char *perf_template = "perf,%z,%q,%f,%D,%a,%g,%-t,%0t";

void reset_parameters() {
    dir = FWD_D;
    dt = mkldnn_f32;
    tag = mkldnn_nchw;
    axis = 1;
    group = 1;
    pattern = NULL;
}

void check_correctness() {
    const prb_t p(dims, dir, dt, tag, axis, group);
    char pstr[max_prb_len];
    prb2str(&p, pstr);

    if (pattern && !match_regex(pstr, pattern))
        return;
    print(1, "run: %s\n", pstr);

    res_t res{};
    const int status = shuffle::doit(&p, &res);

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
        else if (!strncmp("--dir=", argv[arg], 6))
            dir = str2dir(argv[arg] + 6);
        else if (!strncmp("--dt=", argv[arg], 5))
            dt = str2dt(argv[arg] + 5);
        else if (!strncmp("--tag=", argv[arg], 6))
            tag = str2tag(argv[arg] + 6);
        else if (!strncmp("--axis=", argv[arg], 7))
            axis = atoi(argv[arg] + 7);
        else if (!strncmp("--group=", argv[arg], 8))
            group = atoi(argv[arg] + 8);
        else if (!strncmp("--match=", argv[arg], 8))
            pattern = argv[arg] + 8;
        else if (!strncmp("--mode=", argv[0], 7))
            bench_mode = str2bench_mode(argv[0] + 7);
        else if (!strncmp("-v", argv[arg], 2))
            verbose = atoi(argv[arg] + 2);
        else if (!strncmp("--verbose=", argv[arg], 10))
            verbose = atoi(argv[arg] + 10);
        else {
            if (!strncmp("--", argv[arg], 2)) {
                fprintf(stderr, "driver: unknown option: `%s`, exiting...\n",
                        argv[arg]);
                exit(2);
            }
            dims = str2dims(argv[arg]);
            check_correctness();
        }
    }

    return OK;
}

}
