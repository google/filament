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

#include <cstring>
#include <iostream>
#include <math.h>
#include <numeric>
#include <string>

#include "mkldnn.hpp"

using namespace mkldnn;

// User input is:
//     N0 sequences of length T0
const int N0 = 1 + rand() % 31;
//     N1 sequences of length T1
const int N1 = 1 + rand() % 31;
// Assume T0 > T1
const int T0 = 31 + 1 + rand() % 31;
const int T1 = 1 + rand() % 31;

// Memory required to hold it: N0 * T0 + N1 * T1
// However it is possible to have these coming
// as padded chunks in larger memory:
//      e.g. (N0 + N1) * T0
// We don't need to compact the data before processing,
// we can address the chunks via sub-memory and
// process the data via two RNN primitives:
//     of time lengths T1 and T0 - T1.
// The leftmost primitive will process N0 + N1 subsequences of length T1
// The rightmost primitive will process remaining N0 subsequences
// of T0 - T1 length
const int leftmost_batch = N0 + N1;
const int rightmost_batch = N0;

const int leftmost_seq_length = T1;
const int rightmost_seq_length = T0 - T1;

// Number of channels
const int common_feature_size = 1024;

// RNN primitive characteristics
const int common_n_layers = 1;
const int lstm_n_gates = 4;
const int lstm_n_states = 2;

