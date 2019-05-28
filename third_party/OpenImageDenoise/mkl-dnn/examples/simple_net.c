/*******************************************************************************
* Copyright 2016-2018 Intel Corporation
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

// Required for posix_memalign
#define _POSIX_C_SOURCE 200112L

#include "mkldnn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BATCH 8
#define IC 3
#define OC 96
#define CONV_IH 227
#define CONV_IW 227
#define CONV_OH 55
#define CONV_OW 55
#define CONV_STRIDE 4
#define CONV_PAD 0
#define POOL_OH 27
#define POOL_OW 27
#define POOL_STRIDE 2
#define POOL_PAD 0

#define CHECK(f)                                                             \
    do {                                                                     \
        mkldnn_status_t s = f;                                               \
        if (s != mkldnn_success) {                                           \
            printf("[%s:%d] error: %s returns %d\n", __FILE__, __LINE__, #f, \
                    s);                                                      \
            exit(2);                                                         \
        }                                                                    \
    } while (0)

#define CHECK_TRUE(expr)                                              \
    do {                                                              \
        int e_ = expr;                                                \
        if (!e_) {                                                    \
            printf("[%s:%d] %s failed\n", __FILE__, __LINE__, #expr); \
            exit(2);                                                  \
        }                                                             \
    } while (0)

static size_t product(mkldnn_dim_t *arr, size_t size) {
    size_t prod = 1;
    for (size_t i = 0; i < size; ++i)
        prod *= arr[i];
    return prod;
}

typedef struct {
    int nargs;
    mkldnn_exec_arg_t *args;
} args_t;

static void prepare_arg_node(args_t *node, int nargs) {
    node->args = (mkldnn_exec_arg_t *)malloc(sizeof(mkldnn_exec_arg_t) * nargs);
    node->nargs = nargs;
}
static void free_arg_node(args_t *node) {
    free(node->args);
}

static void set_arg(
        mkldnn_exec_arg_t *arg, int arg_idx, mkldnn_memory_t memory) {
    arg->arg = arg_idx;
    arg->memory = memory;
}

static void init_data_memory(uint32_t dim, const mkldnn_dim_t *dims,
        mkldnn_format_tag_t user_tag, mkldnn_data_type_t mkldnn_f32,
        mkldnn_engine_t engine, float *data, mkldnn_memory_t *memory) {
    mkldnn_memory_desc_t user_md;
    CHECK(mkldnn_memory_desc_init_by_tag(
            &user_md, dim, dims, mkldnn_f32, user_tag));
    CHECK(mkldnn_memory_create(memory, &user_md, engine, data));
}

mkldnn_status_t prepare_reorder(mkldnn_memory_t *user_memory, /** in */
        const mkldnn_memory_desc_t *prim_memory_md, /** in */
        mkldnn_engine_t prim_engine, /** in: primitive's engine */
        int dir_is_user_to_prim, /** in: user -> prim or prim -> user */
        mkldnn_memory_t *prim_memory, /** out: primitive's memory created */
        mkldnn_primitive_t *reorder, /** out: reorder primitive created */
        uint32_t *
                net_index, /** primitive index in net (inc if reorder created)
                            */
        mkldnn_primitive_t *net, args_t *net_args /** net params */) {
    const mkldnn_memory_desc_t *user_memory_md;
    mkldnn_memory_get_memory_desc(*user_memory, &user_memory_md);

    mkldnn_engine_t user_mem_engine;
    mkldnn_memory_get_engine(*user_memory, &user_mem_engine);

    if (!mkldnn_memory_desc_equal(user_memory_md, prim_memory_md)) {
        /* memory_create(&p, m, NULL) means allocate memory */
        CHECK(mkldnn_memory_create(prim_memory, prim_memory_md, prim_engine,
                MKLDNN_NATIVE_HANDLE_ALLOCATE));
        mkldnn_primitive_desc_t reorder_pd;
        if (dir_is_user_to_prim) {
            CHECK(mkldnn_reorder_primitive_desc_create(&reorder_pd,
                    user_mem_engine, user_memory_md, prim_engine,
                    prim_memory_md, NULL));
        } else {
            CHECK(mkldnn_reorder_primitive_desc_create(&reorder_pd, prim_engine,
                    prim_memory_md, user_mem_engine, user_memory_md, NULL));
        }
        CHECK(mkldnn_primitive_create(reorder, reorder_pd));
        CHECK(mkldnn_primitive_desc_destroy(reorder_pd));

        net[*net_index] = *reorder;
        prepare_arg_node(&net_args[*net_index], 2);
        set_arg(&net_args[*net_index].args[0], MKLDNN_ARG_FROM,
                dir_is_user_to_prim ? *user_memory : *prim_memory);
        set_arg(&net_args[*net_index].args[1], MKLDNN_ARG_TO,
                dir_is_user_to_prim ? *prim_memory : *user_memory);
        (*net_index)++;
    } else {
        *prim_memory = NULL;
        *reorder = NULL;
    }

    return mkldnn_success;
}

