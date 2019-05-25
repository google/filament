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
#include <stddef.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

#include "mkldnn.h"

#include "src/common/mkldnn_thread.hpp"

#include "mkldnn_common.hpp"
#include "mkldnn_memory.hpp"
#include "norm.hpp"

#include "bnorm/bnorm.hpp"

namespace bnorm {

static int prepare_fwd(const prb_t *p, dnn_mem_t &src, dnn_mem_t &mean,
        dnn_mem_t &var, dnn_mem_t &ss) {
    /** Idea: choose src[] values so that both mean and variance are computed
     * exactly (independently of the order of the computations).
     *
     * The `exactness` is achieved via [a1]: src[i] + src[i+1] = 2 * mean.
     *
     * The variation in src is allowed in the last flex_bits bits.
     * If the sequence (L) is too big (flex_bits <= min_flex_bits), the mean
     * value is set to 0 and src is partially filled with zeros (according to
     * density so that at least want_flex_bits is reserved for src variation.
     * Once src is set, variance is computed.
     *
     * ALG_0: mean is set to 0
     * ALG_1: mean is set to 2^p, where p \in {-2, -1, ..., 4}
     * ALG_AUTO: choose between ALG_0 and ALG_1 automatically */
    const int64_t exact_bits = 24;
    const int64_t L = p->mb * p->id * p->ih * p->iw;
    const int64_t logL = (int64_t)ceilf(log2f(L));

    assert(logL <= 0 || (1<<(logL-1)) < L);
    assert(L <= (1<<logL));

    const int64_t min_flex_bits = 3;
    const int64_t want_flex_bits = 6;

    check_alg_t alg = p->check_alg;
    if (alg == ALG_AUTO) /* choose appropriate checking algorithm */
        alg = (exact_bits - logL) / 2 - 1 >= min_flex_bits ? ALG_1 : ALG_0;

    const int64_t flex_bits = alg == ALG_0
        ? want_flex_bits : ((exact_bits - logL) / 2 - 1);

    if (flex_bits < min_flex_bits)
        return FAIL;

    const int64_t flex_mask = (1 << flex_bits) - 1;

    /* density: (exact_bits - log_2(L * density)) / 2 >= flex_bits */
    const float density = alg == ALG_0
        ? 1.f * (1 << (exact_bits - 2 * flex_bits)) / L : 1.f;
    assert((exact_bits - ceilf(log2f(L * density))) / 2 >= flex_bits);

    print(6, "check_alg: %s, density = %g, flex_bits = " IFMT "\n",
            check_alg2str(alg), density, flex_bits);

    mkldnn::impl::parallel_nd(p->ic, [&](int64_t c) {
        const float m = ((float *)mean)[c] =
            alg == ALG_0 ? 0.f : 0.25f * (1 << (c % 7));
        float v = 0; /* current variance */

        for (int64_t mb = 0; mb < p->mb; ++mb) {
            int64_t l_base = mb * p->id * p->ih * p->iw + c * 239 * 2; // l[0] must be even
            float *s = (float *)src + data_off(p, mb, c, 0, 0, 0);

            for (int64_t d = 0; d < p->id; ++d)
            for (int64_t h = 0; h < p->ih; ++h)
            for (int64_t w = 0; w < p->iw; ++w) {

                const int64_t sp = d * p->ih * p->iw + h * p->iw + w;
                const int64_t l = l_base + sp;

                if (alg == ALG_0 && !flip_coin(l/2 * 257ULL, density)) {
                    s[sp] = 0;
                    continue;
                }

                const int64_t gen = (l / 2 * 1637) & flex_mask;
                const int sgn = l % 2 == 0 ? 1 : -1; /* [a1] */
                const float f = 1.f * sgn * gen / (1 << flex_bits);

                s[sp] = alg == ALG_0 ? f : m * (1.f + f);
                if (L % 2 && (mb * p->id * p->ih * p->iw + sp == L - 1)) {
                    s[sp] = m;
                }
                v += (s[sp] - m) * (s[sp] - m);
            }
        }

        ((float *)var)[c] = v / (p->mb * p->id * p->ih * p->iw);

        if (p->flags & USE_SCALESHIFT) {
            ((float *)ss)[c] = 1.f / 8 * (1 << (c % 7));
            ((float *)ss)[p->ic + c] = ((c % 3) - 1) * ((float *)ss)[c] / 64;
        } else {
            ((float *)ss)[c] = 1;
            ((float *)ss)[p->ic + c] = 0;
        }
    });

    return OK;
}

/** @brief L = 2^k * P, P % 2 != 0 */
static void decompose2(int64_t L, int64_t &k, int64_t &P) {
    P = L;
    for (k = 0; P % 2 == 0; ++k)
        P /= 2;
}

static int prepare_bwd(const prb_t *p, dnn_mem_t &src, dnn_mem_t &d_dst,
        dnn_mem_t &mean, dnn_mem_t &var, dnn_mem_t &ss, dnn_mem_t &mask) {
    const int64_t exact_bits = 24;

    const int64_t L = p->mb * p->id * p->ih * p->iw;
    if (L < 2)
        return FAIL;

    /** Stabilization idea...
     * Since
     *      d_src = func(d_beta / L, d_gamma' / L, ...)
     * try to make d_beta = L / 2^t_beta and d_gamma' = L / 2^t_gamma,
     * where both t_beta and t_gamma are in {1, .., max_k}.
     * Currently, with no obvious reason, max_k is set to 4 for
     * reasonably small problems and to 8 for big problems.
     *
     * Here d_gamma' = d_gamma / sqrt(var + eps).
     * We might hope that division by L would be exact in that case,
     * but that might happen iff L is less than 2^exact_bits, hence
     * restriction [r1]. */

    int64_t k, P;
    decompose2(L, k, P);

    int64_t log2P = (int64_t)ceilf(log2f(P));
    if (log2P >= exact_bits)
        return FAIL; /* [r1] */

    const int64_t max_k = L > (1<<20) ? 8 : 4;
    if (k > max_k && exact_bits - log2P > max_k + 4) {
        log2P += (k - max_k);
        P <<= k - max_k;
        k = max_k;
    }

    const int64_t param_dd_p2 = 7;   // factor_dd <- 2^{0, .., -param_db_p2+1}
    const int64_t param_dd_gen = 32; // gen_dd <- {1, .., param_dd_gen}

    const int64_t param_f_p2 = 1;    // factor_f <- 2^{-param_dg_p2}
    const int64_t param_f_gen = 16;  // gen_f <- {2..param_s_gen}

    const float ub_dg = param_dd_gen * param_f_gen / 2 * L;
    const float ub_db = param_dd_gen * L;
    const float density = MIN3(1.f, (1<<exact_bits) / ub_dg,
            (1<<exact_bits) / ub_db);

    print(5, "prep_bwd: k:" IFMT ", P:" IFMT " log2P:" IFMT ", density = %g\n",
            k, P, log2P, density);

    mkldnn::impl::parallel_nd(p->ic, [&](int64_t c) {
        const float m = ((float *)mean)[c] = c % 2;

        /* var + eps \in {1/4, 1, 4} */
        const float ve_denom = 4.f / (1 << 2 * (c % 3));
        ((float *)var)[c] = ve_denom - p->eps;

        const int64_t dd_p2 = (c * 127 % param_dd_p2);
        const float factor_dd = 1.f / (1 << dd_p2);

        const int64_t f_p2 = 1 + (c % param_f_p2);
        const float factor_f = 1.f / (1 << f_p2);

        const float target_db = factor_dd * P;
        const float target_dg = ve_denom * 2 * target_db;

        float dg = 0, db = 0; /* current d_beta and d_gamma */
        for (int64_t mb = 0; mb < p->mb; ++mb) {
            const int64_t l_base = mb * p->id * p->ih * p->iw;

            const auto off = data_off(p, mb, c, 0, 0, 0);
            float *s = (float *)src + off;
            float *dd = (float *)d_dst + off;
            float *rmask = (float *)mask + off;

            for (int64_t d = 0; d < p->id; ++d)
            for (int64_t h = 0; h < p->ih; ++h)
            for (int64_t w = 0; w < p->iw; ++w) {

                const int64_t sp = d * p->ih * p->iw + h * p->iw + w;
                if (!flip_coin(l_base + sp, density) && l_base + sp + 100 < L) {
                    dd[sp] = 0;
                    s[sp] = m;
                    rmask[sp] = 1;
                    continue;
                }
                if (l_base + sp + 2 >= L) continue; /* last 2 are special */
                const int64_t l = l_base + sp * 7 + c * 19 + mb * 13;

                int64_t rmask_v = 1;
                if (p->flags & FUSE_BN_RELU)
                    rmask[sp] = rmask_v = l % 5 != 1;

                const int sgn_dd = db < target_db ? 1 : -1;
                dd[sp] = sgn_dd * factor_dd * (1 + (l * 3 % param_dd_gen));
                if (rmask_v) db += dd[sp];

                const int sgn_f = dg < target_dg ? 1 : -1;
                const float f =
                    sgn_f * factor_f * (2 + (l * 7 % (param_f_gen - 1)));

                if (rmask_v) dg += f * dd[sp];
                s[sp] = f + m;
            }
        }

        if (1) {
            /* the last 2 elements in src and d_dst are set, so that:
             *      db == target_db
             *      dg == target_dg
             * For this we need to solve the system:
             *      d_dst[l1]           + d_dst[l0]           = target_db - db
             *      d_dst[l1] * src[l1] + d_dst[l0] * src[l0] = target_dg - dg
             *
             * Here l0 -- last index, l1 -- last but one.
             * More over, let's assume src[l1] = 1 and src[l0] = -1. */
            int64_t l0 = data_off(p, p->mb - 1, c, p->id - 1, p->ih - 1,
                    p->iw - 1);
            int64_t l1 = l0 - 1;
            if (p->id == 1 && p->ih == 1 && p->iw == 1)
                l1 = data_off(p, p->mb - 2, c, p->id - 1, p->ih - 1, p->iw - 1);

            ((float *)src)[l1] = 1.f;
            ((float *)src)[l0] = -1.f;
            if (p->flags & FUSE_BN_RELU)
                ((float *)mask)[l0] = ((float *)mask)[l1] = 1;

            float f1 = ((target_db - db) + (target_dg - dg)) /2;
            float f0 = ((target_db - db) - (target_dg - dg)) /2;

            ((float *)d_dst)[l1] = f1 + m;
            ((float *)d_dst)[l0] = f0 + m;
        }

        if (p->flags & USE_SCALESHIFT) {
            ((float *)ss)[c] = 1.f / 2 * (1 << (c % 7));
            ((float *)ss)[p->ic + c] = ((float *)ss)[c] / 64;
        } else {
            ((float *)ss)[c] = 1;
            ((float *)ss)[p->ic + c] = 0;
        }
    });

    return OK;
}

static int compare(const prb_t *p, data_kind_t kind, const dnn_mem_t &fp_mem,
        const dnn_mem_t &dt_mem, res_t *r, const dnn_mem_t *ss = nullptr) {
    const char *skind = data_kind2str(kind);
    const float eps = p->dir & FLAG_FWD
        ? (kind == DATA ? 5e-7 : 0)
        : (kind == DATA ? 2e-7 : 0);

    /* With all the stability tricks bwd_d is still pretty unstable.
     * So let's rely on relative error in L1, L2, and L_inf norms.
     * TODO: make computations for bwd_d more stable and use `L0` here. */
    const bool rely_on_norm = false
        || (kind == DATA && (p->dir & FLAG_BWD) && (p->flags | GLOB_STATS));


    const int64_t N = kind == DATA ? p->mb : 1;
    const int64_t C = kind == DATA ? p->ic : p->ic * (kind == SS ? 2 : 1);
    const int64_t SP = kind == DATA ? p->id * p->ih * p->iw : 1;
    const int64_t nelems = N * C * SP;
    r->total += rely_on_norm ? 1 : nelems;

    diff_norm_t diff_norm;
    for (int64_t n = 0; n < N; n++) {
    for (int64_t c = 0; c < C; c++) {
    for (int64_t sp = 0; sp < SP; ++sp) {
        int64_t i = (n * C + c) * SP + sp;
        const float fp = ((const float *)fp_mem)[i];
        const float dt = ((const float *)dt_mem)[i];
        diff_norm.update(fp, dt);

        if (rely_on_norm)
            continue;

        const float diff = fabsf(fp - dt);
        const float rel_diff = diff / (fabsf(fp) > FLT_MIN ? fabsf(fp) : 1);
        bool ok = (fabsf(fp) > 1e-5 ? rel_diff : diff) <= eps;

        /* When the error is larger than eps, It could be
         * due to catastrophic cancellation in final result
         * which is computed as `Y = a * X + b`.
         * When `a * X`  is close to `b` and `sign(a * X) = - sign(b)`.
         * Then large error in `a * X` could result in a final
         * result (which has a cancellation i.e. `|Y| = |a*X - (-b)|`)
         * which has no meaningful digits left in mantissa.*/
        if (!ok && (p->dir & FLAG_FWD) && kind == DATA && ss) {
            const float beta = ((float *)*ss)[p->ic + c];
            /* Using an empirically derived threshold,
             * check if cancellation error
             * in `|Y| = |a*X - (-b)|` is huge.*/
            bool maybe_cancellation_error = (fabsf(fp - beta) /
                    (fabsf(fp) > FLT_MIN ? fabsf(fp) : 1)) > 1.0f;
            if (maybe_cancellation_error) {
                /* Check for error in `a * X` */
                float diff_aX = fabsf((fp - beta) - (dt - beta));
                float rel_diff_aX = diff_aX /
                    (fabsf(fp - beta) > FLT_MIN ? fabsf(fp - beta) : 1);
                ok = rel_diff_aX <= eps;
            }
        }

        r->errors += !ok;

        bool dump = false
            || (!ok && (r->errors < 10 || verbose >= 10))
            || (verbose >= 50 && i < 30);
        if (dump) {
            const int ind_str_len = 64;
            char ind_str[ind_str_len] = {'\0'};
            if (kind == DATA) {
                int64_t mb, c, d, h, w;
                inv_data_off(p, i, mb, c, d, h, w);
                snprintf(ind_str, ind_str_len, "" IFMT "," IFMT ",", mb, c);
                snprintf(ind_str, ind_str_len, "" IFMT "," IFMT "," IFMT "",
                        d, h, w);
            } else if (kind == SS) {
                snprintf(ind_str, ind_str_len, "" IFMT "," IFMT "",
                        i / p->ic, i % p->ic);
            } else {
                snprintf(ind_str, ind_str_len, "" IFMT "", i);
            }

            print(0, "[%lu][%s%s][%s] fp:%8g dt:%8g diff:%8g rdiff:%8g\n",
                    (unsigned long)i, p->dir & FLAG_BWD ? "D_" : "", skind,
                    ind_str, fp, dt, diff, rel_diff);
        }
    }
    }
    }

    diff_norm.done();

    if (rely_on_norm) {
        r->errors += false
            || diff_norm.rel_diff(norm_t::L1) > eps
            || diff_norm.rel_diff(norm_t::L2) > eps
            || diff_norm.rel_diff(norm_t::L8) > eps;
    }

    if (r->errors || verbose >= 5) {
        const int vl = r->errors ? 0 : 2;
        print(vl, "@@@ [%s%s] diff: l0(``%g``) "
                "l1:(%g,%g,%g,``%g``) "
                "l2:(%g,%g,%g,``%g``) "
                "l8:(%g,%g,%g,``%g``)\n",
                p->dir & FLAG_BWD ? "D_" : "", skind,
                diff_norm.rel_diff(norm_t::L0),
                diff_norm.a_[norm_t::L1], diff_norm.b_[norm_t::L1],
                diff_norm.diff_[norm_t::L1], diff_norm.rel_diff(norm_t::L1),
                diff_norm.a_[norm_t::L2], diff_norm.b_[norm_t::L2],
                diff_norm.diff_[norm_t::L2], diff_norm.rel_diff(norm_t::L2),
                diff_norm.a_[norm_t::L8], diff_norm.b_[norm_t::L8],
                diff_norm.diff_[norm_t::L8], diff_norm.rel_diff(norm_t::L8));
    }

    if (r->errors)
        r->state = FAILED;

    if (r->state == UNTESTED)
        r->state = PASSED; /* optimism */

    return r->state == FAILED ? FAIL : OK;
}

int check_fwd_ws(const dnn_mem_t &data_dt, const dnn_mem_t &ws_dt, res_t *r) {
    /* so far we know ws is just bit-mask of whether value was negative or
     * positive */
    const int64_t nelems = data_dt.nelems(true);
    const float *d = (const float *)data_dt;
    const uint8_t *ws = (const uint8_t *)ws_dt;

    /* some internal knowledge: flags in ws are either stored as bytes (e.g.
     * for the ref implementation) or as bits (e.g. for the jitted one); in
     * the latter case the ws memory has fewer elements than the data memory */
    enum { ws_byte, ws_bit } ws_type;
    ws_type = ws_dt.nelems(true) < nelems ? ws_bit : ws_byte;

    /* more internal knowledge: data_dt and ws_dt are expected to have exactly
     * the same data layout, and data_dt padded regions are expected to be
     * zero, and the respective ws_dt elements should be set accordingly */
    for (int64_t i = 0; i < nelems; i += 8) {
        for (int64_t j = 0; j < MIN2(8, nelems - i); ++j) {
            const bool want = *d > 0;
            const bool bit_set = ws_type == ws_byte ? *ws : !!(*ws & (1<<j));

            const bool ok = bit_set == want;
            r->errors += !ok;

            bool dump = false
                || (!ok && (r->errors < 10 || verbose >= 10))
                || (verbose >= 50 && i < 30);
            if (dump) {
                print(0, "[%lu] ws exp:%d got:%d (data:%g:%a)\n",
                        (unsigned long)(i + j), want, bit_set, *d, *d);
            }

            ++d;
            if (ws_type == ws_byte) ++ws;
        }
        if (ws_type == ws_bit) ++ws;
    }

    if (r->errors)
        r->state = FAILED;

    if (r->state == UNTESTED)
        r->state = PASSED; /* optimism */

    return r->state == FAILED ? FAIL : OK;
}

static int init_pd(const prb_t *p, mkldnn_batch_normalization_desc_t &bd,
        mkldnn_primitive_desc_t &bpd, res_t *r) {
    mkldnn_memory_desc_t data_d;
    mkldnn_dims_t data_dims = {p->mb, p->ic, p->ih, p->iw};
    mkldnn_dims_t data_dims_3d = {p->mb, p->ic, p->id, p->ih, p->iw};
    DNN_SAFE(mkldnn_memory_desc_init_by_tag(&data_d, is_bnorm_3d(p) ? 5 : 4,
        is_bnorm_3d(p) ? data_dims_3d : data_dims, p->dt, p->tag), WARN);

    auto flags = (mkldnn_batch_normalization_flag_t)p->flags;
    if (p->dir & FLAG_FWD) {
        auto prop = p->dir & FLAG_INF
            ? mkldnn_forward_inference : mkldnn_forward_training;
        DNN_SAFE(mkldnn_batch_normalization_forward_desc_init(&bd, prop,
                    &data_d, p->eps, flags), WARN);

    } else {
        auto prop = p->dir & FLAG_WEI
            ? mkldnn_backward : mkldnn_backward_data;
        DNN_SAFE(mkldnn_batch_normalization_backward_desc_init(&bd, prop,
                    &data_d, &data_d, p->eps, flags), WARN);
    }

    auto mkldnn_attr = create_mkldnn_attr(p->attr, 1, NULL);

    mkldnn_primitive_desc_t hint_fwd_pd = NULL;
    if (p->dir & FLAG_BWD) {
        mkldnn_batch_normalization_desc_t bd_fwd;
        DNN_SAFE(mkldnn_batch_normalization_forward_desc_init(&bd_fwd,
                    mkldnn_forward_training, &data_d, p->eps, flags), WARN);
        DNN_SAFE(mkldnn_primitive_desc_create(&hint_fwd_pd, &bd_fwd, NULL,
                    engine, NULL), WARN);
    }
    mkldnn_status_t init_status = mkldnn_primitive_desc_create(&bpd, &bd,
            mkldnn_attr, engine, hint_fwd_pd);

    mkldnn_primitive_desc_destroy(hint_fwd_pd);
    mkldnn_primitive_attr_destroy(mkldnn_attr);

    if (init_status == mkldnn_unimplemented)
        return r->state = UNIMPLEMENTED, OK;
    else
        SAFE(init_status, WARN);

    const char *impl_str = query_impl_info(bpd);
    if (maybe_skip(skip_impl, impl_str)) {
        print(2, "SKIPPED: mkldnn implementation: %s\n", impl_str);
        DNN_SAFE(mkldnn_primitive_desc_destroy(bpd), WARN);
        return r->state = SKIPPED, OK;
    } else {
        print(5, "mkldnn implementation: %s\n", impl_str);
        if (!strstr(impl_str, "jit")) {
            print(1, "WARNING: %s",
                    "accuracy of the implementation being tested "
                    "depends on the compiler and might give false-positives.\n");
            print(1, "         %s",
                    "please consider recompiling the sources with"
                    " `-prec-div -fp-model precise` for a reliable testing.\n");
        }
    }

    return OK;
}

/** converts benchdnn-understandable mask of {0, 1} to workspace */
static int cvt_mask_to_ws(const prb_t *p, const dnn_mem_t &mask_fp,
        dnn_mem_t &ws_dt) {
    mkldnn_dims_t data_dims = {p->mb, p->ic, p->ih, p->iw};
    mkldnn_dims_t data_dims_3d = {p->mb, p->ic, p->id, p->ih, p->iw};

    dnn_mem_t data(is_bnorm_3d(p) ? 5 : 4,
        is_bnorm_3d(p) ? data_dims_3d : data_dims, mkldnn_f32, p->tag);
    SAFE(data.reorder(mask_fp), WARN);

    dnn_mem_t mean(1, &p->ic, mkldnn_f32, mkldnn_x);
    dnn_mem_t var(1, &p->ic, mkldnn_f32, mkldnn_x);
    for (int64_t c = 0; c < p->ic; ++c) ((float *)mean)[c] = 0.5;
    for (int64_t c = 0; c < p->ic; ++c) ((float *)var)[c] = 1;

    mkldnn_batch_normalization_desc_t bd;
    auto flags = (mkldnn_batch_normalization_flag_t)
        (mkldnn_use_global_stats | mkldnn_fuse_bn_relu);
    DNN_SAFE(mkldnn_batch_normalization_forward_desc_init(&bd,
                mkldnn_forward_training, &data.md_, 0, flags), WARN);

    mkldnn_primitive_desc_t bpd;
    DNN_SAFE(mkldnn_primitive_desc_create(&bpd, &bd, NULL, engine, NULL),
            WARN);

    mkldnn_primitive_t b{};
    DNN_SAFE(mkldnn_primitive_create(&b, bpd), WARN);
    DNN_SAFE(mkldnn_primitive_desc_destroy(bpd), CRIT);

    args_t args;
    args.set(MKLDNN_ARG_SRC, data.m_);
    args.set(MKLDNN_ARG_MEAN, mean.m_);
    args.set(MKLDNN_ARG_VARIANCE, var.m_);
    args.set(MKLDNN_ARG_DST, data.m_);
    args.set(MKLDNN_ARG_WORKSPACE, ws_dt.m_);
    DNN_SAFE(mkldnn_primitive_execute(b, stream, args.size(), args), WARN);
    DNN_SAFE(mkldnn_primitive_destroy(b), CRIT);

    return OK;
}

int doit(const prb_t *p, res_t *r) {
    res_t res_zero{};
    *r = res_zero;

    mkldnn_batch_normalization_desc_t bd;
    mkldnn_primitive_desc_t bpd;
    mkldnn_primitive_t b{};

    SAFE(init_pd(p, bd, bpd, r), WARN);
    if (r->state == SKIPPED || r->state == UNIMPLEMENTED)
        return OK;

    const auto fp = mkldnn_f32;
    auto &data_dt_d = bd.data_desc;

    const mkldnn_dims_t dims1d = {p->ic};
    const mkldnn_dims_t dims2d = {2, p->ic};
    const auto src_tag = is_bnorm_3d(p) ? mkldnn_ncdhw : mkldnn_nchw;

    dnn_mem_t data_fp(data_dt_d, fp, src_tag),
              data_dt(data_dt_d);
    dnn_mem_t d_data_fp(data_dt_d, fp, src_tag),
              d_data_dt(data_dt_d);

    dnn_mem_t mean_fp(1, dims1d, fp, mkldnn_x),
              mean_dt(mean_fp.md_);
    dnn_mem_t var_fp(1, dims1d, fp, mkldnn_x),
              var_dt(var_fp.md_);

    dnn_mem_t ss_fp(2, dims2d, fp, mkldnn_nc),
              ss_dt(ss_fp.md_);
    dnn_mem_t d_ss_fp(2, dims2d, fp, mkldnn_nc),
              d_ss_dt(d_ss_fp.md_);

    dnn_mem_t ws_fp(data_fp.md_);
    dnn_mem_t *p_ws_dt = NULL;
    if ((p->flags & FUSE_BN_RELU) && !(p->dir & FLAG_INF)) {
        const auto ws_md = mkldnn_primitive_desc_query_md(bpd,
                mkldnn_query_workspace_md, 0);
        SAFE(ws_md != NULL ? OK : FAIL, WARN);
        p_ws_dt = new dnn_mem_t(*ws_md);
    } else {
        p_ws_dt = new dnn_mem_t();
    }
    dnn_mem_t &ws_dt = *p_ws_dt;

    DNN_SAFE(mkldnn_primitive_create(&b, bpd), WARN);
    DNN_SAFE(mkldnn_primitive_desc_destroy(bpd), CRIT);

    args_t args;

    if (p->dir & FLAG_FWD) {
        if (prepare_fwd(p, data_fp, mean_fp, var_fp, ss_fp) != OK)
            return r->state = MISTRUSTED, OK;

        SAFE(data_dt.reorder(data_fp), WARN);

        /* always in-place so far... */
        args.set(MKLDNN_ARG_SRC, data_dt.m_);
        args.set(MKLDNN_ARG_DST, data_dt.m_);

        if (p->flags & GLOB_STATS) {
            /* prepare mean & var if they are inputs */
            SAFE(mean_dt.reorder(mean_fp), WARN);
            SAFE(var_dt.reorder(var_fp), WARN);
        }
        args.set(MKLDNN_ARG_MEAN, mean_dt.m_);
        args.set(MKLDNN_ARG_VARIANCE, var_dt.m_);

        if (p->flags & USE_SCALESHIFT) {
            SAFE(ss_dt.reorder(ss_fp), WARN);
            args.set(MKLDNN_ARG_SCALE_SHIFT, ss_dt.m_);
        }

        if (p->flags & FUSE_BN_RELU)
            args.set(MKLDNN_ARG_WORKSPACE, ws_dt.m_);

        DNN_SAFE(mkldnn_primitive_execute(b, stream, args.size(), args), WARN);

        if (bench_mode & CORR) {
            compute_ref_fwd(p, data_fp, mean_fp, var_fp, ss_fp, data_fp);
            if (!(p->flags & GLOB_STATS) && !(p->dir & FLAG_INF)) {
                SAFE(compare(p, MEAN, mean_fp, mean_dt, r), WARN);
                SAFE(compare(p, VAR, var_fp, var_dt, r), WARN);
            }
            dnn_mem_t data(data_dt, fp, src_tag);
            SAFE(compare(p, DATA, data_fp, data, r, &ss_fp), WARN);
            if ((p->flags & FUSE_BN_RELU) && !(p->dir & FLAG_INF))
                SAFE(check_fwd_ws(data_dt, ws_dt, r), WARN);
        }
    } else {
        if (prepare_bwd(p, data_fp, d_data_fp, mean_fp, var_fp, ss_fp, ws_fp)
                != OK)
            return r->state = MISTRUSTED, OK;

        SAFE(data_dt.reorder(data_fp), WARN);
        args.set(MKLDNN_ARG_SRC, data_dt.m_);

        SAFE(d_data_dt.reorder(d_data_fp), WARN);
        /* always in-place so far... */
        args.set(MKLDNN_ARG_DIFF_DST, d_data_dt.m_);
        args.set(MKLDNN_ARG_DIFF_SRC, d_data_dt.m_);

        SAFE(mean_dt.reorder(mean_fp), WARN);
        SAFE(var_dt.reorder(var_fp), WARN);
        args.set(MKLDNN_ARG_MEAN, mean_dt.m_);
        args.set(MKLDNN_ARG_VARIANCE, var_dt.m_);

        if (p->flags & USE_SCALESHIFT) {
            SAFE(ss_dt.reorder(ss_fp), WARN);
            args.set(MKLDNN_ARG_SCALE_SHIFT, ss_dt.m_);
            args.set(MKLDNN_ARG_DIFF_SCALE_SHIFT, d_ss_dt.m_);
        }

        if (p->flags & FUSE_BN_RELU) {
            SAFE(cvt_mask_to_ws(p, ws_fp, ws_dt), WARN);
            args.set(MKLDNN_ARG_WORKSPACE, ws_dt.m_);
        }

        DNN_SAFE(mkldnn_primitive_execute(b, stream, args.size(), args), WARN);

        if (bench_mode & CORR) {
            compute_ref_bwd(p, data_fp, mean_fp, var_fp, d_data_fp, ss_fp,
                    ws_fp, d_data_fp, d_ss_fp);
            if ((p->flags & USE_SCALESHIFT) && (p->dir & FLAG_WEI))
                SAFE(compare(p, SS, d_ss_fp, d_ss_dt, r), WARN);
            dnn_mem_t d_data(d_data_dt, fp,
            is_bnorm_3d(p) ? mkldnn_ncdhw : mkldnn_nchw);
            SAFE(compare(p, DATA, d_data_fp, d_data, r), WARN);
        }
    }

    if (bench_mode & PERF) {
        auto &t = r->timer;
        t.reset();
        while (true) {
            DNN_SAFE(mkldnn_primitive_execute(b, stream, args.size(), args), WARN);
            t.stamp();
            const bool stop = false
                || (fix_times_per_prb && t.times() >= fix_times_per_prb)
                || (!fix_times_per_prb
                        && t.total_ms() >= max_ms_per_prb
                        && t.times() >= min_times_per_prb);
            if (stop) break;
        }
    }

    delete p_ws_dt;
    DNN_SAFE(mkldnn_primitive_destroy(b), CRIT);

    return OK;
}

}
