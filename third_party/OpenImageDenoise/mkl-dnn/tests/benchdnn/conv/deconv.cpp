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
#include <stdio.h>
#include <float.h>
#include <math.h>

#include "mkldnn.h"

#include "src/common/mkldnn_thread.hpp"

#include "mkldnn_common.hpp"
#include "mkldnn_memory.hpp"

#include "norm.hpp"

#include "conv/deconv.hpp"
using namespace conv;

namespace deconv {

inline static void swap(int64_t &a, int64_t &b) {
    int64_t temp = a;
    a = b;
    b = temp;
}

inline bool is_deconv_3d(const prb_t *p) {
    return p->id > 1;
}

inline bool is_deconv_1d(const prb_t *p) {
    return !is_deconv_3d(p) && p->ih == 1 && p->kh == 1;
}

inline int transpose_data_wei(const prb_t *p, dnn_mem_t &wei, dnn_mem_t &wei_tr) {
    mkldnn::impl::parallel_nd(
        p->g, p->oc / p->g, p->ic / p->g, p->kd, p->kh, p->kw,
        [&](int64_t g, int64_t oc, int64_t ic, int64_t kd, int64_t kh, int64_t kw) {
        size_t idx = (((((size_t)g * p->ic / p->g + ic) * p->oc / p->g + oc)
        * p->kd + kd) * p->kh + kh) * p->kw + kw;
        ((float*)wei_tr)[idx] = ((float*)wei)[wei_off_f(p, g, oc, ic, kd, kh, kw)];
    });

    return OK;
}

inline int init_pd(const prb_t *p, mkldnn_deconvolution_desc_t &cd,
        mkldnn_primitive_desc_t &dpd, res_t *r) {
    int ndims = is_deconv_3d(p) ? 5 : is_deconv_1d(p) ? 3 : 4;

    mkldnn_memory_desc_t src_d, wei_d, bia_d, dst_d;
    mkldnn_dims_t src_1d_dims = {p->mb, p->ic, p->iw};
    mkldnn_dims_t src_2d_dims = {p->mb, p->ic, p->ih, p->iw};
    mkldnn_dims_t src_3d_dims = {p->mb, p->ic, p->id, p->ih, p->iw};
    mkldnn_dims_t wei_1d_dims = {p->g, p->oc / p->g, p->ic / p->g, p->kw};
    mkldnn_dims_t wei_2d_dims = {p->g, p->oc / p->g, p->ic / p->g, p->kh, p->kw};
    mkldnn_dims_t wei_3d_dims = {p->g, p->oc / p->g, p->ic / p->g, p->kd, p->kh, p->kw};
    mkldnn_dims_t bia_dims = {p->oc};
    mkldnn_dims_t dst_1d_dims = {p->mb, p->oc, p->ow};
    mkldnn_dims_t dst_2d_dims = {p->mb, p->oc, p->oh, p->ow};
    mkldnn_dims_t dst_3d_dims = {p->mb, p->oc, p->od, p->oh, p->ow};

    DNN_SAFE(mkldnn_memory_desc_init_by_tag(&src_d, ndims,
        is_deconv_3d(p) ? src_3d_dims : is_deconv_1d(p) ? src_1d_dims : src_2d_dims,
        p->cfg[SRC].dt, mkldnn_format_tag_any),
            WARN);
    DNN_SAFE(mkldnn_memory_desc_init_by_tag(&wei_d, ndims + p->has_groups,
        is_deconv_3d(p)
        ? &wei_3d_dims[!p->has_groups]
        : is_deconv_1d(p)
        ? &wei_1d_dims[!p->has_groups]
        : &wei_2d_dims[!p->has_groups],
        p->cfg[WEI].dt, mkldnn_format_tag_any), WARN);
    DNN_SAFE(mkldnn_memory_desc_init_by_tag(&bia_d, 1, bia_dims, p->cfg[BIA].dt, mkldnn_format_tag_any), WARN);
    DNN_SAFE(mkldnn_memory_desc_init_by_tag(&dst_d, ndims,
        is_deconv_3d(p) ? dst_3d_dims : is_deconv_1d(p) ? dst_1d_dims : dst_2d_dims,
        p->cfg[DST].dt, mkldnn_format_tag_any), WARN);

    mkldnn_dim_t strides_nd[] = {p->sd, p->sh, p->sw};
    mkldnn_dim_t dilates_nd[] = {p->dd, p->dh, p->dw};
    mkldnn_dim_t padding_nd[] = {p->pd, p->ph, p->pw};

    auto bph = [&](int64_t ih, int64_t oh, int64_t kh, int64_t sh, int64_t ph,
            int64_t dh) {
        return (oh - 1) * sh - ih + ((kh - 1) * (dh + 1) + 1) - ph;
    };

    mkldnn_dim_t padding_r_nd[] = {
        bph(p->od, p->id, p->kd, p->sd, p->pd, p->dd),
        bph(p->oh, p->ih, p->kh, p->sh, p->ph, p->dh),
        bph(p->ow, p->iw, p->kw, p->sw, p->pw, p->dw)};

    mkldnn_dim_t *strides = strides_nd + (5 - ndims);
    mkldnn_dim_t *dilates = dilates_nd + (5 - ndims);
    mkldnn_dim_t *padding = padding_nd + (5 - ndims);
    mkldnn_dim_t *padding_r = padding_r_nd + (5 - ndims);

    mkldnn_alg_kind_t alg = mkldnn_deconvolution_direct;
    if (p->alg == WINO) alg = mkldnn_deconvolution_winograd;

    switch (p->dir) {
    case FWD_D: case FWD_B:
        DNN_SAFE(mkldnn_dilated_deconvolution_forward_desc_init(&cd,
                    mkldnn_forward_inference, alg, &src_d, &wei_d,
                    p->dir == FWD_D ? NULL : &bia_d, &dst_d, strides,
                    dilates, padding, padding_r, mkldnn_padding_zero), WARN);
        break;
    case BWD_D:
        DNN_SAFE(mkldnn_dilated_deconvolution_backward_data_desc_init(&cd, alg,
                    &src_d, &wei_d, &dst_d, strides, dilates, padding,
                    padding_r, mkldnn_padding_zero), WARN);
        break;
    case BWD_W: case BWD_WB:
        DNN_SAFE(mkldnn_dilated_deconvolution_backward_weights_desc_init(&cd,
                    alg, &src_d, &wei_d, p->dir == BWD_W ? NULL : &bia_d,
                    &dst_d, strides, dilates,  padding, padding_r,
                    mkldnn_padding_zero), WARN);
        break;
    default: DNN_SAFE(mkldnn_invalid_arguments, CRIT);
    }

    DNN_SAFE(cd.accum_data_type == p->cfg[ACC].dt
            ? mkldnn_success : mkldnn_unimplemented, CRIT);

    auto mkldnn_attr = create_mkldnn_attr(p->attr, p->oc, p->scales);

    mkldnn_status_t init_status = mkldnn_success;
    init_status = mkldnn_primitive_desc_create(&dpd, &cd, mkldnn_attr,
            engine, NULL);

    mkldnn_primitive_attr_destroy(mkldnn_attr);

    if (init_status == mkldnn_unimplemented)
    {
        return r->state = UNIMPLEMENTED, OK;
    } else
        SAFE(init_status, WARN);

    const char *impl_str = query_impl_info(dpd);
    if (maybe_skip(skip_impl, impl_str)) {
        print(2, "SKIPPED: mkldnn implementation: %s\n", impl_str);
        DNN_SAFE(mkldnn_primitive_desc_destroy(dpd), WARN);
        return r->state = SKIPPED, OK;
    } else {
        print(5, "mkldnn implementation: %s\n", impl_str);
    }

    auto q = [=](mkldnn_query_t query, int index = 0)
    { return *mkldnn_primitive_desc_query_md(dpd, query, index); };

    if (p->dir == BWD_D)
        cd.diff_src_desc = q(mkldnn_query_diff_src_md);
    else
        cd.src_desc = q(mkldnn_query_src_md);

    if (p->dir & FLAG_WEI)
        cd.diff_weights_desc = q(mkldnn_query_diff_weights_md);
    else
        cd.weights_desc = q(mkldnn_query_weights_md);

    if (p->dir & FLAG_BIA) {
        if (p->dir & FLAG_BWD)
            cd.diff_bias_desc = q(mkldnn_query_diff_weights_md, 1);
        else
            cd.bias_desc = q(mkldnn_query_weights_md, 1);
    }

    if (p->dir & FLAG_BWD)
        cd.diff_dst_desc = q(mkldnn_query_diff_dst_md);
    else
        cd.dst_desc = q(mkldnn_query_dst_md);

    return OK;
}

int doit(const prb_t *p, res_t *r) {
    res_t res_zero{};
    *r = res_zero;
    bool with_groups = 1;

    prb_t p_tr((desc_t)*p, p->dir, p->cfg, p->alg, p->attr, p->mb, true);
    swap(p_tr.ic,  p_tr.oc);
    swap(p_tr.ih,  p_tr.oh);
    swap(p_tr.id,  p_tr.od);
    swap(p_tr.iw,  p_tr.ow);

    mkldnn_deconvolution_desc_t cd;
    mkldnn_primitive_desc_t dpd;
    mkldnn_primitive_t c{};

    SAFE(init_pd(p, cd, dpd, r), WARN);
    if (r->state == SKIPPED || r->state == UNIMPLEMENTED)
        return OK;

    DNN_SAFE(mkldnn_primitive_create(&c, dpd), WARN);
    DNN_SAFE_V(mkldnn_primitive_desc_destroy(dpd));

    auto &src_dt_d = p->dir == BWD_D ? cd.diff_src_desc : cd.src_desc;
    auto &wei_dt_d = p->dir & FLAG_WEI ? cd.diff_weights_desc : cd.weights_desc;
    auto &bia_dt_d = p->dir & FLAG_BWD ? cd.diff_bias_desc : cd.bias_desc;
    auto &dst_dt_d = p->dir & FLAG_BWD ? cd.diff_dst_desc: cd.dst_desc;
    auto wei_tr_dt_d = wei_dt_d;
    swap(wei_tr_dt_d.dims[with_groups+0], wei_tr_dt_d.dims[with_groups+1]);

    dnn_mem_t src_dt(src_dt_d, p->cfg[SRC].dt);
    dnn_mem_t wei_dt(wei_dt_d, p->cfg[WEI].dt);
    dnn_mem_t dst_dt(dst_dt_d, p->cfg[DST].dt);
    dnn_mem_t *p_bia_dt = p->dir & FLAG_BIA
        ? new dnn_mem_t(bia_dt_d, p->cfg[BIA].dt) : new dnn_mem_t();
    dnn_mem_t &bia_dt = *p_bia_dt;

    auto src_tag = get_default_tag(src_dt.md_.ndims);
    auto wei_tag = get_default_tag(wei_dt.md_.ndims);

    const auto fp = mkldnn_f32;

    /* memory for ref */
    dnn_mem_t src_fp(src_dt_d, fp, src_tag);
    dnn_mem_t wei_fp(wei_dt_d, fp, wei_tag);
    dnn_mem_t dst_fp(dst_dt_d, fp, src_tag);
    dnn_mem_t wei_tr_fp(wei_tr_dt_d, fp, wei_tag);
    dnn_mem_t *p_bia_fp = p->dir & FLAG_BIA
        ? new dnn_mem_t(bia_dt_d, fp, mkldnn_x) : new dnn_mem_t();
    dnn_mem_t *p_zero_fp = new dnn_mem_t();
    dnn_mem_t &bia_fp = *p_bia_fp, &zero_fp = *p_zero_fp;

    /* fill memory + reorders <-> */
    SAFE(fill_dst(p, dst_dt, dst_fp, r), WARN);
    SAFE(fill_wei(p, wei_dt, wei_fp, r), WARN);
    SAFE(fill_src(p, src_dt, src_fp, r), WARN);

    SAFE(transpose_data_wei(p, wei_fp, wei_tr_fp), WARN);
    if (p->dir & FLAG_BIA)
        SAFE(fill_bia(p, bia_dt, bia_fp, r), WARN);

    args_t args;

    if (p->dir & FLAG_FWD) {
        args.set(MKLDNN_ARG_SRC, src_dt.m_);
        args.set(MKLDNN_ARG_WEIGHTS, wei_dt.m_);
        if (p->dir & FLAG_BIA)
            args.set(MKLDNN_ARG_BIAS, bia_dt.m_);
        args.set(MKLDNN_ARG_DST, dst_dt.m_);

        DNN_SAFE(mkldnn_primitive_execute(c, stream, args.size(), args), WARN);

        if (bench_mode & CORR) {
            compute_ref_bwd_d(&p_tr, dst_fp, wei_tr_fp, bia_fp, src_fp);
            dnn_mem_t dst(dst_dt, fp, src_tag);
            SAFE(compare_dst(p, dst, dst_fp, r, true), WARN);
        }
    } else if (p->dir == BWD_D) {
        args.set(MKLDNN_ARG_DIFF_DST, dst_dt.m_);
        args.set(MKLDNN_ARG_WEIGHTS, wei_dt.m_);
        args.set(MKLDNN_ARG_DIFF_SRC, src_dt.m_);

        DNN_SAFE(mkldnn_primitive_execute(c, stream, args.size(), args), WARN);

        if (bench_mode & CORR) {
            compute_ref_fwd(&p_tr, dst_fp, wei_tr_fp, zero_fp, src_fp);
            dnn_mem_t src(src_dt, fp, src_tag);
            SAFE(compare_src(p, src, src_fp, r, true), WARN);
        }
    } else if (p->dir & FLAG_BWD && p->dir & FLAG_WEI) {
        args.set(MKLDNN_ARG_SRC, src_dt.m_);
        args.set(MKLDNN_ARG_DIFF_DST, dst_dt.m_);
        args.set(MKLDNN_ARG_DIFF_WEIGHTS, wei_dt.m_);
        if (p->dir & FLAG_BIA)
            args.set(MKLDNN_ARG_DIFF_BIAS, bia_dt.m_);

        DNN_SAFE(mkldnn_primitive_execute(c, stream, args.size(), args), WARN);

        if (bench_mode & CORR) {
            compute_ref_bwd_weights(&p_tr, dst_fp, wei_tr_fp, src_fp);
            transpose_data_wei(&p_tr, wei_tr_fp, wei_fp);
            dnn_mem_t wei(wei_dt, fp, wei_tag);
            SAFE(compare_wei(&p_tr, wei, wei_fp, r, true), WARN);
            if (p->dir & FLAG_BIA) {
                compute_ref_bwd_bias(p, bia_fp, dst_fp);
                dnn_mem_t bia(bia_dt, fp, mkldnn_x);
                SAFE(compare_bia(p, bia, bia_fp, r, true), WARN);
            }
        }
    } else {
        delete p_bia_dt;
        delete p_bia_fp;
        delete p_zero_fp;
        SAFE(FAIL, CRIT);
    }

    if (bench_mode & PERF) {
        auto &t = r->timer;
        t.reset();
        while (true) {
            DNN_SAFE(mkldnn_primitive_execute(c, stream, args.size(), args), WARN);
            t.stamp();
            const bool stop = false
                || (fix_times_per_prb && t.times() >= fix_times_per_prb)
                || (!fix_times_per_prb
                        && t.total_ms() >= max_ms_per_prb
                        && t.times() >= min_times_per_prb);
            if (stop) break;
        }
    }

    DNN_SAFE_V(mkldnn_primitive_destroy(c));

    delete p_bia_dt;
    delete p_bia_fp;
    delete p_zero_fp;

   return OK;
}

}
