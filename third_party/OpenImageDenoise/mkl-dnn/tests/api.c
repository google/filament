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

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

#include "mkldnn.h"

#define CHECK(f) do { \
    mkldnn_status_t s = f; \
    if (s != mkldnn_success) { \
        printf("[%s:%d] error: %s returns %d\n", __FILE__, __LINE__, #f, s); \
        exit(2); \
    } \
} while(0)

#define CHECK_TRUE(expr) do { \
    int e_ = expr; \
    if (!e_) { \
        printf("[%s:%d] %s failed\n", __FILE__, __LINE__, #expr); \
        exit(2); \
    } \
} while(0)

static size_t product(mkldnn_dim_t *arr, size_t size) {
    size_t prod = 1;
    for (size_t i = 0; i < size; ++i) prod *= arr[i];
    return prod;
}

typedef float real_t;

#define LENGTH_100 100

void test1() {
    mkldnn_engine_t engine;
    CHECK(mkldnn_engine_create(&engine, mkldnn_cpu, 0));

    mkldnn_dims_t dims = { LENGTH_100 };
    real_t data[LENGTH_100];

    mkldnn_memory_desc_t md;
    const mkldnn_memory_desc_t *c_md_tmp;
    mkldnn_memory_t m;

    CHECK(mkldnn_memory_desc_init_by_tag(&md, 1, dims, mkldnn_f32, mkldnn_x));
    CHECK(mkldnn_memory_create(&m, &md, engine, NULL));

    void *req = NULL;

    CHECK(mkldnn_memory_get_data_handle(m, &req));
    CHECK_TRUE(req == NULL);
    CHECK(mkldnn_memory_set_data_handle(m, data));
    CHECK(mkldnn_memory_get_data_handle(m, &req));
    CHECK_TRUE(req == data);

    CHECK_TRUE(mkldnn_memory_desc_get_size(&md) == LENGTH_100 * sizeof(data[0]));

    CHECK(mkldnn_memory_get_memory_desc(m, &c_md_tmp));
    CHECK_TRUE(mkldnn_memory_desc_equal(&md, c_md_tmp));

    CHECK(mkldnn_memory_destroy(m));

    CHECK(mkldnn_engine_destroy(engine));
}

