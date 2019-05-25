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

#ifndef _SHUFFLE_HPP
#define _SHUFFLE_HPP

#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include <vector>

#include "common.hpp"
#include "dnn_types.hpp"
#include "mkldnn_common.hpp"
#include "mkldnn_memory.hpp"
#include "mkldnn_debug.hpp"

namespace shuffle {

using dims_t = std::vector<int64_t>;

struct dt_conf_t {
    mkldnn_data_type_t dt;
    int min;
    int range;
};

const int int_max_exact = 1<<24;

const dt_conf_t conf_f32 = {mkldnn_f32, -int_max_exact, 2*int_max_exact};
const dt_conf_t conf_s8 = {mkldnn_s8, INT8_MIN, -2*INT8_MIN};
const dt_conf_t conf_u8 = {mkldnn_u8, 0, UINT8_MAX};
const dt_conf_t conf_s32 = {mkldnn_s32, -int_max_exact, 2*int_max_exact};

const size_t max_desc_len = 196;

struct prb_t {
    prb_t(dims_t &dims, dir_t dir, mkldnn_data_type_t dt,
            mkldnn_format_tag_t tag, int axis, int64_t group)
        : dims(dims), dir(dir), dt(dt), tag(tag), a(axis), g(group) {}
    ~prb_t() {}

    dims_t dims;
    dir_t dir;
    mkldnn_data_type_t dt;
    mkldnn_format_tag_t tag;
    int a;
    int64_t g;
};

const size_t max_dims_len = 20;
dims_t str2dims(const char *str);
void dims2str(const dims_t &dims, char *buffer);
const size_t max_prb_len = max_desc_len + 196;
void prb2str(const prb_t *p, char *buffer, bool canonical = false);

extern const char *perf_template; /* performance output template */
void perf_report(const prb_t *p, const res_t *r, const char *pstr);

inline size_t data_off(const prb_t *p,
        int64_t mb, int64_t c, int64_t d, int64_t h, int64_t w) {
    const auto &dims = p->dims;
    return (((mb * dims[1] + c) * dims[2] + d) * dims[3] + h) * dims[4] + w;
}

void compute_shuffle(const prb_t *p, const dnn_mem_t &src, dnn_mem_t &dst);

int fill_memory(const prb_t *p, dnn_mem_t &src);
int doit(const prb_t *p, res_t *res);
int bench(int argc, char **argv, bool main_bench = true);
}

#endif
