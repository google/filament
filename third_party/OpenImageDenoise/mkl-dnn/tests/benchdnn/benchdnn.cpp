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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

#include "mkldnn.h"

#include "common.hpp"
#include "mkldnn_common.hpp"
#include "mkldnn_memory.hpp"

#include "self/self.hpp"
#include "conv/conv.hpp"
#include "conv/deconv.hpp"
#include "ip/ip.hpp"
#include "shuffle/shuffle.hpp"
#include "reorder/reorder.hpp"
#include "bnorm/bnorm.hpp"
#include "rnn/rnn.hpp"

int verbose {0};
bench_mode_t bench_mode {CORR};
stat_t benchdnn_stat {0};

double max_ms_per_prb {3e3};
int min_times_per_prb {5};
int fix_times_per_prb {0};

int main(int argc, char **argv) {
    prim_t prim = DEF;
    --argc; ++argv;

    while (argc > 0) {
        if (!strcmp("--self", argv[0])) prim = SELF;
        else if (!strcmp("--conv", argv[0])) prim = CONV;
        else if (!strcmp("--deconv", argv[0])) prim = DECONV;
        else if (!strcmp("--ip", argv[0])) prim = IP;
        else if (!strcmp("--shuffle", argv[0])) prim = SHUFFLE;
        else if (!strcmp("--reorder", argv[0])) prim = REORDER;
        else if (!strcmp("--bnorm", argv[0])) prim = BNORM;
        else if (!strcmp("--rnn", argv[0])) prim = RNN;
        else if (!strncmp("--mode=", argv[0], 7))
            bench_mode = str2bench_mode(argv[0] + 7);
        else if (!strncmp("--max-ms-per-prb=", argv[0], 17))
            sscanf(argv[0] + 17, "%lf", &max_ms_per_prb);
        else if (!strncmp("-v", argv[0], 2))
            verbose = atoi(argv[0] + 2);
        else if (!strncmp("--verbose=", argv[0], 10))
            verbose = atoi(argv[0] + 10);
        else break;

        --argc;
        ++argv;
    }

    if (max_ms_per_prb < 100 || max_ms_per_prb > 60e3)
        max_ms_per_prb = 3e3;

    init();

    switch (prim) {
    case SELF: self::bench(argc, argv); break;
    case CONV: conv::bench(argc, argv); break;
    case DECONV: deconv::bench(argc, argv); break;
    case IP: ip::bench(argc, argv); break;
    case SHUFFLE: shuffle::bench(argc, argv); break;
    case REORDER: reorder::bench(argc, argv); break;
    case BNORM: bnorm::bench(argc, argv); break;
    case RNN: rnn::bench(argc, argv); break;
    default: fprintf(stderr, "err: unknown driver\n");
    }

    finalize();

    printf("tests:%d passed:%d "
            "skipped:%d mistrusted:%d unimplemented:%d "
            "failed:%d\n",
            benchdnn_stat.tests, benchdnn_stat.passed,
            benchdnn_stat.skipped, benchdnn_stat.mistrusted,
            benchdnn_stat.unimplemented, benchdnn_stat.failed);
    if (bench_mode & PERF) {
        printf("total perf: min(ms):%g avg(ms):%g\n",
                benchdnn_stat.ms[benchdnn_timer_t::min],
                benchdnn_stat.ms[benchdnn_timer_t::avg]);
    }

    return !!benchdnn_stat.failed;
}