void test2() {
    /* AlexNet: c3
     * {2, 256, 13, 13} (x) {384, 256, 3, 3} -> {2, 384, 13, 13}
     * pad: {1, 1}
     * strides: {1, 1}
     */

    const mkldnn_dim_t mb = 2;
    const mkldnn_dim_t groups = 2;
    mkldnn_dim_t c3_src_sizes[4] = {mb, 256, 13, 13};
    mkldnn_dim_t c3_weights_sizes[] = {groups, 384/groups, 256/groups, 3, 3};
    mkldnn_dim_t c3_bias_sizes[1] = {384};
    mkldnn_dim_t strides[] = {1, 1};
    mkldnn_dim_t padding[] = {0, 0}; // set proper values
    mkldnn_dim_t c3_dst_sizes[4] = {mb, 384,
        (c3_src_sizes[2] + 2*padding[0] - c3_weights_sizes[3])/strides[0] + 1,
        (c3_src_sizes[3] + 2*padding[1] - c3_weights_sizes[4])/strides[1] + 1
    };

    real_t *src = (real_t*)calloc(product(c3_src_sizes, 4), sizeof(real_t));
    real_t *weights = (real_t*)calloc(product(c3_weights_sizes, 5), sizeof(real_t));
    real_t *bias = (real_t*)calloc(product(c3_bias_sizes, 1), sizeof(real_t));
    real_t *dst = (real_t*)calloc(product(c3_dst_sizes, 4), sizeof(real_t));
    real_t *out_mem = (real_t*)calloc(product(c3_dst_sizes, 4), sizeof(real_t));
    CHECK_TRUE(src && weights && bias && dst && out_mem);

    for (mkldnn_dim_t i = 0; i < c3_bias_sizes[0]; ++i) bias[i] = i;

    mkldnn_engine_t engine;
    CHECK(mkldnn_engine_create(&engine, mkldnn_cpu, 0));

    mkldnn_stream_t stream;
    CHECK(mkldnn_stream_create(&stream, engine, mkldnn_stream_default_flags));

    /* first describe user data and create data descriptors for future
     * convolution w/ the specified format -- we do not want to do a reorder */
    mkldnn_memory_desc_t c3_src_md, c3_weights_md, c3_bias_md, c3_dst_md, out_md;
    mkldnn_memory_t c3_src, c3_weights, c3_bias, c3_dst, out;

    // src
    {
        CHECK(mkldnn_memory_desc_init_by_tag(&c3_src_md, 4, c3_src_sizes, mkldnn_f32, mkldnn_nChw8c));
        CHECK(mkldnn_memory_create(&c3_src, &c3_src_md, engine, src));
    }

    // weights
    {
        CHECK(mkldnn_memory_desc_init_by_tag(&c3_weights_md, 4 + (groups != 1),
                    c3_weights_sizes + (groups == 1), mkldnn_f32,
                    groups == 1 ? mkldnn_OIhw8i8o : mkldnn_gOIhw8i8o));
        CHECK(mkldnn_memory_create(&c3_weights, &c3_weights_md, engine, weights));
    }

    // bias
    {
        CHECK(mkldnn_memory_desc_init_by_tag(&c3_bias_md, 1, c3_bias_sizes, mkldnn_f32, mkldnn_x));
        CHECK(mkldnn_memory_create(&c3_bias, &c3_bias_md, engine, bias));
    }

    // c3_dst
    {
        CHECK(mkldnn_memory_desc_init_by_tag(&c3_dst_md, 4, c3_dst_sizes, mkldnn_f32, mkldnn_nChw8c));
        CHECK(mkldnn_memory_create(&c3_dst, &c3_dst_md, engine, dst));
    }

    // out
    {
        CHECK(mkldnn_memory_desc_init_by_tag(&out_md, 4, c3_dst_sizes, mkldnn_f32, mkldnn_nchw));
        CHECK(mkldnn_memory_create(&out, &out_md, engine, out_mem));
    }

    /* create a convolution primitive descriptor */
    mkldnn_convolution_desc_t c3_desc;
    mkldnn_primitive_desc_t c3_pd;
    mkldnn_primitive_t c3;

    CHECK(mkldnn_convolution_forward_desc_init(&c3_desc,
                mkldnn_forward_training, mkldnn_convolution_direct,
                &c3_src_md, &c3_weights_md, &c3_bias_md, &c3_dst_md,
                strides, padding, NULL, mkldnn_padding_zero));
    CHECK(mkldnn_primitive_desc_create(&c3_pd, &c3_desc, NULL, engine, NULL));

    CHECK_TRUE(mkldnn_memory_desc_equal(&c3_src_md,
                mkldnn_primitive_desc_query_md(c3_pd, mkldnn_query_src_md, 0)));
    CHECK_TRUE(mkldnn_memory_desc_equal(&c3_weights_md,
                mkldnn_primitive_desc_query_md(c3_pd, mkldnn_query_weights_md, 0)));
    CHECK_TRUE(mkldnn_memory_desc_equal(&c3_bias_md,
                mkldnn_primitive_desc_query_md(c3_pd, mkldnn_query_weights_md, 1)));
    CHECK_TRUE(mkldnn_memory_desc_equal(&c3_dst_md,
                mkldnn_primitive_desc_query_md(c3_pd, mkldnn_query_dst_md, 0)));

    /* create a convolution and execute it */
    CHECK(mkldnn_primitive_create(&c3, c3_pd));
    CHECK(mkldnn_primitive_desc_destroy(c3_pd));

    mkldnn_exec_arg_t c3_args[4] = {
        {MKLDNN_ARG_SRC, c3_src},
        {MKLDNN_ARG_WEIGHTS, c3_weights},
        {MKLDNN_ARG_BIAS, c3_bias},
        {MKLDNN_ARG_DST, c3_dst},
    };
    CHECK(mkldnn_primitive_execute(c3, stream, 4, c3_args));
    CHECK(mkldnn_primitive_destroy(c3));

    /* create a reorder primitive descriptor */
    mkldnn_primitive_desc_t r_pd;
    CHECK(mkldnn_reorder_primitive_desc_create(
                &r_pd, engine, &c3_dst_md, engine, &out_md, NULL));

    /* create a reorder and execute it */
    mkldnn_primitive_t r;
    CHECK(mkldnn_primitive_create(&r, r_pd));
    CHECK(mkldnn_primitive_desc_destroy(r_pd));

    mkldnn_exec_arg_t r_args[2] = {
        {MKLDNN_ARG_FROM, c3_dst},
        {MKLDNN_ARG_TO, out},
    };
    CHECK(mkldnn_primitive_execute(r, stream, 2, r_args));
    CHECK(mkldnn_primitive_destroy(r));

    /* clean-up */
    CHECK(mkldnn_memory_destroy(c3_src));
    CHECK(mkldnn_memory_destroy(c3_weights));
    CHECK(mkldnn_memory_destroy(c3_bias));
    CHECK(mkldnn_memory_destroy(c3_dst));
    CHECK(mkldnn_memory_destroy(out));
    CHECK(mkldnn_stream_destroy(stream));
    CHECK(mkldnn_engine_destroy(engine));

    const mkldnn_dim_t N = c3_dst_sizes[0], C = c3_dst_sizes[1],
          H = c3_dst_sizes[2], W = c3_dst_sizes[3];
    for (mkldnn_dim_t n = 0; n < N; ++n)
    for (mkldnn_dim_t c = 0; c < C; ++c)
    for (mkldnn_dim_t h = 0; h < H; ++h)
    for (mkldnn_dim_t w = 0; w < W; ++w)
    {
        mkldnn_dim_t off = ((n*C + c)*H + h)*W + w;
        CHECK_TRUE(out_mem[off] == bias[c]);
    }

    free(src);
    free(weights);
    free(bias);
    free(dst);
    free(out_mem);
}

