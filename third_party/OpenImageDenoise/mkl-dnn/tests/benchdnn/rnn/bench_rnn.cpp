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

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mkldnn.h"

#include "mkldnn_common.hpp"
#include "mkldnn_debug.hpp"
#include "mkldnn_memory.hpp"

#include "rnn/rnn.hpp"

namespace rnn {

/* global driver parameters */
mkldnn_prop_kind_t prop = mkldnn_forward;
alg_t alg = VANILLA_RNN;
mkldnn_rnn_direction_t direction = mkldnn_unidirectional_left2right;
activation_t activation = RELU;
const char *perf_template = "perf,%n,%d,,,%-t,,%0t,";
const dt_conf_t *cfg = conf_f32;
policy_t scale_policy = NONE;
attr_t attr;
bool allow_unimpl = false;
int mb = 0;

void reset_parameters() {
    cfg = conf_f32;
    attr = attr_t();
    prop = mkldnn_forward;
    alg = VANILLA_RNN;
    direction = mkldnn_unidirectional_left2right;
    activation = RELU;
    scale_policy = NONE;
    allow_unimpl = false;
    mb = 0;
}

int bench(int argc, char **argv, bool main_bench) {
    for (int arg = 0; arg < argc; ++arg) {
        if (!strncmp("--batch=", argv[arg], 8))
            SAFE(batch(argv[arg] + 8, bench), CRIT);
        else if (!strncmp("--prop=", argv[arg], 7)) {
            dir_t dir = str2dir(argv[arg] + 7);
            if (dir == FWD_D)
                prop = mkldnn_forward;
            else if (dir == BWD_DW)
                prop = mkldnn_backward;
            else
                assert("unknown dir");
        } else if (!strncmp("--alg=", argv[arg], 6))
            alg = str2alg(argv[arg] + 6);
        else if (!strncmp("--cfg=", argv[arg], 6))
            cfg = str2cfg(argv[arg] + 6);
        else if (!strncmp("--attr=", argv[arg], 7))
            SAFE(str2attr(&attr, argv[arg] + 7), CRIT);
        else if (!strncmp("--direction=", argv[arg], 12))
            direction = str2direction(argv[arg] + 12);
        else if (!strncmp("--activation=", argv[arg], 13))
            activation = str2activation(argv[arg] + 13);
        else if (!strncmp("--allow-unimpl=", argv[arg], 15))
            allow_unimpl = str2bool(argv[arg] + 15);
        else if (!strncmp("--scaling=", argv[arg], 10))
            scale_policy = str2policy(argv[arg] + 10);
        else if (!strncmp("--reset", argv[arg], 7))
            reset_parameters();
        else if (!strncmp("--perf-template=", argv[arg], 16))
            perf_template = argv[arg] + 16;
        else if (!strncmp("--mb=", argv[arg], 5))
            mb = atoi(argv[arg] + 5);
        else if (!strncmp("-v", argv[arg], 2))
            verbose = atoi(argv[arg] + 2);
        else if (!strncmp("--verbose=", argv[arg], 10))
            verbose = atoi(argv[arg] + 10);
        else {
            rnn_desc_t d;
            if (str2desc(&d, argv[arg]) == FAIL) {
                fprintf(stderr, "driver: unknown option: `%s`, exiting...\n",
                        argv[arg]);
                exit(2);
            }
            if (cfg != conf_f32 && alg != VANILLA_LSTM) {
                fprintf(stderr,
                        "driver: configuration ``%s` is supported for LSTM "
                        "cell only, exiting...\n",
                        cfg2str(cfg));
                exit(2);
            }
            if (cfg != conf_f32 && scale_policy == NONE) {
                fprintf(stderr,
                        "driver: configuration ``%s` requires scale policy to "
                        "be COMMON or PER_OC, exiting...\n",
                        cfg2str(cfg));
                exit(2);
            }
            check(&d);
        }
    }
    return OK;
}

void check(rnn_desc_t *d) {
    const rnn_prb_t p(*d, cfg, prop, alg, direction, activation, attr,
        scale_policy, mb);
    res_t res{};
    char pstr[max_prb_len];

    int status = rnn::doit(&p, &res);

    prb2str(&p, &res, pstr);
    bool want_perf_report = false;

    parse_result(res, want_perf_report, allow_unimpl, status, pstr);

    if (bench_mode & PERF)
        perf_report(&p, &res, pstr);

    benchdnn_stat.tests++;
}

} // namespace rnn
