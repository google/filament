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

#ifndef _CONV_COMMON_HPP
#define _CONV_COMMON_HPP

#include <stdint.h>
#include <limits.h>
#include <assert.h>

#include "common.hpp"
#include "dnn_types.hpp"
#include "mkldnn_common.hpp"
#include "mkldnn_memory.hpp"

namespace deconv {
/* some extra control parameters which shouldn't be placed in prb_t */
extern const char *skip_impl; /* NULL or "" means do not skip anything */
extern bool allow_unimpl; /* true means do not treat unimplemented as error */
extern const char *perf_template; /* performance output template */
}

namespace conv {

enum alg_t { DIRECT, WINO, AUTO };
alg_t str2alg(const char *str);
const char *alg2str(alg_t alg);
alg_t alg_kind2alg(mkldnn_alg_kind_t alg);

struct desc_t {
    int64_t g, mb;
    int64_t ic, id, ih, iw;
    int64_t oc, od, oh, ow;
    int64_t kd, kh, kw;
    int64_t sd, sh, sw;
    int64_t pd, ph, pw;
    int64_t dd, dh, dw;
    bool has_groups;

    const char *name;
};
const size_t max_desc_len = 196;
int str2desc(desc_t *desc, const char *str, bool is_deconv);
void desc2str(const desc_t *d, char *buffer, bool canonical = false);

/** configuration structure, that controls initial data filling + error check
 *
 * dt defines convolution precision
 *
 * for each type (SRC, WEI, BIA, and DST) the values are filled as follows:
 * if (rand() > f_sparsity) then:
 *     v <-- f_base // it is guaranteed each kernel window
 *                  // has at least one non-zero element
 * else:
 *     v <-- f_min + rand() * f_step % (f_max - f_min)
 *
 *
 * on final check the resulting values should be in [min .. max] range, the
 * relative difference should not exceed eps
 */
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
extern const _dt_conf_t conf_f32_full;
extern const _dt_conf_t conf_f32_wino;
extern const _dt_conf_t conf_u8s8s32s32;
extern const _dt_conf_t conf_u8s8s8s32;
extern const _dt_conf_t conf_u8s8u8s32;
extern const _dt_conf_t conf_s8s8s32s32;
extern const _dt_conf_t conf_s8s8s8s32;
extern const _dt_conf_t conf_s8s8u8s32;
extern const _dt_conf_t conf_u8s8f32s32_wino;
extern const _dt_conf_t conf_u8s8s32s32_wino;
extern const _dt_conf_t conf_u8s8s8s32_wino;
extern const _dt_conf_t conf_u8s8u8s32_wino;

const dt_conf_t *str2cfg(const char *str);
const char *cfg2str(const dt_conf_t *cfg);
const dt_conf_t *auto_cfg(const alg_t alg, const dt_conf_t *cfg);

struct prb_t: public desc_t {
    prb_t(const desc_t &desc, dir_t dir, const dt_conf_t *cfg, alg_t alg,
            const attr_t &attr, int64_t mb = 0, bool is_deconv = false)
        : desc_t(desc), dir(dir), cfg(cfg), alg(alg), attr(attr)
        , ops(0), scales(NULL), is_deconv(is_deconv) {
        if (mb) this->mb = mb;
        count_ops();
        generate_oscales();
    }
    ~prb_t() { if (scales) zfree(scales); }

    dir_t dir;
    const dt_conf_t *cfg;
    alg_t alg;
    attr_t attr;

    double ops;
    float *scales;
    bool is_deconv;

