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

#include "src/common/mkldnn_thread.hpp"

#include "bnorm/bnorm.hpp"

namespace bnorm {

void compute_ref_fwd(const prb_t *p, const dnn_mem_t &src, dnn_mem_t &mean,
        dnn_mem_t &var, const dnn_mem_t &ss, dnn_mem_t &dst) {
    auto maybe_post_ops = [&](float &bn_res, float dst) {
        const auto &ops = p->attr.post_ops;
        for (int idx = 0; idx < ops.len; ++idx) {
            using pk = attr_t::post_ops_t::kind_t;
            const auto &e = ops.entry[idx];
            switch (e.kind) {
            case pk::SUM:
                bn_res += e.sum.scale * dst;
                break;
            case pk::RELU:
                bn_res = e.eltwise.scale * (bn_res < 0 ? 0 : bn_res);
                break;
            default:
                assert(!"unknown attr::post_ops::kind");
            }
        }
    };

    mkldnn::impl::parallel_nd(p->ic, [&](int64_t c) {
        float smean = ((float *)mean)[c];
        float svar = ((float *)var)[c];
        float sqrt_var = sqrtf(svar + p->eps);

        float gamma = (p->flags & USE_SCALESHIFT ? ((float *)ss)[c] : 1.0f) / sqrt_var;
        float beta = p->flags & USE_SCALESHIFT ? ((float *)ss)[p->ic + c] : 0;

        for (int64_t mb = 0; mb < p->mb; ++mb)
        for (int64_t d = 0; d < p->id; ++d)
        for (int64_t h = 0; h < p->ih; ++h)
        for (int64_t w = 0; w < p->iw; ++w) {
            auto off = data_off(p, mb, c, d, h, w);
            float res = gamma * (((float *)src)[off] - smean) + beta;
            float &D = ((float *)dst)[off];
            if ((p->flags & FUSE_BN_RELU) && res < 0) res = 0;
            maybe_post_ops(res, D);
            D = res;
        }
    });
}

void compute_ref_bwd(const prb_t *p, const dnn_mem_t &src,
        const dnn_mem_t &mean, const dnn_mem_t &var, const dnn_mem_t &d_dst,
        const dnn_mem_t &ss, const dnn_mem_t &rmask, dnn_mem_t &d_src,
        dnn_mem_t &d_ss) {
    const float NHW = p->mb * p->id * p->ih * p->iw;

    mkldnn::impl::parallel_nd(p->ic, [&](int64_t c) {
        float smean = ((float *)mean)[c];
        float svar = ((float *)var)[c];
        float rcp_denom = 1.f / sqrtf(svar + p->eps);

        float gamma = p->flags & USE_SCALESHIFT ? ((float *)ss)[c] : 1;

        float d_gamma = 0;
        float d_beta = 0;

        for (int64_t mb = 0; mb < p->mb; ++mb)
        for (int64_t d = 0; d < p->id; ++d)
        for (int64_t h = 0; h < p->ih; ++h)
        for (int64_t w = 0; w < p->iw; ++w) {
            auto off = data_off(p, mb, c, d, h, w);
            float dd = ((float *)d_dst)[off];
            if ((p->flags & FUSE_BN_RELU) && ((float *)rmask)[off] == 0)
                dd = 0;

            d_gamma += dd * (((float *)src)[off] - smean);
            d_beta += dd;
        }
        d_gamma *= rcp_denom;

        if ((p->flags & USE_SCALESHIFT) && (p->dir & FLAG_WEI)) {
            ((float *)d_ss)[c] = d_gamma;
            ((float *)d_ss)[p->ic + c] = d_beta;
        }

        for (int64_t mb = 0; mb < p->mb; ++mb)
        for (int64_t d = 0; d < p->id; ++d)
        for (int64_t h = 0; h < p->ih; ++h)
        for (int64_t w = 0; w < p->iw; ++w) {
            auto off = data_off(p, mb, c, d, h, w);
            float dd = ((float *)d_dst)[off];
            if ((p->flags & FUSE_BN_RELU) && ((float *)rmask)[off] == 0)
                dd = 0;
            float ds = dd;

            if (!(p->flags & GLOB_STATS)) {
                const float x = ((float *)src)[off] - smean;
                ds -= (d_beta + x * d_gamma * rcp_denom) / NHW;
            }

            ((float *)d_src)[off] = rcp_denom * ds * gamma;
        }
    });
}

}