mkldnn_status_t simple_net() {

    mkldnn_engine_t engine;
    CHECK(mkldnn_engine_create(&engine, mkldnn_cpu, 0 /* idx */));

    /* build a simple net */
    uint32_t n = 0;
    mkldnn_primitive_t net[10];
    args_t net_args[10];

    float *net_src
            = (float *)malloc(BATCH * IC * CONV_IH * CONV_IW * sizeof(float));
    float *net_dst
            = (float *)malloc(BATCH * OC * POOL_OH * POOL_OW * sizeof(float));

    /* AlexNet: conv
     * {BATCH, IC, CONV_IH, CONV_IW} (x) {OC, IC, CONV_KH, CONV_KW} ->
     * {BATCH, OC, CONV_OH, CONV_OW}
     * strides: {CONV_STRIDE, CONV_STRIDE}
     */
    mkldnn_dim_t conv_user_src_sizes[4] = { BATCH, IC, CONV_IH, CONV_IW };
    mkldnn_dim_t conv_user_weights_sizes[4] = { OC, IC, 11, 11 };
    mkldnn_dim_t conv_bias_sizes[4] = { OC };
    mkldnn_dim_t conv_user_dst_sizes[4] = { BATCH, OC, CONV_OH, CONV_OW };
    mkldnn_dim_t conv_strides[2] = { CONV_STRIDE, CONV_STRIDE };
    mkldnn_dim_t conv_padding[2] = { CONV_PAD, CONV_PAD };

    float *conv_src = net_src;
    float *conv_weights = (float *)malloc(
            product(conv_user_weights_sizes, 4) * sizeof(float));
    float *conv_bias
            = (float *)malloc(product(conv_bias_sizes, 1) * sizeof(float));

    /* create memory for user data */
    mkldnn_memory_t conv_user_src_memory, conv_user_weights_memory,
            conv_user_bias_memory;
    init_data_memory(4, conv_user_src_sizes, mkldnn_nchw, mkldnn_f32, engine,
            conv_src, &conv_user_src_memory);
    init_data_memory(4, conv_user_weights_sizes, mkldnn_oihw, mkldnn_f32,
            engine, conv_weights, &conv_user_weights_memory);
    init_data_memory(1, conv_bias_sizes, mkldnn_x, mkldnn_f32, engine,
            conv_bias, &conv_user_bias_memory);

    /* create data descriptors for convolution w/ no specified format */

    mkldnn_memory_desc_t conv_src_md, conv_weights_md, conv_bias_md,
            conv_dst_md;
    CHECK(mkldnn_memory_desc_init_by_tag(&conv_src_md, 4, conv_user_src_sizes,
            mkldnn_f32, mkldnn_format_tag_any));
    CHECK(mkldnn_memory_desc_init_by_tag(&conv_weights_md, 4,
            conv_user_weights_sizes, mkldnn_f32, mkldnn_format_tag_any));
    CHECK(mkldnn_memory_desc_init_by_tag(
            &conv_bias_md, 1, conv_bias_sizes, mkldnn_f32, mkldnn_x));
    CHECK(mkldnn_memory_desc_init_by_tag(&conv_dst_md, 4, conv_user_dst_sizes,
            mkldnn_f32, mkldnn_format_tag_any));

    /* create a convolution */
    mkldnn_convolution_desc_t conv_any_desc;
    CHECK(mkldnn_convolution_forward_desc_init(&conv_any_desc, mkldnn_forward,
            mkldnn_convolution_direct, &conv_src_md, &conv_weights_md,
            &conv_bias_md, &conv_dst_md, conv_strides, conv_padding,
            conv_padding, mkldnn_padding_zero));

    mkldnn_primitive_desc_t conv_pd;
    CHECK(mkldnn_primitive_desc_create(
            &conv_pd, &conv_any_desc, NULL, engine, NULL));

    mkldnn_memory_t conv_internal_src_memory, conv_internal_weights_memory,
            conv_internal_dst_memory;

    /* create memory for dst data, we don't need reorder it to user data */
    const mkldnn_memory_desc_t *dst_md
            = mkldnn_primitive_desc_query_md(conv_pd, mkldnn_query_dst_md, 0);
    CHECK(mkldnn_memory_create(&conv_internal_dst_memory, dst_md, engine,
            MKLDNN_NATIVE_HANDLE_ALLOCATE));

    /* create reorder primitives between user data and convolution srcs
     * if required */
    mkldnn_primitive_t conv_reorder_src, conv_reorder_weights;

    const mkldnn_memory_desc_t *src_md
            = mkldnn_primitive_desc_query_md(conv_pd, mkldnn_query_src_md, 0);
    CHECK(prepare_reorder(&conv_user_src_memory, src_md, engine, 1,
            &conv_internal_src_memory, &conv_reorder_src, &n, net, net_args));

    const mkldnn_memory_desc_t *weights_md = mkldnn_primitive_desc_query_md(
            conv_pd, mkldnn_query_weights_md, 0);
    CHECK(prepare_reorder(&conv_user_weights_memory, weights_md, engine, 1,
            &conv_internal_weights_memory, &conv_reorder_weights, &n, net,
            net_args));

    mkldnn_memory_t conv_src_memory = conv_internal_src_memory ?
            conv_internal_src_memory :
            conv_user_src_memory;
    mkldnn_memory_t conv_weights_memory = conv_internal_weights_memory ?
            conv_internal_weights_memory :
            conv_user_weights_memory;

    /* finally create a convolution primitive */
    mkldnn_primitive_t conv;
    CHECK(mkldnn_primitive_create(&conv, conv_pd));
    net[n] = conv;
    prepare_arg_node(&net_args[n], 4);
    set_arg(&net_args[n].args[0], MKLDNN_ARG_SRC, conv_src_memory);
    set_arg(&net_args[n].args[1], MKLDNN_ARG_WEIGHTS, conv_weights_memory);
    set_arg(&net_args[n].args[2], MKLDNN_ARG_BIAS, conv_user_bias_memory);
    set_arg(&net_args[n].args[3], MKLDNN_ARG_DST, conv_internal_dst_memory);
    n++;

    /* AlexNet: relu
     * {BATCH, OC, CONV_OH, CONV_OW} -> {BATCH, OC, CONV_OH, CONV_OW}
     */
    float negative_slope = 1.0f;

    /* create relu memory descriptor on dst memory descriptor
     * from previous primitive */
    const mkldnn_memory_desc_t *relu_src_md
            = mkldnn_primitive_desc_query_md(conv_pd, mkldnn_query_dst_md, 0);

    /* create a relu */
    mkldnn_eltwise_desc_t relu_desc;
    CHECK(mkldnn_eltwise_forward_desc_init(&relu_desc, mkldnn_forward,
            mkldnn_eltwise_relu, relu_src_md, negative_slope, 0));

    mkldnn_primitive_desc_t relu_pd;
    CHECK(mkldnn_primitive_desc_create(
            &relu_pd, &relu_desc, NULL, engine, NULL));

    mkldnn_memory_t relu_dst_memory;
    const mkldnn_memory_desc_t *relu_dst_md
            = mkldnn_primitive_desc_query_md(relu_pd, mkldnn_query_dst_md, 0);
    CHECK(mkldnn_memory_create(&relu_dst_memory, relu_dst_md, engine,
            MKLDNN_NATIVE_HANDLE_ALLOCATE));

    /* finally create a relu primitive */
    mkldnn_primitive_t relu;
    CHECK(mkldnn_primitive_create(&relu, relu_pd));
    net[n] = relu;
    prepare_arg_node(&net_args[n], 2);
    set_arg(&net_args[n].args[0], MKLDNN_ARG_SRC, conv_internal_dst_memory);
    set_arg(&net_args[n].args[1], MKLDNN_ARG_DST, relu_dst_memory);
    n++;

    /* AlexNet: lrn
     * {BATCH, OC, CONV_OH, CONV_OW} -> {BATCH, OC, CONV_OH, CONV_OW}
     * local size: 5
     * alpha: 0.0001
     * beta: 0.75
     */
    uint32_t local_size = 5;
    float alpha = 0.0001f;
    float beta = 0.75f;
    float k = 1.0f;

    /* create lrn memory descriptor on dst memory descriptor
     *  from previous primitive */
    const mkldnn_memory_desc_t *lrn_src_md = relu_dst_md;

    /* create a lrn */
    mkldnn_lrn_desc_t lrn_desc;
    CHECK(mkldnn_lrn_forward_desc_init(&lrn_desc, mkldnn_forward,
            mkldnn_lrn_across_channels, lrn_src_md, local_size, alpha, beta,
            k));

    mkldnn_primitive_desc_t lrn_pd;
    CHECK(mkldnn_primitive_desc_create(&lrn_pd, &lrn_desc, NULL, engine, NULL));

    mkldnn_memory_t lrn_dst_memory;
    const mkldnn_memory_desc_t *lrn_dst_md
            = mkldnn_primitive_desc_query_md(lrn_pd, mkldnn_query_dst_md, 0);
    CHECK(mkldnn_memory_create(&lrn_dst_memory, lrn_dst_md, engine,
            MKLDNN_NATIVE_HANDLE_ALLOCATE));

    mkldnn_memory_t lrn_ws_memory;
    const mkldnn_memory_desc_t *lrn_ws_md = mkldnn_primitive_desc_query_md(
            lrn_pd, mkldnn_query_workspace_md, 0);
    CHECK(mkldnn_memory_create(
            &lrn_ws_memory, lrn_ws_md, engine, MKLDNN_NATIVE_HANDLE_ALLOCATE));

    /* finally create a lrn primitive */
    mkldnn_primitive_t lrn;
    CHECK(mkldnn_primitive_create(&lrn, lrn_pd));
    net[n] = lrn;
    prepare_arg_node(&net_args[n], 3);
    set_arg(&net_args[n].args[0], MKLDNN_ARG_SRC, relu_dst_memory);
    set_arg(&net_args[n].args[1], MKLDNN_ARG_DST, lrn_dst_memory);
    set_arg(&net_args[n].args[2], MKLDNN_ARG_WORKSPACE, lrn_ws_memory);
    n++;

    /* AlexNet: pool
     * {BATCH, OC, CONV_OH, CONV_OW} -> {BATCH, OC, POOL_OH, POOL_OW}
     * kernel: {3, 3}
     * strides: {POOL_STRIDE, POOL_STRIDE}
     */

    mkldnn_dim_t pool_dst_sizes[4] = { BATCH, OC, POOL_OH, POOL_OW };
    mkldnn_dim_t pool_kernel[2] = { 3, 3 };
    mkldnn_dim_t pool_strides[2] = { POOL_STRIDE, POOL_STRIDE };
    mkldnn_dim_t pool_padding[2] = { POOL_PAD, POOL_PAD };

    /* create pooling memory descriptor on dst descriptor
     *  from previous primitive */
    const mkldnn_memory_desc_t *pool_src_md = lrn_dst_md;

    /* create descriptors for dst pooling data */
    mkldnn_memory_desc_t pool_dst_any_md;
    CHECK(mkldnn_memory_desc_init_by_tag(&pool_dst_any_md, 4, pool_dst_sizes,
            mkldnn_f32, mkldnn_format_tag_any));

    /* create memory for user data */
    mkldnn_memory_t pool_user_dst_memory;
    init_data_memory(4, pool_dst_sizes, mkldnn_nchw, mkldnn_f32, engine,
            net_dst, &pool_user_dst_memory);

    /* create a pooling */
    mkldnn_pooling_desc_t pool_desc;
    CHECK(mkldnn_pooling_forward_desc_init(&pool_desc, mkldnn_forward,
            mkldnn_pooling_max, pool_src_md, &pool_dst_any_md, pool_strides,
            pool_kernel, pool_padding, pool_padding, mkldnn_padding_zero));

    mkldnn_primitive_desc_t pool_pd;
    CHECK(mkldnn_primitive_desc_create(
            &pool_pd, &pool_desc, NULL, engine, NULL));

    /* create memory for workspace */
    mkldnn_memory_t pool_ws_memory;
    const mkldnn_memory_desc_t *pool_ws_md = mkldnn_primitive_desc_query_md(
            pool_pd, mkldnn_query_workspace_md, 0);
    CHECK(mkldnn_memory_create(&pool_ws_memory, pool_ws_md, engine,
            MKLDNN_NATIVE_HANDLE_ALLOCATE));

    mkldnn_memory_t pool_dst_memory;

    /* create reorder primitives between user data and pooling dsts
     * if required */
    mkldnn_primitive_t pool_reorder_dst;
    mkldnn_memory_t pool_internal_dst_memory;
    const mkldnn_memory_desc_t *pool_dst_md
            = mkldnn_primitive_desc_query_md(pool_pd, mkldnn_query_dst_md, 0);
    n += 1; /* tentative workaround: preserve space for pooling that should
                                     happen before the reorder */
    CHECK(prepare_reorder(&pool_user_dst_memory, pool_dst_md, engine, 0,
            &pool_internal_dst_memory, &pool_reorder_dst, &n, net, net_args));
    n -= pool_reorder_dst ? 2 : 1;

    pool_dst_memory = pool_internal_dst_memory ? pool_internal_dst_memory :
                                                 pool_user_dst_memory;

    /* finally create a pooling primitive */
    mkldnn_primitive_t pool;
    CHECK(mkldnn_primitive_create(&pool, pool_pd));
    net[n] = pool;
    prepare_arg_node(&net_args[n], 3);
    set_arg(&net_args[n].args[0], MKLDNN_ARG_SRC, lrn_dst_memory);
    set_arg(&net_args[n].args[1], MKLDNN_ARG_DST, pool_dst_memory);
    set_arg(&net_args[n].args[2], MKLDNN_ARG_WORKSPACE, pool_ws_memory);
    n++;

    if (pool_reorder_dst)
        n += 1;

    /*********************/

    mkldnn_stream_t stream;
    CHECK(mkldnn_stream_create(&stream, engine, mkldnn_stream_default_flags));
    for (uint32_t i = 0; i < n; ++i) {
        CHECK(mkldnn_primitive_execute(
                net[i], stream, net_args[i].nargs, net_args[i].args));
    }

    /* clean-up */
    for (uint32_t i = 0; i < n; ++i)
        free_arg_node(&net_args[i]);

    CHECK(mkldnn_primitive_desc_destroy(conv_pd));
    CHECK(mkldnn_primitive_desc_destroy(relu_pd));
    CHECK(mkldnn_primitive_desc_destroy(lrn_pd));
    CHECK(mkldnn_primitive_desc_destroy(pool_pd));

    mkldnn_stream_destroy(stream);

    free(net_src);
    free(net_dst);

    mkldnn_memory_destroy(conv_user_src_memory);
    mkldnn_memory_destroy(conv_user_weights_memory);
    mkldnn_memory_destroy(conv_user_bias_memory);
    mkldnn_memory_destroy(conv_internal_src_memory);
    mkldnn_memory_destroy(conv_internal_weights_memory);
    mkldnn_memory_destroy(conv_internal_dst_memory);
    mkldnn_primitive_destroy(conv_reorder_src);
    mkldnn_primitive_destroy(conv_reorder_weights);
    mkldnn_primitive_destroy(conv);

    free(conv_weights);
    free(conv_bias);

    mkldnn_memory_destroy(relu_dst_memory);
    mkldnn_primitive_destroy(relu);

    mkldnn_memory_destroy(lrn_ws_memory);
    mkldnn_memory_destroy(lrn_dst_memory);
    mkldnn_primitive_destroy(lrn);

    mkldnn_memory_destroy(pool_user_dst_memory);
    mkldnn_memory_destroy(pool_internal_dst_memory);
    mkldnn_memory_destroy(pool_ws_memory);
    mkldnn_primitive_destroy(pool_reorder_dst);
    mkldnn_primitive_destroy(pool);

    mkldnn_engine_destroy(engine);

    return mkldnn_success;
}

int main(int argc, char **argv) {
    mkldnn_status_t result = simple_net();
    printf("%s\n", (result == mkldnn_success) ? "passed" : "failed");
    return result;
}