    void count_ops();
    void generate_oscales();

private:
    prb_t(const prb_t &) = delete;
    prb_t &operator=(const prb_t &) = delete;
};
const size_t max_prb_len = max_attr_len + max_desc_len + 196;
void prb2str(const prb_t *p, char *buffer, bool canonical = false);

/* some extra control parameters which shouldn't be placed in prb_t */
extern const char *skip_impl; /* NULL or "" means do not skip anything */
extern bool allow_unimpl; /* true means do not treat unimplemented as error */
extern const char *perf_template; /* performance output template */

inline int64_t src_off_f(const prb_t *p, int64_t mb, int64_t g, int64_t ic,
        int64_t id, int64_t ih, int64_t iw) {
    return (((mb * p->ic + g * p->ic/p->g + ic)
        * p->id + id) * p->ih + ih) * p->iw + iw;
}

inline void inv_src_off_f(const prb_t *p, int64_t off, int64_t &mb, int64_t &g,
        int64_t &ic, int64_t &id, int64_t &ih, int64_t &iw) {
    iw = off % p->iw; off /= p->iw;
    ih = off % p->ih; off /= p->ih;
    id = off % p->id; off /= p->id;
    ic = off % (p->ic / p->g); off /= (p->ic / p->g);
    g = off % p->g; off /= p->g;
    mb = off % p->mb; off /= p->mb;
    assert(off == 0);
}

inline int64_t wei_off_f(const prb_t *p, int64_t g, int64_t oc, int64_t ic,
        int64_t kd, int64_t kh, int64_t kw) {
    return ((((g * p->oc / p->g + oc) * p->ic / p->g + ic)
        * p->kd + kd) * p->kh + kh) * p->kw + kw;
}

inline void inv_wei_off_f(const prb_t *p, int64_t off, int64_t &g, int64_t &oc,
        int64_t &ic, int64_t &kd, int64_t &kh, int64_t &kw) {
    kw = off % p->kw; off /= p->kw;
    kh = off % p->kh; off /= p->kh;
    kd = off % p->kd; off /= p->kd;
    ic = off % (p->ic / p->g); off /= (p->ic / p->g);
    oc = off % (p->oc / p->g); off /= (p->oc / p->g);
    g = off % p->g; off /= p->g;
    assert(off == 0);
}

inline int64_t bia_off_f(const prb_t *p, int64_t g, int64_t oc) {
    return g * p->oc / p->g + oc;
}

inline void inv_bia_off_f(const prb_t *p, int64_t off, int64_t &g, int64_t &oc)
{
    oc = off % (p->oc / p->g); off /= (p->oc / p->g);
    g = off % p->g; off /= p->g;
    assert(off == 0);
}

inline int64_t dst_off_f(const prb_t *p, int64_t mb, int64_t g, int64_t oc,
        int64_t od, int64_t oh, int64_t ow) {
    return (((mb * p->oc + g * p->oc/p->g + oc) * p->od + od)
        * p->oh + oh) * p->ow + ow;
}

inline void inv_dst_off_f(const prb_t *p, int64_t off, int64_t &mb, int64_t &g,
        int64_t &oc, int64_t &od, int64_t &oh, int64_t &ow) {
    ow = off % p->ow; off /= p->ow;
    oh = off % p->oh; off /= p->oh;
    od = off % p->od; off /= p->od;
    oc = off % (p->oc / p->g); off /= (p->oc / p->g);
    g = off % p->g; off /= p->g;
    mb = off % p->mb; off /= p->mb;
    assert(off == 0);
}

float oscale(const prb_t *p, int oc);

void compute_ref_fwd(const prb_t *p, dnn_mem_t &src_m, dnn_mem_t &wei_m,
        dnn_mem_t &bia_m, dnn_mem_t &dst_m);
void compute_ref_bwd_d(const prb_t *p, dnn_mem_t &diff_src_m, dnn_mem_t &wei_m,
        dnn_mem_t &bia_m, dnn_mem_t &diff_dst_m);
void compute_ref_bwd_w(const prb_t *p, dnn_mem_t &src_m, dnn_mem_t &diff_wei_m,
        dnn_mem_t &diff_bia_m, dnn_mem_t &diff_dst_m);

void compute_ref_direct_fwd(const prb_t *p, dnn_mem_t &src_m, dnn_mem_t &wei_m,
        dnn_mem_t &bia_m, dnn_mem_t &dst_m);
void compute_ref_direct_bwd_d(const prb_t *p, dnn_mem_t &diff_src_m, dnn_mem_t &wei_m,
        dnn_mem_t &bia_m, dnn_mem_t &diff_dst_m);
void compute_ref_direct_bwd_w(const prb_t *p, dnn_mem_t &src_m, dnn_mem_t &diff_wei_m,
        dnn_mem_t &diff_bia_m, dnn_mem_t &diff_dst_m);

void compute_wino_ref_fwd(const prb_t *p, dnn_mem_t &src_m, dnn_mem_t &wei_m,
        dnn_mem_t &bia_m, dnn_mem_t &dst_m);
void compute_wino_ref_bwd_d(const prb_t *p, dnn_mem_t &idiff_src_m,
        dnn_mem_t &wei_m, dnn_mem_t &bia_m, dnn_mem_t &diff_dst_m);
void compute_wino_ref_bwd_w(const prb_t *p, dnn_mem_t &src_m,
        dnn_mem_t &diff_wei_m, dnn_mem_t &diff_bia_m, dnn_mem_t &diff_dst_m);

void perf_report(const prb_t *p, const res_t *r, const char *pstr);

int compare_src(const prb_t *p, dnn_mem_t &mem_dt, dnn_mem_t &mem_fp,
        res_t *r, bool final_compare = false);
int compare_wei(const prb_t *p, dnn_mem_t &mem_dt, dnn_mem_t &mem_fp,
        res_t *r, bool final_compare = false);
int compare_bia(const prb_t *p, dnn_mem_t &mem_dt, dnn_mem_t &mem_fp,
        res_t *r, bool final_compare = false);
int compare_dst(const prb_t *p, dnn_mem_t &mem_dt, dnn_mem_t &mem_fp,
        res_t *r, bool final_compare = false);
int fill_src(const prb_t *p, dnn_mem_t &mem_dt, dnn_mem_t &mem_fp,
        res_t *r);
int fill_wei(const prb_t *p, dnn_mem_t &mem_dt, dnn_mem_t &mem_fp,
    res_t *r);
int fill_bia(const prb_t *p, dnn_mem_t &mem_dt, dnn_mem_t &mem_fp,
        res_t *r);
int fill_dst(const prb_t *p, dnn_mem_t &mem_dt, dnn_mem_t &mem_fp,
        res_t *r);
double get_trust_nz_level(const prb_t *p, data_kind_t kind, bool final_compare);

void compute_ref_bwd_bias(const prb_t *p, dnn_mem_t &diff_bia_m, dnn_mem_t &diff_dst_m);
void compute_bias_fwd(const prb_t *p, dnn_mem_t &bia_m, dnn_mem_t &dst_m);
void compute_ref_bwd_weights(const prb_t *p, dnn_mem_t &src_m,dnn_mem_t &diff_wei_m, dnn_mem_t &diff_dst_m);

}

#endif
