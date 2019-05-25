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

#include <assert.h>

#include <cstring>
#include <iostream>
#include <math.h>
#include <numeric>
#include <string>

#include "mkldnn.hpp"

// MSVC doesn't support collapse clause in omp parallel
#if defined(_MSC_VER) && !defined(__clang__) && !defined(__INTEL_COMPILER)
#define collapse(x)
#endif

using namespace mkldnn;

using dim_t = mkldnn::memory::dim;

const dim_t batch = 64;
const dim_t src_seq_length_max = 25;
const dim_t tgt_seq_length_max = 27;

const dim_t feature_size = 1024;

const dim_t enc_bidir_n_layers = 1;
const dim_t enc_unidir_n_layers = 7;
const dim_t dec_n_layers = 8;

const int lstm_n_gates = 4;
const int lstm_n_states = 2;
std::vector<int32_t> weighted_src_layer(batch *feature_size, 1);
std::vector<float> alignment_model(
        src_seq_length_max *batch *feature_size, 1.0f);
std::vector<float> alignments(src_seq_length_max *batch, 1.0f);
std::vector<float> exp_sums(batch, 1.0f);

const float onef = 1.0, zerof = 0.0;
const dim_t onei = 1;

void compute_weighted_annotations(float *weighted_annotations,
        dim_t src_seq_length_max, dim_t batch, dim_t feature_size,
        float *weights_annot, float *annotations) {
    // annotations(aka enc_dst_layer) is (t, n, 2c)
    // weights_annot is (2c, c)

    dim_t num_weighted_annotations = src_seq_length_max * batch;
    // annotation[i] = GEMM(weights_annot, enc_dst_layer[i]);
    mkldnn_sgemm("N", "N", &feature_size, &num_weighted_annotations,
            &feature_size, &onef, weights_annot, &feature_size, annotations,
            &feature_size, &zerof, weighted_annotations, &feature_size);
}

void compute_sum_of_rows(
        int8_t *a, dim_t rows, dim_t cols, int32_t *a_reduced) {
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (dim_t i = 0; i < cols; i++) {
        a_reduced[i] = 0;
        for (dim_t j = 0; j < rows; j++) {
            a_reduced[i] += (int32_t)a[i * rows + j];
        }
    }
}

void compute_attention(float *context_vectors, dim_t src_seq_length_max,
        dim_t batch, dim_t feature_size, int8_t *weights_src_layer,
        float weights_src_layer_scale, int32_t *compensation,
        uint8_t *dec_src_layer, float dec_src_layer_scale,
        float dec_src_layer_shift, uint8_t *annotations,
        float *weighted_annotations, float *weights_alignments) {
    // dst_iter : (n, c) matrix
    // src_layer: (n, c) matrix
    // weighted_annotations (t, n, c)

    // weights_yi is (c, c)
    // weights_ai is (c, 1)
    // tmp[i] is (n, c)
    // a[i] is (n, 1)
    // p is (n, 1)

    // first we precompute the weighted_dec_src_layer
    int8_t ao = 0;
    int8_t bo = 0;
    int32_t co = 0;
    mkldnn_gemm_s8u8s32("N", "N", "F", &feature_size, &batch, &feature_size,
            &onef, weights_src_layer, &feature_size, &ao, dec_src_layer,
            &feature_size, &bo, &zerof, weighted_src_layer.data(),
            &feature_size, &co);

    // then we compute the alignment model
    float *alignment_model_ptr = alignment_model.data();
#ifdef _OPENMP
#pragma omp parallel for collapse(2)
#endif
    for (dim_t i = 0; i < src_seq_length_max; i++) {
        for (dim_t j = 0; j < batch; j++) {
            for (dim_t k = 0; k < feature_size; k++) {
                size_t tnc_offset
                        = i * batch * feature_size + j * feature_size + k;
                alignment_model_ptr[tnc_offset] = tanhf(
                        (float)(weighted_src_layer.data()[j * feature_size + k]
                                - dec_src_layer_shift * compensation[k])
                                / (dec_src_layer_scale
                                          * weights_src_layer_scale)
                        + weighted_annotations[tnc_offset]);
            }
        }
    }

    // gemv with alignments weights. the resulting alignments are in alignments
    dim_t num_weighted_annotations = src_seq_length_max * batch;
    mkldnn_sgemm("N", "N", &onei, &num_weighted_annotations, &feature_size,
            &onef, weights_alignments, &onei, alignment_model_ptr,
            &feature_size, &zerof, alignments.data(), &onei);

// softmax on alignments. the resulting context weights are in alignments
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (dim_t i = 0; i < batch; i++)
        exp_sums[i] = 0.0f;
#ifdef _OPENMP
#pragma omp parallel for collapse(2)
#endif
    for (dim_t i = 0; i < src_seq_length_max; i++) {
        for (dim_t j = 0; j < batch; j++) {
            alignments[i * batch + j] = expf(alignments[i * batch + j]);
            exp_sums[j] += alignments[i * batch + j];
        }
    }

#ifdef _OPENMP
#pragma omp parallel for collapse(2)
#endif
    for (dim_t i = 0; i < src_seq_length_max; i++)
        for (dim_t j = 0; j < batch; j++)
            alignments[i * batch + j] /= exp_sums[j];

// then we compute the context vectors
#ifdef _OPENMP
#pragma omp parallel for collapse(2)
#endif
    for (dim_t i = 0; i < batch; i++)
        for (dim_t j = 0; j < feature_size; j++)
            context_vectors[i * (feature_size + feature_size) + feature_size
                    + j]
                    = 0.0f;

#ifdef _OPENMP
#pragma omp parallel for collapse(3)
#endif
    for (dim_t i = 0; i < batch; i++)
        for (dim_t k = 0; k < src_seq_length_max; k++)
            for (dim_t j = 0; j < feature_size; j++)
                context_vectors[i * (feature_size + feature_size) + feature_size
                        + j]
                        += alignments[k * batch + i]
                        * (((float)annotations[j
                                    + feature_size * (i + batch * k)]
                                   - dec_src_layer_shift)
                                  / dec_src_layer_scale);
}

