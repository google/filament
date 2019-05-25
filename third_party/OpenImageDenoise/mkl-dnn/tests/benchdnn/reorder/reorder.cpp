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

#include "dnn_types.hpp"
#include "mkldnn_common.hpp"
#include "mkldnn_memory.hpp"

#include "reorder.hpp"

namespace reorder {

int get_scale_mask(const mkldnn_memory_desc_t &md, const attr_t &attr) {
    using P = attr_t::scale_t::policy_t;
    const auto policy = attr.oscale.policy;

    int scale_mask = 0;

    switch (policy) {
    case P::PER_DIM_0: scale_mask = (1 << 0); break;
    case P::PER_DIM_1: scale_mask = (1 << 1); break;
    case P::PER_DIM_01: scale_mask = (1 << 0) + (1 << 1); break;
    case P::COMMON:
    case P::NONE: scale_mask = 0; break;
    default: SAFE_V(FAIL);
    }

    return scale_mask;
}

int scales_count(int64_t *count, int *mask, const dnn_mem_t &memory,
        const attr_t &attr) {
    const mkldnn_memory_desc_t &md = memory.md_;
    const int scale_mask = get_scale_mask(md, attr);
    if (mask) *mask = scale_mask;

    int64_t uniq_scales = 1;
    for(int d = 0; d < md.ndims; ++d) {
        if (scale_mask & (1 << d))
            uniq_scales *= md.dims[d];
    }
    *count = uniq_scales;
    return OK;
}

int fill_scales(const prb_t *p, float *scales, int64_t count) {
    const float scale_value = p->attr.oscale.scale;

    for (int64_t i = 0; i < count; ++i)
        scales[i] = scale_value;

    if (count != 1) scales[count - 1] = scale_value + 1.1;

    return OK;
}

inline float saturate(float value, float min, float max) {
    return MAX2(min, MIN2(max, value));
}

int fill_memory(const prb_t *p, dnn_mem_t &mem, const float *scales,
        const attr_t &attr) {
    const dt_conf_t c_src = p->conf_in;
    const int range = c_src->range;
    const int max = c_src->min + range - 1;
    int scale_mask = get_scale_mask(mem.md_, attr);

    const int64_t nelems = mem.nelems();

    for (int64_t idx = 0; idx < nelems; ++idx) {
        const int64_t mask_idx = mem.get_scale_idx(idx, scale_mask);
        const float scale = scales[mask_idx];

        const float gen[7] = {
            (float)max, /* saturate to max of output data type */
            (float)c_src->min, /* saturate to min of output data type */
            (float)1.6 / scale, /* rounding check */
            (float)0.2 / scale, /* saturate to 0 */
            (float)1.0,
            (float)2.0,
            (float)scale,
        };

        float value = saturate(gen[idx % 7], c_src->min, max);
        mem.set_elem(idx, value);
    }

    return OK;
}

/* TODO: Complete */
int reorder(const prb_t *p, dnn_mem_t &dst, const dnn_mem_t &src,
        const float *scales) {
    auto dst_dt = dst.dt();

    int64_t nelems = src.nelems();

    /* calculate min max for data_type */
    /* TODO: add dst range support */
//    const auto c_dst = p->conf_out;
//    const float dst_conf_min = c_dst.min;
//    const float dst_conf_max = dst_conf_min + c_dst.range - 1;

    auto dst_width = dst.sizeof_dt() * 8;

    const float dst_dt_min = dst_dt == mkldnn_u8
        ? 0.f : -(float)(1l << (dst_width - 1));
    const float dst_dt_max = dst_dt == mkldnn_u8
        ? 255.f : (float)((1l << (dst_width - 1)) - 1);

    /* TODO: add dst range support */
//    const float dst_max = MIN2(dst_conf_max, dst_dt_max);
//    const float dst_min = MAX2(dst_conf_min, dst_dt_min);
    const float dst_max = dst_dt_max;
    const float dst_min = dst_dt_min;

    const int scale_mask = get_scale_mask(src.md_, p->attr);

    for (int64_t idx = 0; idx < nelems; ++idx) {
        float src_ = src.get_elem(idx);
        const int64_t scale_idx = dst.get_scale_idx(idx, scale_mask);

        const float scale = scales[scale_idx];

        float dst_ = saturate(src_ * scale, dst_min, dst_max);

        /* parse round mode and round value*/
        if (dst_dt != mkldnn_f32) {
            dst_ = mxcsr_round(dst_);
            dst_ = saturate(dst_, dst_min, dst_max);
        }

        dst.set_elem(idx, dst_);
    }

    return OK;
}

int compare(const prb_t *p, dnn_mem_t &mem_expected, dnn_mem_t &mem_computed,
        const float *scales, int64_t count, res_t *r){
    int64_t nelems = mem_expected.nelems();
    assert(nelems == mem_computed.nelems());

    r->errors = 0;
    r->total = nelems;

    /* TODO: range support */
    const auto dt = mem_expected.dt();
    const size_t width = mem_expected.sizeof_dt() * 8;

    const float dt_min = dt == mkldnn_u8
        ? 0.f : -(float)(1l << (width - 1));
    const float dt_max = dt == mkldnn_u8
        ? 255.f : (float)((1l << (width - 1)) - 1);

    int64_t inf_p = 0, inf_n = 0, zeros = 0, reg = 0;

    for (int64_t i = 0; i < nelems; ++i) {
        const float expected = mem_expected.get_elem(i);
        const float computed = mem_computed.get_elem(i);
        const float diff = fabsf(computed - expected);

        if (expected == dt_max) inf_p++;
        else if (expected == dt_min) inf_n++;
        else if (expected == 0.0) zeros++;
        else
            reg++;

        if (r->errors < 10 && diff != 0.0) {
            printf("idx: " IFMT " exp: %f com:%f\n", i, expected, computed);
            r->errors++;
        }
    }

    if (r->errors)
        r->state = FAILED;

    if (r->state == UNTESTED)
        r->state = PASSED; /* optimism */

    float max_scale = scales[0];
    for (int64_t i = 1; i < count; ++i) {
        if (scales[i] > max_scale) max_scale = scales[i];
    }

    dt_conf_t c_src = p->conf_in;
    dt_conf_t c_dst = p->conf_out;
    const int c_src_max = c_src->min + c_src->range - 1;
    const int c_dst_max = c_dst->min + c_dst->range - 1;

    bool check_inf_p = (dt != mkldnn_f32 && dt != mkldnn_s32)
        && (c_src_max * max_scale > c_dst_max) ? true : false;
    bool check_inf_n = (dt != mkldnn_f32 && dt != mkldnn_s32)
        && (c_src->min * max_scale < c_dst->min) ? true : false;
    bool check_zeros = (dt != mkldnn_f32)
        && (dt_min != 0 && dt_max != 0) ? true : false;

    bool mistrusted = reg == 0
        || (check_inf_p && inf_p == 0)
        || (check_inf_n && inf_n == 0)
        || (check_zeros && zeros == 0);
    if (mistrusted) r->state = MISTRUSTED;

    return r->state == FAILED ? FAIL : OK;
}

int check_reorder(const prb_t *p, res_t *res) {
/*                                       ___________________
 *                                      |                   |
 *                                      | performance timer |
 *                                      |___________________|
 *                                                |
 *   _______________           ______________     V     ________________
 *  |               | MKL-DNN |              | MKL-DNN |                |
 *  | dt_in fmt_ref |-------->| dt_in fmt_in |-------->| dt_out fmt_out |
 *  |_______________|         |______________|    ^    |________________|
 *           |                                    |            |
 *  benchdnn |<-------------------------------- scales         | MKL-DNN
 *   ________V_______                                   _______V________
 *  |                |                                 |                |
 *  | dt_out fmt_ref |         <= compare =>           | dt_out fmt_ref |
 *  |________________|                                 |________________|
 *
 * Steps:
 * 1. create memory
 * 2. fill scales
 * 3. fill input memory
 * 4. execute mkl-dnn: reorder->q10n->reorder
 * 5. execute benchdnn: q10n
 * 6. compare results
 * 7. performance measurment
 * 8. clean up
 */

    const reorder_conf_t &r = p->reorder;
    const int ndims = (int)r.dims.size();
    const int64_t *dims = &r.dims[0];

    /* Step 1: create memory */
    dnn_mem_t mem_dt_in_fmt_ref(ndims, dims, p->conf_in->dt, nullptr);
    dnn_mem_t mem_dt_in_fmt_in(ndims, dims, p->conf_in->dt, r.tag_in);
    dnn_mem_t mem_dt_out_fmt_out(ndims, dims, p->conf_out->dt, r.tag_out);
    dnn_mem_t mem_dt_out_fmt_ref(ndims, dims, p->conf_out->dt, nullptr);
    dnn_mem_t mem_test_dt_out_fmt_ref(ndims, dims, p->conf_out->dt, nullptr);

    /* Step 2: fill scales */
    int64_t count = 0;
    int mask = 0;
    SAFE(scales_count(&count, &mask, mem_dt_out_fmt_out, p->attr), WARN);
    float *scales = (float *)zmalloc(sizeof(float) * count, 64);
    SAFE(scales != NULL ? OK : FAIL, CRIT);
    SAFE(fill_scales(p, scales, count), WARN);
    /* Step 3: fill input memory */
    SAFE(fill_memory(p, mem_dt_in_fmt_ref, scales, p->attr), WARN);

    /* Step 4: execute mkl-dnn */
    SAFE(mem_dt_in_fmt_in.reorder(mem_dt_in_fmt_ref), WARN);

    auto mkldnn_attr = create_mkldnn_attr(p->attr, count, mask, scales);

    mkldnn_primitive_desc_t check_rpd;
    mkldnn_status_t init_status = mkldnn_reorder_primitive_desc_create(
            &check_rpd, engine, &mem_dt_in_fmt_in.md_, engine,
            &mem_dt_out_fmt_out.md_, mkldnn_attr);
    if (init_status == mkldnn_unimplemented) {
        res->state = UNIMPLEMENTED;
        goto cleanup;
    }
    mkldnn_primitive_desc_destroy(check_rpd);
    SAFE(init_status, WARN);

    SAFE(mem_dt_out_fmt_out.reorder(mem_dt_in_fmt_in, mkldnn_attr), WARN);

    /* Step 5: check correctness */
    if (bench_mode & CORR) {
        /* Step 5a: reorder output from mkldnn to ref format using mkldnn */
        SAFE(mem_dt_out_fmt_ref.reorder(mem_dt_out_fmt_out), WARN);

        /* Step 5b: execute benchdnn reorder */
        SAFE(reorder(p, mem_test_dt_out_fmt_ref, mem_dt_in_fmt_ref, scales), WARN);

        /* Step 5c: compare benchdnn and mkldnn output */
        SAFE(compare(p, mem_test_dt_out_fmt_ref, mem_dt_out_fmt_ref,
                    scales, count, res), WARN);
    }

    /* Step 6: performance measurement */
    if (bench_mode & PERF) {
        mkldnn_primitive_desc_t perf_r_pd;
        DNN_SAFE(mkldnn_reorder_primitive_desc_create(&perf_r_pd, engine,
                    &mem_dt_in_fmt_in.md_, engine, &mem_dt_out_fmt_out.md_,
                    mkldnn_attr), WARN);

        mkldnn_primitive_t perf_r;
        DNN_SAFE(mkldnn_primitive_create(&perf_r, perf_r_pd), WARN);
        DNN_SAFE_V(mkldnn_primitive_desc_destroy(perf_r_pd));

        args_t args;
        args.set(MKLDNN_ARG_FROM, mem_dt_in_fmt_in.m_);
        args.set(MKLDNN_ARG_TO, mem_dt_out_fmt_out.m_);

        auto &t = res->timer;
        t.reset();
        while (true) {
            DNN_SAFE(mkldnn_primitive_execute(perf_r, stream, args.size(), args), WARN);
            t.stamp();
            const bool stop = false
                || (fix_times_per_prb && t.times() >= fix_times_per_prb)
                || (!fix_times_per_prb
                        && t.total_ms() >= max_ms_per_prb
                        && t.times() >= min_times_per_prb);
            if (stop) break;
        }

        DNN_SAFE_V(mkldnn_primitive_destroy(perf_r));
    }

    /* Step 7: clean up */
cleanup:
    mkldnn_primitive_attr_destroy(mkldnn_attr);
    zfree(scales);

    return OK;
}

int doit(const prb_t *p, res_t *r) {
    return check_reorder(p, r);
}

}