void simple_net() {
    using tag = memory::format_tag;
    using dt = memory::data_type;

    auto cpu_engine = engine(engine::cpu, 0);
    stream s(cpu_engine);

    bool is_training = true;
    auto fwd_inf_train = is_training ? prop_kind::forward_training :
                                       prop_kind::forward_inference;

    std::vector<primitive> fwd_net;
    std::vector<primitive> bwd_net;

    // Input tensor holds two batches with different sequence lengths.
    // Shorter sequences are padded
    memory::dims net_src_dims = {
        /* time */ T0, // maximum sequence length
        /* n    */ N0 + N1, // total batch size
        /* c    */ common_feature_size // common number of channels
    };

    /*
     * Two RNN primitives for different sequence lenghts,
     * one unidirectional layer, LSTM-based
     */

    memory::dims leftmost_src_layer_dims = { /* time */ leftmost_seq_length,
        /* n    */ leftmost_batch,
        /* c    */ common_feature_size };
    memory::dims rightmost_src_layer_dims = { /* time */ rightmost_seq_length,
        /* n    */ rightmost_batch,
        /* c    */ common_feature_size };
    memory::dims common_weights_layer_dims
            = { /* layers              */ common_n_layers,
                  /* directions          */ 1,
                  /* input feature size  */ common_feature_size,
                  /* gates number        */ lstm_n_gates,
                  /* output feature size */ common_feature_size };
    memory::dims common_weights_iter_dims
            = { /* layers              */ common_n_layers,
                  /* directions          */ 1,
                  /* input feature size  */ common_feature_size,
                  /* gates number        */ lstm_n_gates,
                  /* output feature size */ common_feature_size };
    memory::dims common_bias_dims = { /* layers              */ common_n_layers,
        /* directions          */ 1,
        /* gates number        */ lstm_n_gates,
        /* output feature size */ common_feature_size };
    memory::dims leftmost_dst_layer_dims = { /* time */ leftmost_seq_length,
        /* n    */ leftmost_batch,
        /* c    */ common_feature_size };
    memory::dims rightmost_dst_layer_dims = { /* time */ rightmost_seq_length,
        /* n    */ rightmost_batch,
        /* c    */ common_feature_size };

    // leftmost primitive passes its states to the next RNN iteration
    // so it needs dst_iter parameter.
    //
    // rightmost primitive will consume these as src_iter and will access the
    // memory via a sub-memory because it will have different batch dimension.
    // We have arranged our primitives so that
    // leftmost_batch >= rightmost_batch, and so the rightmost data will fit
    // into the memory allocated for the leftmost.
    memory::dims leftmost_dst_iter_dims = { /* layers     */ common_n_layers,
        /* directions */ 1,
        /* states     */ lstm_n_states,
        /* n          */ leftmost_batch,
        /* c          */ common_feature_size };
    memory::dims rightmost_src_iter_dims = { /* layers     */ common_n_layers,
        /* directions */ 1,
        /* states     */ lstm_n_states,
        /* n          */ rightmost_batch,
        /* c          */ common_feature_size };

    // multiplication of tensor dimensions
    auto tz_volume = [=](memory::dims tz_dims) {
        return std::accumulate(tz_dims.begin(), tz_dims.end(), (memory::dim)1,
                std::multiplies<memory::dim>());
    };

    // Create auxillary f32 memory descriptor
    // based on user- supplied dimensions and layout.
    auto formatted_md
            = [=](memory::dims dimensions, memory::format_tag layout) {
                  return memory::desc{ { dimensions }, dt::f32, layout };
              };
    // Create auxillary generic f32 memory descriptor
    // based on supplied dimensions, with format_tag::any.
    auto generic_md = [=](memory::dims dimensions) {
        return formatted_md(dimensions, tag::any);
    };

    //
    // I/O memory, coming from user
    //

    // Net input
    std::vector<float> net_src(tz_volume(net_src_dims), 1.0f);
    // NOTE: in this example we study input sequences with variable batch
    // dimension, which get processed by two separate RNN primitives, thus
    // the destination memory for the two will have different shapes: batch
    // is the second dimension currently: see format_tag::tnc.
    // We are not copying the output to some common user provided memory as we
    // suggest that the user should rather keep the two output memories separate
    // throughout the whole topology and only reorder to something else as
    // needed.
    // So there's no common net_dst, but there are two destinations instead:
    //    leftmost_dst_layer_memory
    //    rightmost_dst_layer_memory

    // Memory for the user allocated memory
    // Suppose user data is in tnc format.
    auto net_src_memory
            = mkldnn::memory({ { net_src_dims }, dt::f32, tag::tnc },
                    cpu_engine, net_src.data());
    // src_layer memory of the leftmost and rightmost RNN primitives
    // are accessed through the respective sub-memories in larger memory.
    // View primitives compute the strides to accomodate for padding.
    auto user_leftmost_src_layer_md = net_src_memory.get_desc().submemory_desc(
            leftmost_src_layer_dims, { 0, 0, 0 } /* t, n, c offsets */);
    auto user_rightmost_src_layer_md
            = net_src_memory.get_desc().submemory_desc(rightmost_src_layer_dims,
                    { leftmost_seq_length, 0, 0 } /* t, n, c offsets */);
    auto leftmost_src_layer_memory = net_src_memory;
    auto rightmost_src_layer_memory = net_src_memory;

    // Other user provided memory arrays, desrciptors and primitives with the
    // data layouts chosen by user. We'll have to reorder if RNN
    // primitive prefers it in a different format.
    std::vector<float> user_common_weights_layer(
            tz_volume(common_weights_layer_dims), 1.0f);
    auto user_common_weights_layer_memory
            = mkldnn::memory({ common_weights_layer_dims, dt::f32, tag::ldigo },
                    cpu_engine, user_common_weights_layer.data());

    std::vector<float> user_common_weights_iter(
            tz_volume(common_weights_iter_dims), 1.0f);
    auto user_common_weights_iter_memory = mkldnn::memory(
            { { common_weights_iter_dims }, dt::f32, tag::ldigo }, cpu_engine,
            user_common_weights_layer.data());

    std::vector<float> user_common_bias(tz_volume(common_bias_dims), 1.0f);
    auto user_common_bias_memory
            = mkldnn::memory({ { common_bias_dims }, dt::f32, tag::ldgo },
                    cpu_engine, user_common_bias.data());

    std::vector<float> user_leftmost_dst_layer(
            tz_volume(leftmost_dst_layer_dims), 1.0f);
    auto user_leftmost_dst_layer_memory
            = mkldnn::memory({ { leftmost_dst_layer_dims }, dt::f32, tag::tnc },
                    cpu_engine, user_leftmost_dst_layer.data());

    std::vector<float> user_rightmost_dst_layer(
            tz_volume(rightmost_dst_layer_dims), 1.0f);
    auto user_rightmost_dst_layer_memory = mkldnn::memory(
            { { rightmost_dst_layer_dims }, dt::f32, tag::tnc }, cpu_engine,
            user_rightmost_dst_layer.data());

    // Describe RNN cell
    rnn_cell::desc uni_cell(algorithm::vanilla_lstm);

    // Describe layer, forward pass, leftmost primitive.
    // There are no primitives to the left from here,
    // so src_iter_desc needs to be zero memory desc
    rnn_forward::desc leftmost_layer_desc(
            /* aprop_kind         */ fwd_inf_train,
            /* cell               */ uni_cell,
            /* direction          */ rnn_direction::unidirectional_left2right,
            /* src_layer_desc     */ user_leftmost_src_layer_md,
            /* src_iter_desc      */ memory::desc(),
            /* weights_layer_desc */ generic_md(common_weights_layer_dims),
            /* weights_iter_desc  */ generic_md(common_weights_iter_dims),
            /* bias_desc          */ generic_md(common_bias_dims),
            /* dst_layer_desc     */
            formatted_md(leftmost_dst_layer_dims, tag::tnc),
            /* dst_iter_desc      */ generic_md(leftmost_dst_iter_dims));
    // Describe primitive
    auto leftmost_prim_desc = mkldnn::rnn_forward::primitive_desc(
            leftmost_layer_desc, cpu_engine);

    //
    // Need to connect leftmost and rightmost via "iter" parameters.
    // We allocate memory here based on the shapes provided by RNN primitive.
    //

    auto leftmost_dst_iter_memory
            = mkldnn::memory(leftmost_prim_desc.dst_iter_desc(), cpu_engine);

    // rightmost src_iter will be a sub-memory of dst_iter of leftmost
    auto rightmost_src_iter_md
            = leftmost_dst_iter_memory.get_desc().submemory_desc(
                    rightmost_src_iter_dims,
                    { 0, 0, 0, 0, 0 } /* l, d, s, n, c offsets */);

    auto rightmost_src_iter_memory = leftmost_dst_iter_memory;

    // Now rightmost primitive
    // There are no primitives to the right from here,
    // so dst_iter_desc is explicit zero memory desc
    rnn_forward::desc rightmost_layer_desc(
            /* aprop_kind         */ fwd_inf_train,
            /* cell               */ uni_cell,
            /* direction          */ rnn_direction::unidirectional_left2right,
            /* src_layer_desc     */ user_rightmost_src_layer_md,
            /* src_iter_desc      */ rightmost_src_iter_md,
            /* weights_layer_desc */ generic_md(common_weights_layer_dims),
            /* weights_iter_desc  */ generic_md(common_weights_iter_dims),
            /* bias_desc          */ generic_md(common_bias_dims),
            /* dst_layer_desc     */
            formatted_md(rightmost_dst_layer_dims, tag::tnc),
            /* dst_iter_desc      */ memory::desc());
    auto rightmost_prim_desc = mkldnn::rnn_forward::primitive_desc(
            rightmost_layer_desc, cpu_engine);

    //
    // Weights and biases, layer memory
    // Same layout should work across the layer, no reorders
    // needed between leftmost and rigthmost, only reordering
    // user memory to the RNN-friendly shapes.
    //

    auto common_weights_layer_memory = user_common_weights_layer_memory;
    if (leftmost_prim_desc.weights_layer_desc()
            != common_weights_layer_memory.get_desc()) {
        common_weights_layer_memory = mkldnn::memory(
                leftmost_prim_desc.weights_layer_desc(), cpu_engine);
        reorder(user_common_weights_layer_memory, common_weights_layer_memory)
                .execute(s, user_common_weights_layer_memory,
                        common_weights_layer_memory);
    }

    auto common_weights_iter_memory = user_common_weights_iter_memory;
    if (leftmost_prim_desc.weights_iter_desc()
            != common_weights_iter_memory.get_desc()) {
        common_weights_iter_memory = mkldnn::memory(
                leftmost_prim_desc.weights_iter_desc(), cpu_engine);
        reorder(user_common_weights_iter_memory, common_weights_iter_memory)
                .execute(s, user_common_weights_iter_memory,
                        common_weights_iter_memory);
    }

    auto common_bias_memory = user_common_bias_memory;
    if (leftmost_prim_desc.bias_desc() != common_bias_memory.get_desc()) {
        common_bias_memory
                = mkldnn::memory(leftmost_prim_desc.bias_desc(), cpu_engine);
        reorder(user_common_bias_memory, common_bias_memory)
                .execute(s, user_common_bias_memory, common_bias_memory);
    }

    //
    // Destination layer memory
    //

    auto leftmost_dst_layer_memory = user_leftmost_dst_layer_memory;
    if (leftmost_prim_desc.dst_layer_desc()
            != leftmost_dst_layer_memory.get_desc()) {
        leftmost_dst_layer_memory = mkldnn::memory(
                leftmost_prim_desc.dst_layer_desc(), cpu_engine);
        reorder(user_leftmost_dst_layer_memory, leftmost_dst_layer_memory)
                .execute(s, user_leftmost_dst_layer_memory,
                        leftmost_dst_layer_memory);
    }

    auto rightmost_dst_layer_memory = user_rightmost_dst_layer_memory;
    if (rightmost_prim_desc.dst_layer_desc()
            != rightmost_dst_layer_memory.get_desc()) {
        rightmost_dst_layer_memory = mkldnn::memory(
                rightmost_prim_desc.dst_layer_desc(), cpu_engine);
        reorder(user_rightmost_dst_layer_memory, rightmost_dst_layer_memory)
                .execute(s, user_rightmost_dst_layer_memory,
                        rightmost_dst_layer_memory);
    }

    // We also create workspace memory based on the information from
    // the workspace_primitive_desc(). This is needed for internal
    // communication between forward and backward primitives during
    // training.
    auto create_ws = [=](mkldnn::rnn_forward::primitive_desc &pd) {
        return mkldnn::memory(pd.workspace_desc(), cpu_engine);
    };
    auto leftmost_workspace_memory = create_ws(leftmost_prim_desc);
    auto rightmost_workspace_memory = create_ws(rightmost_prim_desc);

    // Construct the RNN primitive objects
    rnn_forward leftmost_layer(leftmost_prim_desc);
    leftmost_layer.execute(s,
            { { MKLDNN_ARG_SRC_LAYER, leftmost_src_layer_memory },
                    { MKLDNN_ARG_WEIGHTS_LAYER, common_weights_layer_memory },
                    { MKLDNN_ARG_WEIGHTS_ITER, common_weights_iter_memory },
                    { MKLDNN_ARG_BIAS, common_bias_memory },
                    { MKLDNN_ARG_DST_LAYER, leftmost_dst_layer_memory },
                    { MKLDNN_ARG_DST_ITER, leftmost_dst_iter_memory },
                    { MKLDNN_ARG_WORKSPACE, leftmost_workspace_memory } });

    rnn_forward rightmost_layer(rightmost_prim_desc);
    rightmost_layer.execute(s,
            { { MKLDNN_ARG_SRC_LAYER, rightmost_src_layer_memory },
                    { MKLDNN_ARG_SRC_ITER, rightmost_src_iter_memory },
                    { MKLDNN_ARG_WEIGHTS_LAYER, common_weights_layer_memory },
                    { MKLDNN_ARG_WEIGHTS_ITER, common_weights_iter_memory },
                    { MKLDNN_ARG_BIAS, common_bias_memory },
                    { MKLDNN_ARG_DST_LAYER, rightmost_dst_layer_memory },
                    { MKLDNN_ARG_WORKSPACE, rightmost_workspace_memory } });

    // No backward pass for inference
    if (!is_training)
        return;

    //
    // Backward primitives will reuse memory from forward
    // and allocate/describe specifics here. Only relevant for training.
    //

    // User-provided memory for backward by data output
    std::vector<float> net_diff_src(tz_volume(net_src_dims), 1.0f);
    auto net_diff_src_memory
            = mkldnn::memory(formatted_md(net_src_dims, tag::tnc), cpu_engine,
                    net_diff_src.data());

    // diff_src follows the same layout we have for net_src
    auto user_leftmost_diff_src_layer_md
            = net_diff_src_memory.get_desc().submemory_desc(
                    leftmost_src_layer_dims, { 0, 0, 0 } /* t, n, c offsets */);
    auto user_rightmost_diff_src_layer_md
            = net_diff_src_memory.get_desc().submemory_desc(
                    rightmost_src_layer_dims,
                    { leftmost_seq_length, 0, 0 } /* t, n, c offsets */);

    auto leftmost_diff_src_layer_memory = net_diff_src_memory;
    auto rightmost_diff_src_layer_memory = net_diff_src_memory;

    // User-provided memory for backpropagation by weights
    std::vector<float> user_common_diff_weights_layer(
            tz_volume(common_weights_layer_dims), 1.0f);
    auto user_common_diff_weights_layer_memory = mkldnn::memory(
            formatted_md(common_weights_layer_dims, tag::ldigo), cpu_engine,
            user_common_diff_weights_layer.data());

    std::vector<float> user_common_diff_bias(tz_volume(common_bias_dims), 1.0f);
    auto user_common_diff_bias_memory
            = mkldnn::memory(formatted_md(common_bias_dims, tag::ldgo),
                    cpu_engine, user_common_diff_bias.data());

    // User-provided input to the backward primitive.
    // To be updated by the user after forward pass using some cost function.
    memory::dims net_diff_dst_dims = { /* time */ T0,
        /* n    */ N0 + N1,
        /* c    */ common_feature_size };
    // Suppose user data is in tnc format.
    std::vector<float> net_diff_dst(tz_volume(net_diff_dst_dims), 1.0f);
    auto net_diff_dst_memory
            = mkldnn::memory(formatted_md(net_diff_dst_dims, tag::tnc),
                    cpu_engine, net_diff_dst.data());
    // diff_dst_layer memory of the leftmost and rightmost RNN primitives
    // are accessed through the respective sub-memory in larger memory.
    // View primitives compute the strides to accomodate for padding.
    auto user_leftmost_diff_dst_layer_md
            = net_diff_dst_memory.get_desc().submemory_desc(
                    leftmost_dst_layer_dims, { 0, 0, 0 } /* t, n, c offsets */);
    auto user_rightmost_diff_dst_layer_md
            = net_diff_dst_memory.get_desc().submemory_desc(
                    rightmost_dst_layer_dims,
                    { leftmost_seq_length, 0, 0 } /* t, n, c offsets */);
    auto leftmost_diff_dst_layer_memory = net_diff_dst_memory;
    auto rightmost_diff_dst_layer_memory = net_diff_dst_memory;

    // Backward leftmost primitive descriptor
    rnn_backward::desc leftmost_layer_bwd_desc(
            /* aprop_kind              */ prop_kind::backward,
            /* cell                    */ uni_cell,
            /* direction               */
            rnn_direction::unidirectional_left2right,
            /* src_layer_desc          */ user_leftmost_src_layer_md,
            /* src_iter_desc           */ memory::desc(),
            /* weights_layer_desc      */ generic_md(common_weights_layer_dims),
            /* weights_iter_desc       */ generic_md(common_weights_iter_dims),
            /* bias_desc               */ generic_md(common_bias_dims),
            /* dst_layer_desc          */
            formatted_md(leftmost_dst_layer_dims, tag::tnc),
            /* dst_iter_desc           */ generic_md(leftmost_dst_iter_dims),
            /* diff_src_layer_desc     */ user_leftmost_diff_src_layer_md,
            /* diff_src_iter_desc      */ memory::desc(),
            /* diff_weights_layer_desc */ generic_md(common_weights_layer_dims),
            /* diff_weights_iter_desc  */ generic_md(common_weights_iter_dims),
            /* diff_bias_desc          */ generic_md(common_bias_dims),
            /* diff_dst_layer_desc     */ user_leftmost_diff_dst_layer_md,
            /* diff_dst_iter_desc      */ generic_md(leftmost_dst_iter_dims));
    auto leftmost_bwd_prim_desc = mkldnn::rnn_backward::primitive_desc(
            leftmost_layer_bwd_desc, cpu_engine, leftmost_prim_desc);

    // As the batch dimensions are different between leftmost and rightmost
    // we need to use a sub-memory. rightmost needs less memory, so it will
    // be a sub-memory of leftmost.
    auto leftmost_diff_dst_iter_memory = mkldnn::memory(
            leftmost_bwd_prim_desc.diff_dst_iter_desc(), cpu_engine);

    auto rightmost_diff_src_iter_md
            = leftmost_diff_dst_iter_memory.get_desc().submemory_desc(
                    rightmost_src_iter_dims,
                    { 0, 0, 0, 0, 0 } /* l, d, s, n, c offsets */);

    auto rightmost_diff_src_iter_memory = leftmost_diff_dst_iter_memory;

    // Backward rightmost primitive descriptor
    rnn_backward::desc rightmost_layer_bwd_desc(
            /* aprop_kind              */ prop_kind::backward,
            /* cell                    */ uni_cell,
            /* direction               */
            rnn_direction::unidirectional_left2right,
            /* src_layer_desc          */ user_rightmost_src_layer_md,
            /* src_iter_desc           */ generic_md(rightmost_src_iter_dims),
            /* weights_layer_desc      */ generic_md(common_weights_layer_dims),
            /* weights_iter_desc       */ generic_md(common_weights_iter_dims),
            /* bias_desc               */ generic_md(common_bias_dims),
            /* dst_layer_desc          */
            formatted_md(rightmost_dst_layer_dims, tag::tnc),
            /* dst_iter_desc           */ memory::desc(),
            /* diff_src_layer_desc     */ user_rightmost_diff_src_layer_md,
            /* diff_src_iter_desc      */ rightmost_diff_src_iter_md,
            /* diff_weights_layer_desc */ generic_md(common_weights_layer_dims),
            /* diff_weights_iter_desc  */ generic_md(common_weights_iter_dims),
            /* diff_bias_desc          */ generic_md(common_bias_dims),
            /* diff_dst_layer_desc     */ user_rightmost_diff_dst_layer_md,
            /* diff_dst_iter_desc      */ memory::desc());
    auto rightmost_bwd_prim_desc = mkldnn::rnn_backward::primitive_desc(
            rightmost_layer_bwd_desc, cpu_engine, rightmost_prim_desc);

    //
    // Memory for backward pass
    //

    // src layer uses the same memory as forward
    auto leftmost_src_layer_bwd_memory = leftmost_src_layer_memory;
    auto rightmost_src_layer_bwd_memory = rightmost_src_layer_memory;

    // Memory for weights and biases for backward pass
    // Try to use the same memory between forward and backward, but
    // sometimes reorders are needed.
    auto common_weights_layer_bwd_memory = common_weights_layer_memory;
    if (leftmost_bwd_prim_desc.weights_layer_desc()
            != leftmost_prim_desc.weights_layer_desc()) {
        common_weights_layer_bwd_memory = memory(
                leftmost_bwd_prim_desc.weights_layer_desc(), cpu_engine);
        reorder(common_weights_layer_memory, common_weights_layer_bwd_memory)
                .execute(s, common_weights_layer_memory,
                        common_weights_layer_bwd_memory);
    }

    auto common_weights_iter_bwd_memory = common_weights_iter_memory;
    if (leftmost_bwd_prim_desc.weights_iter_desc()
            != leftmost_prim_desc.weights_iter_desc()) {
        common_weights_iter_bwd_memory = memory(
                leftmost_bwd_prim_desc.weights_iter_desc(), cpu_engine);
        reorder(common_weights_iter_memory, common_weights_iter_bwd_memory)
                .execute(s, common_weights_iter_memory,
                        common_weights_iter_bwd_memory);
    }

    auto common_bias_bwd_memory = common_bias_memory;
    if (leftmost_bwd_prim_desc.bias_desc() != common_bias_memory.get_desc()) {
        common_bias_bwd_memory = mkldnn::memory(
                leftmost_bwd_prim_desc.bias_desc(), cpu_engine);
        reorder(common_bias_memory, common_bias_bwd_memory)
                .execute(s, common_bias_memory, common_bias_bwd_memory);
    }

    // diff_weights and biases
    auto common_diff_weights_layer_memory
            = user_common_diff_weights_layer_memory;
    auto reorder_common_diff_weights_layer = false;
    if (leftmost_bwd_prim_desc.diff_weights_layer_desc()
            != common_diff_weights_layer_memory.get_desc()) {
        common_diff_weights_layer_memory = mkldnn::memory(
                leftmost_bwd_prim_desc.diff_weights_layer_desc(), cpu_engine);
        reorder_common_diff_weights_layer = true;
    }

    auto common_diff_bias_memory = user_common_diff_bias_memory;
    auto reorder_common_diff_bias = false;
    if (leftmost_bwd_prim_desc.diff_bias_desc()
            != common_diff_bias_memory.get_desc()) {
        common_diff_bias_memory = mkldnn::memory(
                leftmost_bwd_prim_desc.diff_bias_desc(), cpu_engine);
        reorder_common_diff_bias = true;
    }

    // dst_layer memory for backward pass
    auto leftmost_dst_layer_bwd_memory = leftmost_dst_layer_memory;
    if (leftmost_bwd_prim_desc.dst_layer_desc()
            != leftmost_dst_layer_bwd_memory.get_desc()) {
        leftmost_dst_layer_bwd_memory = mkldnn::memory(
                leftmost_bwd_prim_desc.dst_layer_desc(), cpu_engine);
        reorder(leftmost_dst_layer_memory, leftmost_dst_layer_bwd_memory)
                .execute(s, leftmost_dst_layer_memory,
                        leftmost_dst_layer_bwd_memory);
    }

    auto rightmost_dst_layer_bwd_memory = rightmost_dst_layer_memory;
    if (rightmost_bwd_prim_desc.dst_layer_desc()
            != rightmost_dst_layer_bwd_memory.get_desc()) {
        rightmost_dst_layer_bwd_memory = mkldnn::memory(
                rightmost_bwd_prim_desc.dst_layer_desc(), cpu_engine);
        reorder(rightmost_dst_layer_memory, rightmost_dst_layer_bwd_memory)
                .execute(s, rightmost_dst_layer_memory,
                        rightmost_dst_layer_bwd_memory);
    }

    // Similar to forward, the backward primitives are connected
    // via "iter" parameters.
    auto common_diff_weights_iter_memory = mkldnn::memory(
            leftmost_bwd_prim_desc.diff_weights_iter_desc(), cpu_engine);

    auto leftmost_dst_iter_bwd_memory = leftmost_dst_iter_memory;
    if (leftmost_bwd_prim_desc.dst_iter_desc()
            != leftmost_dst_iter_bwd_memory.get_desc()) {
        leftmost_dst_iter_bwd_memory = mkldnn::memory(
                leftmost_bwd_prim_desc.dst_iter_desc(), cpu_engine);
        reorder(leftmost_dst_iter_memory, leftmost_dst_iter_bwd_memory)
                .execute(s, leftmost_dst_iter_memory,
                        leftmost_dst_iter_bwd_memory);
    }

    // Construct the RNN primitive objects for backward
    rnn_backward rightmost_layer_bwd(rightmost_bwd_prim_desc);
    rightmost_layer_bwd.execute(s,
            { { MKLDNN_ARG_SRC_LAYER, rightmost_src_layer_bwd_memory },
                    { MKLDNN_ARG_SRC_ITER, rightmost_src_iter_memory },
                    { MKLDNN_ARG_WEIGHTS_LAYER,
                            common_weights_layer_bwd_memory },
                    { MKLDNN_ARG_WEIGHTS_ITER, common_weights_iter_bwd_memory },
                    { MKLDNN_ARG_BIAS, common_bias_bwd_memory },
                    { MKLDNN_ARG_DST_LAYER, rightmost_dst_layer_bwd_memory },
                    { MKLDNN_ARG_DIFF_SRC_LAYER,
                            rightmost_diff_src_layer_memory },
                    { MKLDNN_ARG_DIFF_SRC_ITER,
                            rightmost_diff_src_iter_memory },
                    { MKLDNN_ARG_DIFF_WEIGHTS_LAYER,
                            common_diff_weights_layer_memory },
                    { MKLDNN_ARG_DIFF_WEIGHTS_ITER,
                            common_diff_weights_iter_memory },
                    { MKLDNN_ARG_DIFF_BIAS, common_diff_bias_memory },
                    { MKLDNN_ARG_DIFF_DST_LAYER,
                            rightmost_diff_dst_layer_memory },
                    { MKLDNN_ARG_WORKSPACE, rightmost_workspace_memory } });

    rnn_backward leftmost_layer_bwd(leftmost_bwd_prim_desc);
    leftmost_layer_bwd.execute(s,
            { { MKLDNN_ARG_SRC_LAYER, leftmost_src_layer_bwd_memory },
                    { MKLDNN_ARG_WEIGHTS_LAYER,
                            common_weights_layer_bwd_memory },
                    { MKLDNN_ARG_WEIGHTS_ITER, common_weights_iter_bwd_memory },
                    { MKLDNN_ARG_BIAS, common_bias_bwd_memory },
                    { MKLDNN_ARG_DST_LAYER, leftmost_dst_layer_bwd_memory },
                    { MKLDNN_ARG_DST_ITER, leftmost_dst_iter_bwd_memory },
                    { MKLDNN_ARG_DIFF_SRC_LAYER,
                            leftmost_diff_src_layer_memory },
                    { MKLDNN_ARG_DIFF_WEIGHTS_LAYER,
                            common_diff_weights_layer_memory },
                    { MKLDNN_ARG_DIFF_WEIGHTS_ITER,
                            common_diff_weights_iter_memory },
                    { MKLDNN_ARG_DIFF_BIAS, common_diff_bias_memory },
                    { MKLDNN_ARG_DIFF_DST_LAYER,
                            leftmost_diff_dst_layer_memory },
                    { MKLDNN_ARG_DIFF_DST_ITER, leftmost_diff_dst_iter_memory },
                    { MKLDNN_ARG_WORKSPACE, leftmost_workspace_memory } });

    if (reorder_common_diff_weights_layer) {
        reorder(common_diff_weights_layer_memory,
                user_common_diff_weights_layer_memory)
                .execute(s, common_diff_weights_layer_memory,
                        user_common_diff_weights_layer_memory);
    }

    if (reorder_common_diff_bias) {
        reorder(common_diff_bias_memory, user_common_diff_bias_memory)
                .execute(s, common_diff_bias_memory,
                        user_common_diff_bias_memory);
    }

    //
    // User updates weights and bias using diffs
    //
}

int main(int argc, char **argv) {
    try {
        simple_net();
        std::cout << "ok\n";
    } catch (error &e) {
        std::cerr << "status: " << e.status << std::endl;
        std::cerr << "message: " << e.message << std::endl;
        return 1;
    }
    return 0;
}
