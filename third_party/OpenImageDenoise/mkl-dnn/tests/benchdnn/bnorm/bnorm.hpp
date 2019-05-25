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

#ifndef _BNORM_HPP
#define _BNORM_HPP

#include <stdint.h>
#include <limits.h>
#include <assert.h>

#include "common.hpp"
#include "dnn_types.hpp"
#include "mkldnn_common.hpp"
#include "mkldnn_memory.hpp"
#include "mkldnn_debug.hpp"

namespace bnorm {

enum check_alg_t { ALG_0, ALG_1, ALG_AUTO };
check_alg_t str2check_alg(const char *str);
const char* check_alg2str(check_alg_t alg);

using flags_t = unsigned;
const flags_t GLOB_STATS = mkldnn_use_global_stats;
const flags_t USE_SCALESHIFT = mkldnn_use_scaleshift;
const flags_t FUSE_BN_RELU = mkldnn_fuse_bn_relu;
flags_t str2flags(const char *str);
const char *flags2str(flags_t flags);

struct desc_t {
    int64_t mb, ic, id, ih, iw;
    float eps;
    const char *name;
};
const size_t max_desc_len = 196;
int str2desc(desc_t *desc, const char *str);
void desc2str(const desc_t *d, char *buffer, bool canonical = false);

struct prb_t: public desc_t {
    prb_t(const desc_t &desc, int64_t mb, dir_t dir, mkldnn_data_type_t dt,
            mkldnn_format_tag_t tag, flags_t flags, const attr_t &attr,
            check_alg_t check_alg)
        : desc_t(desc), check_alg(check_alg), dir(dir), dt(dt), tag(tag)
        , flags(flags), attr(attr)
    { if (mb) this->mb = mb; }
    ~prb_t() {}

    check_alg_t check_alg;

    dir_t dir;
    mkldnn_data_type_t dt;
    mkldnn_format_tag_t tag;
    flags_t flags;
    attr_t attr;
};
const size_t max_prb_len = max_attr_len + max_desc_len + 196;
void prb2str(const prb_t *p, char *buffer, bool canonical = false);

/* some extra control parameters which shouldn't be placed in prb_t */
extern const char *skip_impl; /* NULL or "" means do not skip anything */

extern const char *perf_template; /* performance output template */
void perf_report(const prb_t *p, const res_t *r, const char *pstr);

inline size_t data_off(const prb_t *p,
        int64_t mb, int64_t c, int64_t d, int64_t h, int64_t w) {
    return (((mb * p->ic + c) * p->id + d) * p->ih + h) * p->iw + w;
}

inline void inv_data_off(const prb_t *p, size_t off,
        int64_t &mb, int64_t &c, int64_t &d, int64_t &h, int64_t &w) {
    w = off % p->iw; off /= p->iw;
    h = off % p->ih; off /= p->ih;
    d = off % p->id; off /= p->id;
    c = off % p->ic; off /= p->ic;
    mb = off % p->mb; off /= p->mb;
    assert(off == 0);
}

inline bool is_bnorm_3d(const prb_t *p)
{
    return (p->id > 1) ? 1 : 0;
}

void compute_ref_fwd(const prb_t *p, const dnn_mem_t &src, dnn_mem_t &mean,
        dnn_mem_t &var, const dnn_mem_t &ss, dnn_mem_t &dst);
void compute_ref_bwd(const prb_t *p, const dnn_mem_t &src,
        const dnn_mem_t &mean, const dnn_mem_t &var, const dnn_mem_t &d_dst,
        const dnn_mem_t &ss, const dnn_mem_t &rmask, dnn_mem_t &d_src,
        dnn_mem_t &d_ss);

int doit(const prb_t *p, res_t *res);
int bench(int argc, char **argv, bool main_bench = true);

}

#endif
