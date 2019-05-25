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
#include <random>

#include "mkldnn.h"

#include "src/common/mkldnn_thread.hpp"

#include "mkldnn_common.hpp"
#include "mkldnn_memory.hpp"
#include "norm.hpp"

#include "rnn/rnn.hpp"
#include "rnn/rnn_aux.hpp"

namespace rnn {

#define CALL_MKLDNN_RNN 1

mkldnn_primitive_attr_t create_mkldnn_rnn_attr(const rnn_prb_t *p) {
    mkldnn_primitive_attr_t mkldnn_attr = NULL;

    DNN_SAFE_V(mkldnn_primitive_attr_create(&mkldnn_attr));
    if (p->scale_policy == PER_OC) {
        DNN_SAFE_V(mkldnn_primitive_attr_set_rnn_weights_qparams(
                mkldnn_attr, p->dic * p->n_gates(), 0x3, p->wei_oc_scales));
    } else if (p->scale_policy == COMMON && p->wei_scale != 1.) {
        DNN_SAFE_V(mkldnn_primitive_attr_set_rnn_weights_qparams(
                mkldnn_attr, 1, 0, &p->wei_scale));
    }

    if (p->data_scale != 1.0 || p->data_shift != 0.0) {
        DNN_SAFE_V(mkldnn_primitive_attr_set_rnn_data_qparams(
                mkldnn_attr, p->data_scale, p->data_shift));
    }

    return mkldnn_attr;
}

int fill_memory(const rnn_prb_t *p, rnn_data_kind_t kind, dnn_mem_t &mem1,
        dnn_mem_t &mem2) {
#ifdef CALL_MKLDNN_RNN
    const size_t nelems = mem1.nelems();
    assert(mem1.nelems() == mem2.nelems());
#else
    const size_t nelems = mem2.nelems();
#endif

    dt_conf_t c = p->cfg[kind];
    float mean = c.f_mean, var = c.f_var, min = c.f_min, max = c.f_max;
    mkldnn::impl::parallel(0, [&](int ithr, int nthr) {
        size_t chunk_size = (nelems + nthr - 1) / nthr;
        size_t idx_start = ithr * chunk_size;
        size_t idx_end = MIN2(idx_start + chunk_size, nelems);
        std::minstd_rand msr;
        msr.seed((unsigned long int)kind);
        std::normal_distribution<float> gen(mean, var);
        msr.discard(idx_start);
        for (size_t idx = idx_start; idx < idx_end; ++idx) {
            auto val = (c.dt == mkldnn_f32) ? gen(msr) : round(gen(msr));
            mem2.set_elem(idx, MAX2(MIN2(val, max), min));
        }
    });

    mem1.reorder(mem2);
    return OK;
}

inline int init_pd(const rnn_prb_t *p, mkldnn_rnn_desc_t rd[2],
        mkldnn_primitive_desc_t rpd[2], res_t *r) {
    const bool is_bwd = p->prop == mkldnn_backward;
    // If we are testing backward, we have to first run forward
    // training first in order to generate a valid workspace.
    auto fwd_prop = is_bwd ? mkldnn_forward_training : mkldnn_forward_inference;
    const bool is_gru_lbr = p->alg == LBR_GRU;
    int the_stride = 1;
    /// @todo we need to add stride support for diff_* tensors too
    mkldnn_memory_desc_t input_d, states_d, weights_input_d, weights_states_d,
            bias_d, dst_last_layer_d, dst_last_iteration_d, diff_input_d,
            diff_states_d, diff_weights_input_d, diff_weights_states_d,
            diff_bias_d, diff_last_layer_d, diff_last_iteration_d;

    // dimensions with ref
    mkldnn_dims_t input_dims = { p->n_iter, p->mb, p->slc };
    // bidirectional = 2, s for lstm = 2, for all other = 1
    mkldnn_dims_t weights_input_dims
            = { p->n_layer, p->n_directions(), p->slc, p->n_gates(), p->dic };
    mkldnn_dims_t weights_states_dims
            = { p->n_layer, p->n_directions(), p->sic, p->n_gates(), p->dic };
    mkldnn_dims_t bias_dims
            = { p->n_layer, p->n_directions(), p->n_gates() + is_gru_lbr, p->dic };
    // mkldnn_tnc
    int64_t lastlay_dlc = (p->direction == mkldnn_bidirectional_concat)
            ? 2 * p->dlc
            : p->dlc;
    mkldnn_dims_t dst_last_layer_dims = { p->n_iter, p->mb, lastlay_dlc };

    DNN_SAFE(mkldnn_memory_desc_init_by_tag(
                     &input_d, 3, input_dims, p->cfg[input].dt, mkldnn_tnc),
            WARN);
    input_d.format_desc.blocking.strides[0] += the_stride;

    mkldnn_dims_t states_dims
            = { p->n_layer, p->n_directions(), p->n_states(), p->mb, p->sic };
    DNN_SAFE(mkldnn_memory_desc_init_by_tag(&states_d, 5, states_dims,
                     p->cfg[states].dt, mkldnn_ldsnc),
            WARN);

    states_d.format_desc.blocking.strides[3] = p->sic + the_stride;
    states_d.format_desc.blocking.strides[2]
            = states_d.format_desc.blocking.strides[3] * states_d.dims[3]
            + the_stride;
    for (int d = 1; d >= 0; --d)
        states_d.format_desc.blocking.strides[d]
                = states_d.format_desc.blocking.strides[d + 1]
                * states_d.dims[d + 1];

    DNN_SAFE(mkldnn_memory_desc_init_by_tag(&weights_input_d, 5, weights_input_dims,
                     p->cfg[weights_input].dt, mkldnn_format_tag_any),
            WARN);

    DNN_SAFE(mkldnn_memory_desc_init_by_tag(&weights_states_d, 5, weights_states_dims,
                     p->cfg[weights_states].dt, mkldnn_format_tag_any),
            WARN);

    DNN_SAFE(mkldnn_memory_desc_init_by_tag(
                     &bias_d, 4, bias_dims, p->cfg[bias].dt, mkldnn_format_tag_any),
            WARN);

    DNN_SAFE(mkldnn_memory_desc_init_by_tag(&dst_last_layer_d, 3, dst_last_layer_dims,
                     p->cfg[dst_last_layer].dt, mkldnn_tnc),
            WARN);
    dst_last_layer_d.format_desc.blocking.strides[0] += the_stride;

    mkldnn_dims_t dst_last_iteration_dims
            = { p->n_layer, p->n_directions(), p->n_states(), p->mb, p->dic };
    DNN_SAFE(mkldnn_memory_desc_init_by_tag(&dst_last_iteration_d, 5,
                     dst_last_iteration_dims, p->cfg[dst_last_iteration].dt,
                     mkldnn_ldsnc),
            WARN);

    dst_last_iteration_d.format_desc.blocking.strides[3]
            = p->sic + the_stride;
    dst_last_iteration_d.format_desc.blocking.strides[2]
            = dst_last_iteration_d.format_desc.blocking.strides[3]
                    * dst_last_iteration_d.dims[3]
            + the_stride;
    for (int d = 1; d >= 0; --d)
        dst_last_iteration_d.format_desc.blocking.strides[d]
                = dst_last_iteration_d.format_desc.blocking.strides[d + 1]
                * dst_last_iteration_d.dims[d + 1];

    mkldnn_alg_kind_t kind = alg2kind(p->alg);
    mkldnn_alg_kind_t f = activation2kind(p->activation);

    mkldnn_rnn_cell_desc_t rcd;
    DNN_SAFE(mkldnn_rnn_cell_desc_init(&rcd, kind, f, 0U, 0, 0), WARN);
    // Initializing the forward pass
    // When inference, we use forward_inference
    // When training, we use forward_training
    {
        mkldnn_status_t init_status = mkldnn_success;
        init_status = mkldnn_rnn_forward_desc_init(&rd[0], fwd_prop, &rcd,
                         p->direction, &input_d, &states_d, &weights_input_d,
                         &weights_states_d, &bias_d, &dst_last_layer_d,
                         &dst_last_iteration_d);
        if (init_status == mkldnn_unimplemented)
            return r->state = UNIMPLEMENTED, OK;
        else
            SAFE(init_status, WARN);
    }

    if (is_bwd) {
        DNN_SAFE(mkldnn_memory_desc_init_by_tag(&diff_input_d, 3, input_dims,
                         p->cfg[dst_diff_input].dt, mkldnn_format_tag_any),
                WARN);
        DNN_SAFE(mkldnn_memory_desc_init_by_tag(&diff_states_d, 5, states_dims,
                         p->cfg[dst_diff_states].dt, mkldnn_format_tag_any),
                WARN);
        DNN_SAFE(mkldnn_memory_desc_init_by_tag(&diff_weights_input_d, 5,
                         weights_input_dims, p->cfg[dst_diff_weights_input].dt,
                         mkldnn_format_tag_any),
                WARN);
        DNN_SAFE(mkldnn_memory_desc_init_by_tag(&diff_weights_states_d, 5,
                         weights_states_dims,
                         p->cfg[dst_diff_weights_states].dt, mkldnn_format_tag_any),
                WARN);
        DNN_SAFE(mkldnn_memory_desc_init_by_tag(&diff_bias_d, 4, bias_dims,
                         p->cfg[dst_diff_bias].dt, mkldnn_format_tag_any),
                WARN);
        DNN_SAFE(mkldnn_memory_desc_init_by_tag(&diff_last_layer_d, 3,
                         dst_last_layer_dims, p->cfg[diff_last_layer].dt,
                         mkldnn_format_tag_any),
                WARN);
        DNN_SAFE(mkldnn_memory_desc_init_by_tag(&diff_last_iteration_d, 5,
                         dst_last_iteration_dims,
                         p->cfg[diff_last_iteration].dt, mkldnn_format_tag_any),
                WARN);
        DNN_SAFE(mkldnn_rnn_backward_desc_init(&rd[1], p->prop, &rcd,
                         p->direction, &input_d, &states_d, &weights_input_d,
                         &weights_states_d, &bias_d, &dst_last_layer_d,
                         &dst_last_iteration_d, &diff_input_d, &diff_states_d,
                         &diff_weights_input_d, &diff_weights_states_d,
                         &diff_bias_d, &diff_last_layer_d,
                         &diff_last_iteration_d),
                WARN);
    }
    auto mkldnn_attr = create_mkldnn_rnn_attr(p);
    mkldnn_status_t init_status = mkldnn_success;
    for (int i = 0; i < 1 + (int)is_bwd; i++) {
        init_status = mkldnn_primitive_desc_create(
                &(rpd[i]), &(rd[i]), mkldnn_attr, engine, NULL);
        if (init_status == mkldnn_unimplemented)
            return r->state = UNIMPLEMENTED, OK;
        else
            SAFE(init_status, WARN);
    }
    mkldnn_primitive_attr_destroy(mkldnn_attr);

    auto q = [=](mkldnn_query_t query, int rpd_idx, int index = 0)
    { return *mkldnn_primitive_desc_query_md(rpd[rpd_idx], query, index); };

    for (int i = 0; i < 1 + (int)is_bwd; i++) {
        rd[i].src_layer_desc = q(mkldnn_query_src_md, i);
        rd[i].src_iter_desc = q(mkldnn_query_src_md, i, 1);
        rd[i].weights_layer_desc = q(mkldnn_query_weights_md, i);
        rd[i].weights_iter_desc = q(mkldnn_query_weights_md, i, 1);
        rd[i].bias_desc = q(mkldnn_query_weights_md, i, 2);
        rd[i].dst_layer_desc = q(mkldnn_query_dst_md, i);
        rd[i].dst_iter_desc = q(mkldnn_query_dst_md, i, 1);
    }
    if (is_bwd) {
        rd[1].diff_src_layer_desc = q(mkldnn_query_diff_src_md, 1);
        rd[1].diff_src_iter_desc = q(mkldnn_query_diff_src_md, 1, 1);
        rd[1].diff_weights_layer_desc = q(mkldnn_query_diff_weights_md, 1);
        rd[1].diff_weights_iter_desc = q(mkldnn_query_diff_weights_md, 1, 1);
        rd[1].diff_bias_desc = q(mkldnn_query_diff_weights_md, 1, 2);
        rd[1].diff_dst_layer_desc = q(mkldnn_query_diff_dst_md, 1);
        rd[1].diff_dst_iter_desc = q(mkldnn_query_diff_dst_md, 1, 1);
    }

    return OK;
}

int doit(const rnn_prb_t *p, res_t *r) {
    res_t res_zero{};
    *r = res_zero;

    const auto fp = mkldnn_f32;

    if (p->alg != VANILLA_LSTM && p->alg != VANILLA_RNN
        && p->alg != VANILLA_GRU && p->alg != LBR_GRU) {
        printf("p->alg: %d\n", (int)p->alg);
        r->state = UNIMPLEMENTED;
        return OK;
    }

    const bool is_bwd = p->prop == mkldnn_backward;

    dnn_mem_t *input_dt = nullptr;
    dnn_mem_t *states_dt = nullptr;
    dnn_mem_t *weights_input_dt = nullptr;
    dnn_mem_t *weights_states_dt = nullptr;
    dnn_mem_t *bias_dt = nullptr;
    dnn_mem_t *dst_last_layer_dt = nullptr;
    dnn_mem_t *dst_last_iteration_dt = nullptr;

    dnn_mem_t *bwd_weights_input_dt = nullptr;
    dnn_mem_t *bwd_weights_states_dt = nullptr;
    dnn_mem_t *dst_diff_input_dt = nullptr;
    dnn_mem_t *dst_diff_states_dt = nullptr;
    dnn_mem_t *dst_diff_weights_input_dt = nullptr;
    dnn_mem_t *dst_diff_weights_states_dt = nullptr;
    dnn_mem_t *dst_diff_bias_dt = nullptr;
    dnn_mem_t *diff_last_layer_dt = nullptr;
    dnn_mem_t *diff_last_iteration_dt = nullptr;

    dnn_mem_t *input_fp = nullptr;
    dnn_mem_t *states_fp = nullptr;
    dnn_mem_t *weights_input_fp = nullptr;
    dnn_mem_t *weights_states_fp = nullptr;
    dnn_mem_t *bias_fp = nullptr;
    dnn_mem_t *dst_last_layer_fp = nullptr;
    dnn_mem_t *dst_last_iteration_fp = nullptr;

    dnn_mem_t *dst_diff_input_fp = nullptr;
    dnn_mem_t *dst_diff_states_fp = nullptr;
    dnn_mem_t *dst_diff_weights_input_fp = nullptr;
    dnn_mem_t *dst_diff_weights_states_fp = nullptr;
    dnn_mem_t *dst_diff_bias_fp = nullptr;
    dnn_mem_t *diff_last_layer_fp = nullptr;
    dnn_mem_t *diff_last_iteration_fp = nullptr;

    dnn_mem_t *workspace_dt = nullptr;

    mkldnn_rnn_desc_t rd[2];
    mkldnn_primitive_desc_t rpd[2] = {nullptr};
    mkldnn_primitive_t c{};
    SAFE(init_pd(p, rd, rpd, r), WARN);
    if (r->state == SKIPPED || r->state == UNIMPLEMENTED)
        return OK;

    auto &input_dt_d = rd[0].src_layer_desc;
    auto &states_dt_d = rd[0].src_iter_desc;
    auto &weights_input_dt_d = rd[0].weights_layer_desc;
    auto &weights_states_dt_d = rd[0].weights_iter_desc;
    auto &bias_dt_d = rd[0].bias_desc;
    auto &dst_last_layer_dt_d = rd[0].dst_layer_desc;
    auto &dst_last_iteration_dt_d = rd[0].dst_iter_desc;

    auto &bwd_weights_input_dt_d = rd[1].weights_layer_desc;
    auto &bwd_weights_states_dt_d = rd[1].weights_iter_desc;
    auto &diff_src_layer_dt_d = rd[1].diff_src_layer_desc;
    auto &diff_src_iter_dt_d = rd[1].diff_src_iter_desc;
    auto &diff_weights_layer_dt_d = rd[1].diff_weights_layer_desc;
    auto &diff_weights_iter_dt_d = rd[1].diff_weights_iter_desc;
    auto &diff_bias_dt_d = rd[1].diff_bias_desc;
    auto &diff_dst_layer_dt_d = rd[1].diff_dst_layer_desc;
    auto &diff_dst_iter_dt_d = rd[1].diff_dst_iter_desc;

    input_dt = new dnn_mem_t(input_dt_d, p->cfg[input].dt);
    states_dt = new dnn_mem_t(states_dt_d, p->cfg[states].dt);
    weights_input_dt
            = new dnn_mem_t(weights_input_dt_d, p->cfg[weights_input].dt);
    weights_states_dt
            = new dnn_mem_t(weights_states_dt_d, p->cfg[weights_states].dt);
    bias_dt = new dnn_mem_t(bias_dt_d, p->cfg[bias].dt);
    dst_last_layer_dt
            = new dnn_mem_t(dst_last_layer_dt_d, p->cfg[dst_last_layer].dt);
    dst_last_iteration_dt = new dnn_mem_t(
            dst_last_iteration_dt_d, p->cfg[dst_last_iteration].dt);

    if (is_bwd) {
        bwd_weights_input_dt = new dnn_mem_t(bwd_weights_input_dt_d, fp);
        bwd_weights_states_dt = new dnn_mem_t(bwd_weights_states_dt_d, fp);
        dst_diff_input_dt = new dnn_mem_t(diff_src_layer_dt_d, fp);
        dst_diff_states_dt = new dnn_mem_t(diff_src_iter_dt_d, fp);
        dst_diff_weights_input_dt = new dnn_mem_t(diff_weights_layer_dt_d, fp);
        dst_diff_weights_states_dt = new dnn_mem_t(diff_weights_iter_dt_d, fp);
        dst_diff_bias_dt = new dnn_mem_t(diff_bias_dt_d, fp);
        diff_last_layer_dt = new dnn_mem_t(diff_dst_layer_dt_d, fp);
        diff_last_iteration_dt = new dnn_mem_t(diff_dst_iter_dt_d, fp);
    }

    input_fp = new dnn_mem_t(input_dt_d, fp, mkldnn_tnc);
    states_fp = new dnn_mem_t(states_dt_d, fp, mkldnn_ldsnc);
    weights_input_fp = new dnn_mem_t(weights_input_dt_d, fp, mkldnn_ldigo);
    weights_states_fp = new dnn_mem_t(weights_states_dt_d, fp, mkldnn_ldigo);
    bias_fp = new dnn_mem_t(bias_dt_d, fp, mkldnn_ldgo);
    dst_last_layer_fp = new dnn_mem_t(dst_last_layer_dt_d, fp, mkldnn_tnc);
    dst_last_iteration_fp
            = new dnn_mem_t(dst_last_iteration_dt_d, fp, mkldnn_ldsnc);

    if (is_bwd) {
        dst_diff_input_fp = new dnn_mem_t(diff_src_layer_dt_d, fp, mkldnn_tnc);
        dst_diff_states_fp
                = new dnn_mem_t(diff_src_iter_dt_d, fp, mkldnn_ldsnc);
        dst_diff_weights_input_fp
                = new dnn_mem_t(diff_weights_layer_dt_d, fp, mkldnn_ldigo);
        dst_diff_weights_states_fp
                = new dnn_mem_t(diff_weights_iter_dt_d, fp, mkldnn_ldigo);
        dst_diff_bias_fp = new dnn_mem_t(diff_bias_dt_d, fp, mkldnn_ldgo);
        diff_last_layer_fp = new dnn_mem_t(diff_dst_layer_dt_d, fp, mkldnn_tnc);
        diff_last_iteration_fp
                = new dnn_mem_t(diff_dst_iter_dt_d, fp, mkldnn_ldsnc);

        const auto ws_md = mkldnn_primitive_desc_query_md(
                rpd[0], mkldnn_query_workspace_md, 0);
        SAFE(ws_md != NULL ? OK : FAIL, WARN);
        workspace_dt = new dnn_mem_t(*ws_md);
    }

    SAFE(fill_memory(p, input, *input_dt, *input_fp), WARN);
    SAFE(fill_memory(p, states, *states_dt, *states_fp), WARN);
    SAFE(fill_memory(p, weights_input, *weights_input_dt, *weights_input_fp),
            WARN);
    SAFE(fill_memory(p, weights_states, *weights_states_dt, *weights_states_fp),
            WARN);
    SAFE(fill_memory(p, bias, *bias_dt, *bias_fp), WARN);
    SAFE(fill_memory(p, dst_last_layer, *dst_last_layer_dt, *dst_last_layer_fp),
            WARN);
    SAFE(fill_memory(p, dst_last_iteration, *dst_last_iteration_dt,
                 *dst_last_iteration_fp),
            WARN);

    if (is_bwd) {
        SAFE(bwd_weights_states_dt->reorder(*weights_states_dt), WARN);
        SAFE(bwd_weights_input_dt->reorder(*weights_input_dt), WARN);
        SAFE(fill_memory(
                     p, dst_diff_input, *dst_diff_input_dt, *dst_diff_input_fp),
                WARN);
        SAFE(fill_memory(p, dst_diff_states, *dst_diff_states_dt,
                     *dst_diff_states_fp),
                WARN);
        SAFE(fill_memory(p, dst_diff_weights_input, *dst_diff_weights_input_dt,
                     *dst_diff_weights_input_fp),
                WARN);
        SAFE(fill_memory(p, dst_diff_weights_states,
                     *dst_diff_weights_states_dt, *dst_diff_weights_states_fp),
                WARN);
        SAFE(fill_memory(
                     p, dst_diff_bias, *dst_diff_bias_dt, *dst_diff_bias_fp),
                WARN);
        SAFE(fill_memory(p, diff_last_layer, *diff_last_layer_dt,
                     *diff_last_layer_fp),
                WARN);
        SAFE(fill_memory(p, diff_last_iteration, *diff_last_iteration_dt,
                     *diff_last_iteration_fp),
                WARN);
    }

    args_t args;

    // Running the forward pass
    {
        DNN_SAFE(mkldnn_primitive_create(&c, rpd[0]), WARN);
        DNN_SAFE(mkldnn_primitive_desc_destroy(rpd[0]), CRIT);

        args.set(MKLDNN_ARG_SRC_LAYER, input_dt->m_);
        args.set(MKLDNN_ARG_SRC_ITER, states_dt->m_);
        args.set(MKLDNN_ARG_WEIGHTS_LAYER, weights_input_dt->m_);
        args.set(MKLDNN_ARG_WEIGHTS_ITER, weights_states_dt->m_);
        args.set(MKLDNN_ARG_BIAS, bias_dt->m_);

        args.set(MKLDNN_ARG_DST_LAYER, dst_last_layer_dt->m_);
        args.set(MKLDNN_ARG_DST_ITER, dst_last_iteration_dt->m_);
        if (workspace_dt)
            args.set(MKLDNN_ARG_WORKSPACE, workspace_dt->m_);

#ifdef CALL_MKLDNN_RNN
        DNN_SAFE(mkldnn_primitive_execute(c, stream, args.size(), args), WARN);
#endif
        if ((p->prop == mkldnn_forward) && (bench_mode & CORR)) {
            compute_ref_fwd(p, *input_fp, *states_fp, *weights_input_fp,
                    *weights_states_fp, *bias_fp, *dst_last_layer_fp,
                    *dst_last_iteration_fp, p->direction);
            dnn_mem_t dst_last_layer(*dst_last_layer_dt, fp, mkldnn_tnc);
            dnn_mem_t dst_last_iteration(
                    *dst_last_iteration_dt, fp, mkldnn_ldsnc);
            SAFE(compare_dst_last_layer(
                         p, dst_last_layer, *dst_last_layer_fp, r, true),
                    WARN);
            SAFE(compare_dst_last_iteration(p, dst_last_iteration,
                         *dst_last_iteration_fp, r, true),
                    WARN);
        }
    }

    if (is_bwd) {
        args.clear();
        DNN_SAFE(mkldnn_primitive_destroy(c), CRIT);

        DNN_SAFE(mkldnn_primitive_create(&c, rpd[1]), WARN);
        DNN_SAFE(mkldnn_primitive_desc_destroy(rpd[1]), CRIT);

        args.set(MKLDNN_ARG_SRC_LAYER, input_dt->m_);
        args.set(MKLDNN_ARG_SRC_ITER, states_dt->m_);
        args.set(MKLDNN_ARG_WEIGHTS_LAYER, bwd_weights_input_dt->m_);
        args.set(MKLDNN_ARG_WEIGHTS_ITER, bwd_weights_states_dt->m_);
        args.set(MKLDNN_ARG_BIAS, bias_dt->m_);
        args.set(MKLDNN_ARG_DST_LAYER, dst_last_layer_dt->m_);
        args.set(MKLDNN_ARG_DST_ITER, dst_last_iteration_dt->m_);
        args.set(MKLDNN_ARG_DIFF_DST_LAYER, diff_last_layer_dt->m_);
        args.set(MKLDNN_ARG_DIFF_DST_ITER, diff_last_iteration_dt->m_);
        args.set(MKLDNN_ARG_WORKSPACE, workspace_dt->m_);

        args.set(MKLDNN_ARG_DIFF_SRC_LAYER, dst_diff_input_dt->m_);
        args.set(MKLDNN_ARG_DIFF_SRC_ITER, dst_diff_states_dt->m_);
        args.set(MKLDNN_ARG_DIFF_WEIGHTS_LAYER, dst_diff_weights_input_dt->m_);
        args.set(MKLDNN_ARG_DIFF_WEIGHTS_ITER, dst_diff_weights_states_dt->m_);
        args.set(MKLDNN_ARG_DIFF_BIAS, dst_diff_bias_dt->m_);

#ifdef CALL_MKLDNN_RNN
        DNN_SAFE(mkldnn_primitive_execute(c, stream, args.size(), args), WARN);
#endif

        if (bench_mode & CORR) {
            compute_ref_bwd(p, *input_fp, *states_fp, *diff_last_layer_fp,
                    *diff_last_iteration_fp, *weights_input_fp,
                    *weights_states_fp, *bias_fp, *dst_last_layer_fp,
                    *dst_last_iteration_fp, *dst_diff_input_fp,
                    *dst_diff_states_fp, *dst_diff_weights_input_fp,
                    *dst_diff_weights_states_fp, *dst_diff_bias_fp,
                    p->direction);

            dnn_mem_t dst_last_layer(*dst_last_layer_dt, fp, mkldnn_tnc);
            dnn_mem_t dst_last_iteration(
                    *dst_last_iteration_dt, fp, mkldnn_ldsnc);
            SAFE(compare_dst_last_layer(
                         p, dst_last_layer, *dst_last_layer_fp, r, true),
                    WARN);
            SAFE(compare_dst_last_iteration(p, dst_last_iteration,
                         *dst_last_iteration_fp, r, true),
                    WARN);

            dnn_mem_t diff_input(*dst_diff_input_dt, fp, mkldnn_tnc);
            dnn_mem_t diff_states(*dst_diff_states_dt, fp, mkldnn_ldsnc);
            SAFE(compare_input(p, diff_input, *dst_diff_input_fp, r, true),
                    WARN);
            SAFE(compare_states(p, diff_states, *dst_diff_states_fp, r, true),
                    WARN);

            dnn_mem_t diff_weights_input(
                    *dst_diff_weights_input_dt, fp, mkldnn_ldigo);
            dnn_mem_t diff_weights_states(
                    *dst_diff_weights_states_dt, fp, mkldnn_ldigo);
            SAFE(compare_weights_input(p, diff_weights_input,
                         *dst_diff_weights_input_fp, r, true),
                    WARN);
            SAFE(compare_weights_states(p, diff_weights_states,
                         *dst_diff_weights_states_fp, r, true),
                    WARN);

            dnn_mem_t diff_bias(*dst_diff_bias_dt, fp, mkldnn_ldgo);
            SAFE(compare_bias(p, diff_bias, *dst_diff_bias_fp, r, true), WARN);
        }
    }

    if (bench_mode & PERF) {
        auto &t = r->timer;
        t.reset();
        while (true) {
#ifdef CALL_MKLDNN_RNN
            DNN_SAFE(mkldnn_primitive_execute(c, stream, args.size(), args), WARN);
#endif
            t.stamp();
            const bool stop = false
                    || (fix_times_per_prb && t.times() >= fix_times_per_prb)
                    || (!fix_times_per_prb && t.total_ms() >= max_ms_per_prb
                               && t.times() >= min_times_per_prb);
            if (stop)
                break;
        }
    }

    // cleanup
    delete input_fp;
    delete states_fp;
    delete weights_input_fp;
    delete weights_states_fp;
    delete bias_fp;
    delete dst_last_layer_fp;
    delete dst_last_iteration_fp;

    if (is_bwd) {
        delete bwd_weights_input_dt;
        delete bwd_weights_states_dt;
        delete dst_diff_input_fp;
        delete dst_diff_states_fp;
        delete dst_diff_weights_input_fp;
        delete dst_diff_weights_states_fp;
        delete dst_diff_bias_fp;
        delete diff_last_layer_fp;
        delete diff_last_iteration_fp;
    }

    delete input_dt;
    delete states_dt;
    delete weights_input_dt;
    delete weights_states_dt;
    delete bias_dt;
    delete dst_last_layer_dt;
    delete dst_last_iteration_dt;

    if (is_bwd) {
        delete dst_diff_input_dt;
        delete dst_diff_states_dt;
        delete dst_diff_weights_input_dt;
        delete dst_diff_weights_states_dt;
        delete dst_diff_bias_dt;
        delete diff_last_layer_dt;
        delete diff_last_iteration_dt;
    }

    delete workspace_dt;

    DNN_SAFE(mkldnn_primitive_destroy(c), CRIT);

    return OK;
}
} // namespace rnn
