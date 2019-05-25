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

#ifndef _IP_HPP
#define _IP_HPP

#include "mkldnn.h"

#include "common.hpp"
#include "mkldnn_common.hpp"
#include "mkldnn_memory.hpp"

namespace ip {
const size_t max_prb_len = 392;
const size_t max_desc_len = 196;

struct desc_t {
    int64_t mb, oc, ic, id, ih, iw;
    const char *name;
};

typedef struct dt_conf_t {
    mkldnn_data_type_t dt;
    double min, max; /* representative */
    int f_min, f_max; /* fill range */
    int f_base; /* fill base, use 0 */
    int f_step; /* fill step, use 1 */
    double f_sparsity; /* amount of non-zeros, default 0.25 */
    double eps; /* acceptable error */
} _dt_conf_t[DAT_TOTAL];

extern const _dt_conf_t conf_f32;
extern const _dt_conf_t conf_u8s8s32s32;
extern const _dt_conf_t conf_u8s8s8s32;
extern const _dt_conf_t conf_u8s8u8s32;

struct prb_t : public desc_t {
    prb_t(const desc_t &desc, int64_t mb, dir_t dir, const dt_conf_t *cfg,
            const attr_t &attr)
        : desc_t(desc), dir(dir), cfg(cfg), attr(attr), scales(NULL) {
        if (mb)
            this->mb = mb;
        generate_oscales();
    }
    ~prb_t() { if (scales) zfree(scales); }
    dir_t dir;
    const dt_conf_t *cfg;
    attr_t attr;
    float *scales;

    void generate_oscales();
};

int str2desc(desc_t *desc, const char *str);
void prb2str(const prb_t *p, char *buffer, bool canonical = false);
void perf_report(const prb_t *p, const res_t *r, const char *pstr);
const dt_conf_t *str2cfg(const char *str);
const char *cfg2str(const dt_conf_t *cfg);

extern const char *perf_template; /* performance output template */

inline size_t src_off_f(const prb_t *p,
        int64_t mb, int64_t ic, int64_t id, int64_t ih, int64_t iw) {
    return (((mb * p->ic + ic) * p->id + id) * p->ih + ih) * p->iw + iw;
}

inline size_t wei_off_f(const prb_t *p,
        int64_t oc, int64_t ic, int64_t id, int64_t ih, int64_t iw) {
    return (((oc * p->ic + ic) * p->id + id) * p->ih + ih) * p->iw + iw;
}

inline size_t bia_off_f(const prb_t *p, int64_t oc) { return oc; }

inline size_t dst_off_f(const prb_t *p, int64_t mb, int64_t oc) {
    return mb * p->oc + oc;
}

void compute_ref_fwd(const prb_t *p, dnn_mem_t &src_m, dnn_mem_t &wei_m,
        dnn_mem_t &bia_m, dnn_mem_t &dst_m);
void compute_ref_bwd_d(const prb_t *p, dnn_mem_t &diff_src_m, dnn_mem_t &wei_m,
        dnn_mem_t &diff_dst_m);
void compute_ref_bwd_w(const prb_t *p, dnn_mem_t &src_m, dnn_mem_t &diff_wei_m,
        dnn_mem_t &diff_bia_m, dnn_mem_t &diff_dst_m);

int doit(const prb_t *p, res_t *res);

int bench(int argc, char **argv, bool main_bench = true);
}

#endif