void test3() {
    const mkldnn_dim_t mb = 2;
    mkldnn_dim_t l2_data_sizes[4] = {mb, 256, 13, 13};

    real_t *src = (real_t*)calloc(product(l2_data_sizes, 4), sizeof(real_t));
    real_t *dst = (real_t*)calloc(product(l2_data_sizes, 4), sizeof(real_t));
    CHECK_TRUE(src && dst);

    for (size_t i = 0; i < product(l2_data_sizes, 4); ++i)
        src[i] = (i % 13) + 1;

    mkldnn_engine_t engine;
    CHECK(mkldnn_engine_create(&engine, mkldnn_cpu, 0));

    mkldnn_stream_t stream;
    CHECK(mkldnn_stream_create(&stream, engine, mkldnn_stream_default_flags));

    mkldnn_memory_desc_t l2_data_md;
    mkldnn_memory_t l2_src, l2_dst;

    // src, dst
    {
        CHECK(mkldnn_memory_desc_init_by_tag(
                &l2_data_md, 4, l2_data_sizes, mkldnn_f32, mkldnn_nchw));
        CHECK(mkldnn_memory_create(&l2_src, &l2_data_md, engine, src));
        CHECK(mkldnn_memory_create(&l2_dst, &l2_data_md, engine, dst));
    }

    /* create an lrn */
    mkldnn_lrn_desc_t l2_desc;
    mkldnn_primitive_desc_t l2_pd;
    mkldnn_primitive_t l2;

    CHECK(mkldnn_lrn_forward_desc_init(&l2_desc,
                mkldnn_forward_inference, mkldnn_lrn_across_channels,
                &l2_data_md, 5, 1e-4, 0.75, 1.0));
    CHECK(mkldnn_primitive_desc_create(&l2_pd, &l2_desc, NULL, engine, NULL));

    CHECK_TRUE(mkldnn_memory_desc_equal(&l2_data_md,
                mkldnn_primitive_desc_query_md(l2_pd, mkldnn_query_src_md, 0)));
    CHECK_TRUE(mkldnn_memory_desc_equal(&l2_data_md,
                mkldnn_primitive_desc_query_md(l2_pd, mkldnn_query_dst_md, 0)));
    CHECK_TRUE(mkldnn_primitive_desc_query_s32(
                l2_pd, mkldnn_query_num_of_inputs_s32, 0) == 1);
    CHECK_TRUE(mkldnn_primitive_desc_query_s32(
                l2_pd, mkldnn_query_num_of_outputs_s32, 0) == 1);

    CHECK(mkldnn_primitive_create(&l2, l2_pd));
    CHECK(mkldnn_primitive_desc_destroy(l2_pd));

    mkldnn_exec_arg_t l2_args[2] = {
        {MKLDNN_ARG_SRC, l2_src},
        {MKLDNN_ARG_DST, l2_dst},
    };
    CHECK(mkldnn_primitive_execute(l2, stream, 2, l2_args));
    CHECK(mkldnn_primitive_destroy(l2));

    /* clean-up */
    CHECK(mkldnn_memory_destroy(l2_src));
    CHECK(mkldnn_memory_destroy(l2_dst));
    CHECK(mkldnn_stream_destroy(stream));
    CHECK(mkldnn_engine_destroy(engine));

    const mkldnn_dim_t N = l2_data_sizes[0], C = l2_data_sizes[1],
          H = l2_data_sizes[2], W = l2_data_sizes[3];
    for (mkldnn_dim_t n = 0; n < N; ++n)
    for (mkldnn_dim_t c = 0; c < C; ++c)
    for (mkldnn_dim_t h = 0; h < H; ++h)
    for (mkldnn_dim_t w = 0; w < W; ++w)
    {
        size_t off = ((n*C + c)*H + h)*W + w;
        real_t e = (off % 13) + 1;
        real_t diff = (real_t)fabs(dst[off] - e);
        if (diff/fabs(e) > 0.0125)
            printf("exp: %g, got: %g\n", e, dst[off]);
        CHECK_TRUE(diff/fabs(e) < 0.0125);
    }

    free(src);
    free(dst);
}

int main() {
    test1();
    test2();
    test3();
    return 0;
}