void copy_context(float *src_iter, dim_t n_layers, dim_t n_states, dim_t batch,
        dim_t feature_size) {
// we copy the context from the first layer to all other layers
#ifdef _OPENMP
#pragma omp parallel for collapse(3)
#endif
    for (dim_t k = 1; k < n_layers; k++)
        for (dim_t j = 0; j < batch; j++)
            for (dim_t i = 0; i < feature_size; i++)
                src_iter[(k * n_states * batch + j)
                                * (feature_size + feature_size)
                        + i]
                        = src_iter[j * (feature_size + feature_size) + i];
}

void simple_net() {
    auto cpu_engine = engine(engine::cpu, 0);
    stream s(cpu_engine);

    /*
      GNMT low precicion example.
      Note, we do not implement connection yet.
      For the encoder we use:
      - one primitive for the bidirectional layer of the encoder
      - one primitive for all remaining unidirectional layers in the encoder
      For the decoder we use:
      - one primitive for the first iteration
      - one primitive for all subsequent iterations in the decoder. Note that
        in this example, this primitive computes the states in place.
      - the attention mechanism is implemented separately as there is no support
        for the context vectors in MKL-DNN yet
     */

    std::vector<primitive> encoder_net, decoder_net;
    std::vector<std::unordered_map<int, memory>> encoder_net_args,
            decoder_net_args;

    std::vector<float> net_src(batch * src_seq_length_max * feature_size, 0.1f);
    std::vector<float> net_dst(batch * tgt_seq_length_max * feature_size, 0.1f);

    /* Quantization factors for fp32 data */

    const float data_shift = 64.;
    const float data_scale = 63.;
    const int weights_scale_mask = 3; // 11 for last two dimensions of ldigo
    std::vector<float> weights_scales(lstm_n_gates * feature_size);
    /* assign halves of vector with arbitrary values */
    const dim_t scales_half = lstm_n_gates * feature_size / 2;
    std::fill(
            weights_scales.begin(), weights_scales.begin() + scales_half, 30.f);
    std::fill(weights_scales.begin() + scales_half + 1, weights_scales.end(),
            65.5f);

    /* Encoder */

    memory::dims enc_bidir_src_layer_tz
            = { src_seq_length_max, batch, feature_size };
    memory::dims enc_bidir_weights_layer_tz = { enc_bidir_n_layers, 2,
        feature_size, lstm_n_gates, feature_size };
    memory::dims enc_bidir_weights_iter_tz = { enc_bidir_n_layers, 2,
        feature_size, lstm_n_gates, feature_size };
    memory::dims enc_bidir_bias_tz
            = { enc_bidir_n_layers, 2, lstm_n_gates, feature_size };
    memory::dims enc_bidir_dst_layer_tz
            = { src_seq_length_max, batch, 2 * feature_size };

    /* GNMT encoder: 1 bidirectional layer and 7 unidirectional layers */

    std::vector<float> user_enc_bidir_wei_layer(
            enc_bidir_n_layers * 2 * feature_size * lstm_n_gates * feature_size,
            0.3f);
    std::vector<float> user_enc_bidir_wei_iter(
            enc_bidir_n_layers * 2 * feature_size * lstm_n_gates * feature_size,
            0.2f);
    std::vector<float> user_enc_bidir_bias(
            enc_bidir_n_layers * 2 * lstm_n_gates * feature_size, 1.0f);

    /* Create the memory for user data */
    auto user_enc_bidir_src_layer_md = memory::desc({ enc_bidir_src_layer_tz },
            memory::data_type::f32, memory::format_tag::tnc);

    auto user_enc_bidir_wei_layer_md
            = memory::desc({ enc_bidir_weights_layer_tz },
                    memory::data_type::f32, memory::format_tag::ldigo);

    auto user_enc_bidir_wei_iter_md
            = memory::desc({ enc_bidir_weights_iter_tz },
                    memory::data_type::f32, memory::format_tag::ldigo);

    auto user_enc_bidir_bias_md = memory::desc({ enc_bidir_bias_tz },
            memory::data_type::f32, memory::format_tag::ldgo);

    auto user_enc_bidir_src_layer_memory
            = memory(user_enc_bidir_src_layer_md, cpu_engine, net_src.data());
    auto user_enc_bidir_wei_layer_memory = memory(user_enc_bidir_wei_layer_md,
            cpu_engine, user_enc_bidir_wei_layer.data());
    auto user_enc_bidir_wei_iter_memory = memory(user_enc_bidir_wei_iter_md,
            cpu_engine, user_enc_bidir_wei_iter.data());
    auto user_enc_bidir_bias_memory = memory(
            user_enc_bidir_bias_md, cpu_engine, user_enc_bidir_bias.data());

    /* Create memory descriptors for RNN data w/o specified layout */
    auto enc_bidir_src_layer_md = memory::desc({ enc_bidir_src_layer_tz },
            memory::data_type::u8, memory::format_tag::any);

    auto enc_bidir_wei_layer_md = memory::desc({ enc_bidir_weights_layer_tz },
            memory::data_type::s8, memory::format_tag::any);

    auto enc_bidir_wei_iter_md = memory::desc({ enc_bidir_weights_iter_tz },
            memory::data_type::s8, memory::format_tag::any);

    auto enc_bidir_dst_layer_md = memory::desc({ enc_bidir_dst_layer_tz },
            memory::data_type::u8, memory::format_tag::any);

    /* Create bidirectional RNN */
    rnn_cell::desc bi_cell(algorithm::vanilla_lstm);

    /* Check if int8 RNN is supported */
    try {
        rnn_forward::desc bi_layer_desc(prop_kind::forward_inference, bi_cell,
                rnn_direction::bidirectional_concat, enc_bidir_src_layer_md,
                memory::desc(), enc_bidir_wei_layer_md, enc_bidir_wei_iter_md,
                user_enc_bidir_bias_md, enc_bidir_dst_layer_md, memory::desc());
    } catch (error &e) {
        if (e.status == mkldnn_unimplemented) {
            std::cerr
                    << "Dependency on Intel(R) MKL version 2019u2 or newer is "
                       "required for int8 RNN"
                    << std::endl;
        }
        throw;
    }

    rnn_forward::desc bi_layer_desc(prop_kind::forward_inference, bi_cell,
            rnn_direction::bidirectional_concat, enc_bidir_src_layer_md,
            memory::desc(), enc_bidir_wei_layer_md, enc_bidir_wei_iter_md,
            user_enc_bidir_bias_md, enc_bidir_dst_layer_md, memory::desc());

    /* Define RNN attributes that store quantization parameters */
    primitive_attr attr;
    attr.set_rnn_data_qparams(data_scale, data_shift);
    attr.set_rnn_weights_qparams(weights_scale_mask, weights_scales);

    auto enc_bidir_prim_desc
            = rnn_forward::primitive_desc(bi_layer_desc, attr, cpu_engine);

    /* Create memory for input data and use reorders to quantize values to int8
     * NOTE: same attributes are used when creating RNN primitive and reorders
     */
    auto enc_bidir_src_layer_memory
            = memory(enc_bidir_prim_desc.src_layer_desc(), cpu_engine);
    auto enc_bidir_src_layer_reorder_pd = reorder::primitive_desc(
            user_enc_bidir_src_layer_memory, enc_bidir_src_layer_memory, attr);
    encoder_net.push_back(reorder(enc_bidir_src_layer_reorder_pd));
    encoder_net_args.push_back(
            { { MKLDNN_ARG_FROM, user_enc_bidir_src_layer_memory },
                    { MKLDNN_ARG_TO, enc_bidir_src_layer_memory } });

    auto enc_bidir_wei_layer_memory
            = memory(enc_bidir_prim_desc.weights_layer_desc(), cpu_engine);
    auto enc_bidir_wei_layer_reorder_pd = reorder::primitive_desc(
            user_enc_bidir_wei_layer_memory, enc_bidir_wei_layer_memory, attr);
    reorder(enc_bidir_wei_layer_reorder_pd)
            .execute(s, user_enc_bidir_wei_layer_memory,
                    enc_bidir_wei_layer_memory);

    auto enc_bidir_wei_iter_memory
            = memory(enc_bidir_prim_desc.weights_iter_desc(), cpu_engine);
    auto enc_bidir_wei_iter_reorder_pd = reorder::primitive_desc(
            user_enc_bidir_wei_iter_memory, enc_bidir_wei_iter_memory, attr);
    reorder(enc_bidir_wei_iter_reorder_pd)
            .execute(s, user_enc_bidir_wei_iter_memory,
                    enc_bidir_wei_iter_memory);

    auto enc_bidir_dst_layer_memory
            = memory(enc_bidir_prim_desc.dst_layer_desc(), cpu_engine);

    encoder_net.push_back(rnn_forward(enc_bidir_prim_desc));
    encoder_net_args.push_back(
            { { MKLDNN_ARG_SRC_LAYER, enc_bidir_src_layer_memory },
                    { MKLDNN_ARG_WEIGHTS_LAYER, enc_bidir_wei_layer_memory },
                    { MKLDNN_ARG_WEIGHTS_ITER, enc_bidir_wei_iter_memory },
                    { MKLDNN_ARG_BIAS, user_enc_bidir_bias_memory },
                    { MKLDNN_ARG_DST_LAYER, enc_bidir_dst_layer_memory } });

    /* GNMT encoder: unidirectional layers */
    // First unidirectinal layer scales 2 * feature_size output of bidirectional
    // layer to feature_size output
    std::vector<float> user_enc_uni_first_wei_layer(
            1 * 1 * 2 * feature_size * lstm_n_gates * feature_size, 0.3f);
    std::vector<float> user_enc_uni_first_wei_iter(
            1 * 1 * feature_size * lstm_n_gates * feature_size, 0.2f);
    std::vector<float> user_enc_uni_first_bias(
            1 * 1 * lstm_n_gates * feature_size, 1.0f);

    memory::dims user_enc_uni_first_wei_layer_dims
            = { 1, 1, 2 * feature_size, lstm_n_gates, feature_size };
    memory::dims user_enc_uni_first_wei_iter_dims
            = { 1, 1, feature_size, lstm_n_gates, feature_size };
    memory::dims user_enc_uni_first_bias_dims
            = { 1, 1, lstm_n_gates, feature_size };
    memory::dims enc_uni_first_dst_layer_dims
            = { src_seq_length_max, batch, feature_size };

    auto user_enc_uni_first_wei_layer_md
            = memory::desc({ user_enc_uni_first_wei_layer_dims },
                    memory::data_type::f32, memory::format_tag::ldigo);
    auto user_enc_uni_first_wei_iter_md
            = memory::desc({ user_enc_uni_first_wei_iter_dims },
                    memory::data_type::f32, memory::format_tag::ldigo);
    auto user_enc_uni_first_bias_md
            = memory::desc({ user_enc_uni_first_bias_dims },
                    memory::data_type::f32, memory::format_tag::ldgo);
    auto user_enc_uni_first_wei_layer_memory
            = memory(user_enc_uni_first_wei_layer_md, cpu_engine,
                    user_enc_uni_first_wei_layer.data());
    auto user_enc_uni_first_wei_iter_memory
            = memory(user_enc_uni_first_wei_iter_md, cpu_engine,
                    user_enc_uni_first_wei_iter.data());
    auto user_enc_uni_first_bias_memory = memory(user_enc_uni_first_bias_md,
            cpu_engine, user_enc_uni_first_bias.data());

    auto enc_uni_first_wei_layer_md
            = memory::desc({ user_enc_uni_first_wei_layer_dims },
                    memory::data_type::s8, memory::format_tag::any);
    auto enc_uni_first_wei_iter_md
            = memory::desc({ user_enc_uni_first_wei_iter_dims },
                    memory::data_type::s8, memory::format_tag::any);
    auto enc_uni_first_dst_layer_md
            = memory::desc({ enc_uni_first_dst_layer_dims },
                    memory::data_type::u8, memory::format_tag::any);

    rnn_cell::desc enc_uni_first_cell(algorithm::vanilla_lstm);
    rnn_forward::desc enc_uni_first_layer_desc(prop_kind::forward_inference,
            enc_uni_first_cell, rnn_direction::unidirectional_left2right,
            enc_bidir_dst_layer_md, memory::desc(), enc_uni_first_wei_layer_md,
            enc_uni_first_wei_iter_md, user_enc_uni_first_bias_md,
            enc_uni_first_dst_layer_md, memory::desc());

    auto enc_uni_first_prim_desc = rnn_forward::primitive_desc(
            enc_uni_first_layer_desc, attr, cpu_engine);

    auto enc_uni_first_wei_layer_memory
            = memory(enc_uni_first_prim_desc.weights_layer_desc(), cpu_engine);
    reorder(user_enc_uni_first_wei_layer_memory, enc_uni_first_wei_layer_memory)
            .execute(s, user_enc_uni_first_wei_layer_memory,
                    enc_uni_first_wei_layer_memory);

    auto enc_uni_first_wei_iter_memory
            = memory(enc_uni_first_prim_desc.weights_iter_desc(), cpu_engine);
    reorder(user_enc_uni_first_wei_iter_memory, enc_uni_first_wei_iter_memory)
            .execute(s, user_enc_uni_first_wei_iter_memory,
                    enc_uni_first_wei_iter_memory);

    auto enc_uni_first_dst_layer_memory
            = memory(enc_uni_first_prim_desc.dst_layer_desc(), cpu_engine);

    encoder_net.push_back(rnn_forward(enc_uni_first_prim_desc));
    encoder_net_args.push_back({ { MKLDNN_ARG_SRC_LAYER,
                                         enc_bidir_dst_layer_memory },
            { MKLDNN_ARG_WEIGHTS_LAYER, enc_uni_first_wei_layer_memory },
            { MKLDNN_ARG_WEIGHTS_ITER, enc_uni_first_wei_iter_memory },
            { MKLDNN_ARG_BIAS, user_enc_uni_first_bias_memory },
            { MKLDNN_ARG_DST_LAYER, enc_uni_first_dst_layer_memory } });

    /* Remainging unidirectional layers */
    std::vector<float> user_enc_uni_wei_layer((enc_unidir_n_layers - 1) * 1
                    * feature_size * lstm_n_gates * feature_size,
            0.3f);
    std::vector<float> user_enc_uni_wei_iter((enc_unidir_n_layers - 1) * 1
                    * feature_size * lstm_n_gates * feature_size,
            0.2f);
    std::vector<float> user_enc_uni_bias(
            (enc_unidir_n_layers - 1) * 1 * lstm_n_gates * feature_size, 1.0f);

    memory::dims user_enc_uni_wei_layer_dims = { (enc_unidir_n_layers - 1), 1,
        feature_size, lstm_n_gates, feature_size };
    memory::dims user_enc_uni_wei_iter_dims = { (enc_unidir_n_layers - 1), 1,
        feature_size, lstm_n_gates, feature_size };
    memory::dims user_enc_uni_bias_dims
            = { (enc_unidir_n_layers - 1), 1, lstm_n_gates, feature_size };
    memory::dims enc_dst_layer_dims
            = { src_seq_length_max, batch, feature_size };

    auto user_enc_uni_wei_layer_md
            = memory::desc({ user_enc_uni_wei_layer_dims },
                    memory::data_type::f32, memory::format_tag::ldigo);
    auto user_enc_uni_wei_iter_md = memory::desc({ user_enc_uni_wei_iter_dims },
            memory::data_type::f32, memory::format_tag::ldigo);
    auto user_enc_uni_bias_md = memory::desc({ user_enc_uni_bias_dims },
            memory::data_type::f32, memory::format_tag::ldgo);

    auto user_enc_uni_wei_layer_memory = memory(user_enc_uni_wei_layer_md,
            cpu_engine, user_enc_uni_wei_layer.data());
    auto user_enc_uni_wei_iter_memory = memory(
            user_enc_uni_wei_iter_md, cpu_engine, user_enc_uni_wei_iter.data());
    auto user_enc_uni_bias_memory = memory(
            user_enc_uni_bias_md, cpu_engine, user_enc_uni_bias.data());

    auto enc_uni_wei_layer_md = memory::desc({ user_enc_uni_wei_layer_dims },
            memory::data_type::s8, memory::format_tag::any);
    auto enc_uni_wei_iter_md = memory::desc({ user_enc_uni_wei_iter_dims },
            memory::data_type::s8, memory::format_tag::any);
    auto enc_dst_layer_md = memory::desc({ enc_dst_layer_dims },
            memory::data_type::f32, memory::format_tag::any);

    rnn_cell::desc enc_uni_cell(algorithm::vanilla_lstm);
    rnn_forward::desc enc_uni_layer_desc(prop_kind::forward_inference,
            enc_uni_cell, rnn_direction::unidirectional_left2right,
            enc_uni_first_dst_layer_md, memory::desc(), enc_uni_wei_layer_md,
            enc_uni_wei_iter_md, user_enc_uni_bias_md, enc_dst_layer_md,
            memory::desc());
    auto enc_uni_prim_desc
            = rnn_forward::primitive_desc(enc_uni_layer_desc, attr, cpu_engine);

    auto enc_uni_wei_layer_memory
            = memory(enc_uni_prim_desc.weights_layer_desc(), cpu_engine);
    auto enc_uni_wei_layer_reorder_pd = reorder::primitive_desc(
            user_enc_uni_wei_layer_memory, enc_uni_wei_layer_memory, attr);
    reorder(enc_uni_wei_layer_reorder_pd)
            .execute(
                    s, user_enc_uni_wei_layer_memory, enc_uni_wei_layer_memory);

    auto enc_uni_wei_iter_memory
            = memory(enc_uni_prim_desc.weights_iter_desc(), cpu_engine);
    auto enc_uni_wei_iter_reorder_pd = reorder::primitive_desc(
            user_enc_uni_wei_iter_memory, enc_uni_wei_iter_memory, attr);
    reorder(enc_uni_wei_iter_reorder_pd)
            .execute(s, user_enc_uni_wei_iter_memory, enc_uni_wei_iter_memory);

    auto enc_dst_layer_memory
            = memory(enc_uni_prim_desc.dst_layer_desc(), cpu_engine);

    encoder_net.push_back(rnn_forward(enc_uni_prim_desc));
    encoder_net_args.push_back(
            { { MKLDNN_ARG_SRC_LAYER, enc_uni_first_dst_layer_memory },
                    { MKLDNN_ARG_WEIGHTS_LAYER, enc_uni_wei_layer_memory },
                    { MKLDNN_ARG_WEIGHTS_ITER, enc_uni_wei_iter_memory },
                    { MKLDNN_ARG_BIAS, user_enc_uni_bias_memory },
                    { MKLDNN_ARG_DST_LAYER, enc_dst_layer_memory } });

    /* Decoder with attention mechanism */
    std::vector<float> user_dec_wei_layer(
            dec_n_layers * 1 * feature_size * lstm_n_gates * feature_size,
            0.2f);
    std::vector<float> user_dec_wei_iter(dec_n_layers * 1
                    * (feature_size + feature_size) * lstm_n_gates
                    * feature_size,
            0.3f);
    std::vector<float> user_dec_bias(
            dec_n_layers * 1 * lstm_n_gates * feature_size, 1.0f);
    std::vector<int8_t> user_weights_attention_src_layer(
            feature_size * feature_size, 1);
    float weights_attention_scale = 127.;
    std::vector<float> user_weights_annotation(
            feature_size * feature_size, 1.0f);
    std::vector<float> user_weights_alignments(feature_size, 1.0f);
    // Buffer to store decoder output for all iterations
    std::vector<uint8_t> dec_dst(tgt_seq_length_max * batch * feature_size, 0);

    memory::dims user_dec_wei_layer_dims
            = { dec_n_layers, 1, feature_size, lstm_n_gates, feature_size };
    memory::dims user_dec_wei_iter_dims = { dec_n_layers, 1,
        feature_size + feature_size, lstm_n_gates, feature_size };
    memory::dims user_dec_bias_dims
            = { dec_n_layers, 1, lstm_n_gates, feature_size };
    memory::dims dec_src_layer_dims = { 1, batch, feature_size };
    memory::dims dec_dst_layer_dims = { 1, batch, feature_size };

    // We will use the same memory for dec_src_iter and dec_dst_iter
    // However, dec_src_iter has a context vector but not
    // dec_dst_iter.
    // To resolve this we will create one memory that holds the
    // context vector as well as the both the hidden and cell states.
    // For the dst_iter, we will use a view on this memory.
    // Note that the cell state will be padded by
    // feature_size values. However, we do not compute or
    // access those.
    memory::dims dec_dst_iter_dims = { dec_n_layers, 1, lstm_n_states, batch,
        feature_size + feature_size };
    memory::dims dec_dst_iter_noctx_dims
            = { dec_n_layers, 1, lstm_n_states, batch, feature_size };

    auto user_dec_wei_layer_md = memory::desc({ user_dec_wei_layer_dims },
            memory::data_type::f32, memory::format_tag::ldigo);
    auto user_dec_wei_iter_md = memory::desc({ user_dec_wei_iter_dims },
            memory::data_type::f32, memory::format_tag::ldigo);
    auto user_dec_bias_md = memory::desc({ user_dec_bias_dims },
            memory::data_type::f32, memory::format_tag::ldgo);
    auto dec_src_layer_md = memory::desc({ dec_src_layer_dims },
            memory::data_type::u8, memory::format_tag::tnc);
    auto dec_dst_layer_md = memory::desc({ dec_dst_layer_dims },
            memory::data_type::u8, memory::format_tag::tnc);
    auto dec_dst_iter_md = memory::desc({ dec_dst_iter_dims },
            memory::data_type::f32, memory::format_tag::ldsnc);

    auto user_dec_wei_layer_memory = memory(
            user_dec_wei_layer_md, cpu_engine, user_dec_wei_layer.data());
    auto user_dec_wei_iter_memory = memory(
            user_dec_wei_iter_md, cpu_engine, user_dec_wei_iter.data());
    auto user_dec_bias_memory
            = memory(user_dec_bias_md, cpu_engine, user_dec_bias.data());
    auto dec_src_layer_memory = memory(dec_src_layer_md, cpu_engine);
    auto dec_dst_layer_memory
            = memory(dec_dst_layer_md, cpu_engine, dec_dst.data());

    /* Create memory descriptors for RNN data w/o specified layout */
    auto dec_wei_layer_md = memory::desc({ user_dec_wei_layer_dims },
            memory::data_type::s8, memory::format_tag::any);
    auto dec_wei_iter_md = memory::desc({ user_dec_wei_iter_dims },
            memory::data_type::s8, memory::format_tag::any);

    /* As mentioned above, we create a view without context out of the
     memory with context. */
    auto dec_dst_iter_memory = memory(dec_dst_iter_md, cpu_engine);
    auto dec_dst_iter_noctx_md = dec_dst_iter_md.submemory_desc(
            dec_dst_iter_noctx_dims, { 0, 0, 0, 0, 0 });

    rnn_cell::desc dec_cell(algorithm::vanilla_lstm);
    rnn_forward::desc dec_ctx_desc(prop_kind::forward_inference, dec_cell,
            rnn_direction::unidirectional_left2right, dec_src_layer_md,
            dec_dst_iter_md, dec_wei_layer_md, dec_wei_iter_md,
            user_dec_bias_md, dec_dst_layer_md, dec_dst_iter_noctx_md);
    auto dec_ctx_prim_desc
            = rnn_forward::primitive_desc(dec_ctx_desc, attr, cpu_engine);

    /* Create memory for input data and use reorders to quantize values
     * to int8 */
    auto dec_wei_layer_memory
            = memory(dec_ctx_prim_desc.weights_layer_desc(), cpu_engine);
    auto dec_wei_layer_reorder_pd = reorder::primitive_desc(
            user_dec_wei_layer_memory, dec_wei_layer_memory, attr);
    reorder(dec_wei_layer_reorder_pd)
            .execute(s, user_dec_wei_layer_memory, dec_wei_layer_memory);

    auto dec_wei_iter_memory
            = memory(dec_ctx_prim_desc.weights_iter_desc(), cpu_engine);
    auto dec_wei_iter_reorder_pd = reorder::primitive_desc(
            user_dec_wei_iter_memory, dec_wei_iter_memory, attr);
    reorder(dec_wei_iter_reorder_pd)
            .execute(s, user_dec_wei_iter_memory, dec_wei_iter_memory);

    decoder_net.push_back(rnn_forward(dec_ctx_prim_desc));
    decoder_net_args.push_back({ { MKLDNN_ARG_SRC_LAYER, dec_src_layer_memory },
            { MKLDNN_ARG_SRC_ITER, dec_dst_iter_memory },
            { MKLDNN_ARG_WEIGHTS_LAYER, dec_wei_layer_memory },
            { MKLDNN_ARG_WEIGHTS_ITER, dec_wei_iter_memory },
            { MKLDNN_ARG_BIAS, user_dec_bias_memory },
            { MKLDNN_ARG_DST_LAYER, dec_dst_layer_memory },
            { MKLDNN_ARG_DST_ITER, dec_dst_iter_memory } });

    /* Allocating temporary buffers for attention mechanism */
    std::vector<float> weighted_annotations(
            src_seq_length_max * batch * feature_size, 1.0f);
    std::vector<int32_t> weights_attention_sum_rows(feature_size, 1);

    /*
       Execution
     */
    auto execute = [&]() {
        // run encoder (1 stream)
        assert(encoder_net.size() == encoder_net_args.size()
                && "something is missing");
        for (size_t p = 0; p < encoder_net.size(); ++p)
            encoder_net.at(p).execute(s, encoder_net_args.at(p));

        // compute the weighted annotations once before the decoder
        compute_weighted_annotations(weighted_annotations.data(),
                src_seq_length_max, batch, feature_size,
                user_weights_annotation.data(),
                (float *)enc_dst_layer_memory.get_data_handle());
        // precompute compensation for s8u8s32 gemm in compute attention
        compute_sum_of_rows(user_weights_attention_src_layer.data(),
                feature_size, feature_size, weights_attention_sum_rows.data());

        // We initialise src_layer to the embedding of </s>, which
        // are assumed to be 0 here
        memset(dec_src_layer_memory.get_data_handle(), 0,
                dec_src_layer_memory.get_desc().get_size());
        // From now on, src points to the output of the last iteration

        for (dim_t i = 0; i < tgt_seq_length_max; i++) {
            uint8_t *src_att_layer_handle
                    = (uint8_t *)dec_src_layer_memory.get_data_handle();
            float *src_att_iter_handle
                    = (float *)dec_dst_iter_memory.get_data_handle();

            // Compute attention context vector into the first layer src_iter
            compute_attention(src_att_iter_handle, src_seq_length_max, batch,
                    feature_size, user_weights_attention_src_layer.data(),
                    weights_attention_scale, weights_attention_sum_rows.data(),
                    src_att_layer_handle, data_scale, data_shift,
                    (uint8_t *)enc_bidir_dst_layer_memory.get_data_handle(),
                    weighted_annotations.data(),
                    user_weights_alignments.data());

            // copy the context vectors to all layers of src_iter
            copy_context(src_att_iter_handle, dec_n_layers, lstm_n_states,
                    batch, feature_size);

            // run the decoder iteration
            assert(decoder_net.size() == decoder_net_args.size()
                    && "something is missing");
            for (size_t p = 0; p < decoder_net.size(); ++p)
                decoder_net.at(p).execute(s, decoder_net_args.at(p));

            // Move the handle on the src/dst layer to the next iteration
            auto dst_layer_handle
                    = (uint8_t *)dec_dst_layer_memory.get_data_handle();
            dec_src_layer_memory.set_data_handle(dst_layer_handle);
            dec_dst_layer_memory.set_data_handle(
                    dst_layer_handle + batch * feature_size);
        }

    };

    execute();
}

int main(int argc, char **argv) {
    try {
        simple_net();
        std::cout << "ok\n";
    } catch (error &e) {
        std::cerr << "status: " << e.status << std::endl;
        std::cerr << "message: " << e.message << std::endl;
    }
    return 0;
}
