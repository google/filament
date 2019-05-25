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

#include "gtest/gtest.h"
#include "mkldnn_test_common.hpp"

#include "mkldnn.hpp"

namespace mkldnn {

template <typename data_t>
void check_softmax_fwd(prop_kind aprop_kind, memory &src, memory &dst, int axis)
{
    data_t *dst_ptr = (data_t *)dst.get_data_handle();

    const memory::desc dst_pd = dst.get_desc();
    const mkldnn::impl::memory_desc_wrapper dst_mdw(dst_pd.data);

    ASSERT_EQ(dst_mdw.data_type(),
            memory::data_type::f32); // TODO: type assert

    float result = 0.0f;
    // Worst case error bound on naive summation
    // algorithm is on the order of n*machine_precision.
    // See e.g. N. J. Higham. Accuracy and stability of numerical algorithms.
    //     SIAM Publications, Philadelphia, 2nd edition, 2002.
    // So below tests will use error bound dependent
    // on the number of elements in reduction.
    const float eps = std::numeric_limits<float>::epsilon();

    memory::dim MB = dst_pd.data.dims[0];
    memory::dim C = dst_pd.data.dims[1];
    if (MB*C == 0) return;

    if (dst_pd.data.ndims == 2) {
        if (axis == 1) {
            for (memory::dim n = 0; n < MB; ++n) {
                result = 0.0f;

                for (memory::dim c = 0; c < C; ++c) {
                    result += dst_ptr[dst_mdw.off_l(n * C + c, true)];
                }
                EXPECT_NEAR(result, 1.0, eps*C);
            }
        }
        else if (axis == 0) {
            for (memory::dim c = 0; c < C; ++c) {
                result = 0.0f;

                for (memory::dim n = 0; n < MB; ++n) {
                    result += dst_ptr[dst_mdw.off_l(n * C + c, true)];
                }
                EXPECT_NEAR(result, 1.0, eps*MB);
            }
        }
    } else {
        memory::dim H = dst_pd.data.dims[2];
        memory::dim W = dst_pd.data.dims[3];
        if (H*W == 0) return;

        auto off = [=](memory::dim n, memory::dim c, memory::dim h, memory::dim w)
        {
            return ((size_t)n * W * H * C + (size_t)c * W * H + (size_t)h * W + w);
        };

        if (axis == 0) {
            for (memory::dim c = 0; c < C; ++c)
            for (memory::dim h = 0; h < H; ++h)
            for (memory::dim w = 0; w < W; ++w)
            {
                result = 0.0f;

                for (memory::dim n = 0; n < MB; ++n) {
                    result += dst_ptr[dst_mdw.off_l(off(n, c, h, w), true)];
                }
                EXPECT_NEAR(result, 1.0, eps*MB);
            }
        } else if (axis == 1) {
            for (memory::dim n = 0; n < MB; ++n)
            for (memory::dim h = 0; h < H; ++h)
            for (memory::dim w = 0; w < W; ++w)
            {
                result = 0.0f;

                for (memory::dim c = 0; c < C; ++c) {
                    result += dst_ptr[dst_mdw.off_l(off(n, c, h, w), true)];
                }
                EXPECT_NEAR(result, 1.0, eps*C);
            }
        } else if (axis == 2) {
            for (memory::dim n = 0; n < MB; ++n)
            for (memory::dim c = 0; c < C; ++c)
            for (memory::dim w = 0; w < W; ++w)
            {
                result = 0.0f;

                for (memory::dim h = 0; h < H; ++h) {
                    result += dst_ptr[dst_mdw.off_l(off(n, c, h, w), true)];
                }
                EXPECT_NEAR(result, 1.0, eps*H);
            }
        } else if (axis == 3) {
            for (memory::dim n = 0; n < MB; ++n)
            for (memory::dim c = 0; c < C; ++c)
            for (memory::dim h = 0; h < H; ++h)
            {
                result = 0.0f;

                for (memory::dim w = 0; w < W; ++w) {
                    result += dst_ptr[dst_mdw.off_l(off(n, c, h, w), true)];
                }
                EXPECT_NEAR(result, 1.0, eps*W);
            }
        }
    }
}

template <typename data_t>
struct softmax_test_params {
    prop_kind aprop_kind;
    engine::kind engine_kind;
    memory::format_tag memory_format;
    memory::dims dims;
    int axis;
    bool expect_to_fail;
    mkldnn_status_t expected_status;
};

template <typename data_t>
class softmax_test : public ::testing::TestWithParam<softmax_test_params<data_t>> {
    softmax_test_params<data_t> p;
protected:
    virtual void SetUp() {
        p = ::testing::TestWithParam<softmax_test_params<data_t>>::GetParam();
        catch_expected_failures([=](){Test();}, p.expect_to_fail,
                    p.expected_status);
    }

    void Test() {
        ASSERT_TRUE(p.engine_kind == engine::kind::cpu);
        ASSERT_TRUE(p.aprop_kind == prop_kind::forward_training
                    || p.aprop_kind == prop_kind::forward_scoring
                    || p.aprop_kind == prop_kind::forward_inference);
        auto eng = engine(p.engine_kind, 0);
        auto strm = stream(eng);

        memory::data_type prec = data_traits<data_t>::data_type;

        auto mem_desc = memory::desc(p.dims, prec, p.memory_format);

        auto src = memory(mem_desc, eng);
        auto dst = memory(mem_desc, eng);

        auto softmax_desc = softmax_forward::desc(p.aprop_kind, mem_desc,
                    p.axis);
        auto softmax_prim_desc
            = softmax_forward::primitive_desc(softmax_desc, eng);
        auto softmax = softmax_forward(softmax_prim_desc);

        auto test_with_given_fill = [&](data_t mean, data_t var) {
            fill_data<data_t>(mem_desc.get_size() / sizeof(data_t),
                    (data_t *)src.get_data_handle(), mean, var);

            softmax.execute(strm, {{MKLDNN_ARG_SRC, src}, {MKLDNN_ARG_DST, dst}});
            check_softmax_fwd<data_t>(p.aprop_kind, src, dst, p.axis);
        };

        test_with_given_fill(-50, 50);
        test_with_given_fill(-200, 1);
        test_with_given_fill(   0, 1);
        test_with_given_fill( 200, 1);
    }
};

using softmax_forward_test_float = softmax_test<float>;
using softmax_fwd_test_params_float = softmax_test_params<float>;

TEST_P(softmax_forward_test_float, TestsSoftmax) { }
INSTANTIATE_TEST_SUITE_P(TestSoftmaxForward, softmax_forward_test_float,
        ::testing::Values(
            softmax_fwd_test_params_float{prop_kind::forward_scoring,
            engine::kind::cpu, memory::format_tag::nchw, {2, -2, 128, 256}, 0,
            true, mkldnn_invalid_arguments},
            softmax_fwd_test_params_float{prop_kind::forward_scoring,
            engine::kind::cpu, memory::format_tag::nchw, {2, 2, 128, 256}, 5,
            true, mkldnn_invalid_arguments},
            softmax_fwd_test_params_float{prop_kind::forward_scoring,
            engine::kind::cpu, memory::format_tag::nchw, {2, 0, 5, 5}, 0},
            softmax_fwd_test_params_float{prop_kind::forward_scoring,
            engine::kind::cpu, memory::format_tag::nchw, {2, 0, 5, 5}, 1},
            softmax_fwd_test_params_float{prop_kind::forward_scoring,
            engine::kind::cpu, memory::format_tag::nchw, {2, 19, 128, 256}, 0},
            softmax_fwd_test_params_float{prop_kind::forward_scoring,
            engine::kind::cpu, memory::format_tag::nchw, {2, 19, 128, 256}, 1},
            softmax_fwd_test_params_float{prop_kind::forward_scoring,
            engine::kind::cpu, memory::format_tag::nchw, {2, 19, 128, 256}, 2},
            softmax_fwd_test_params_float{prop_kind::forward_scoring,
            engine::kind::cpu, memory::format_tag::nchw, {1, 8, 1024, 16}, 2},
            softmax_fwd_test_params_float{prop_kind::forward_scoring,
            engine::kind::cpu, memory::format_tag::nchw, {2, 19, 128, 256}, 3},
            softmax_fwd_test_params_float{prop_kind::forward_scoring,
            engine::kind::cpu, memory::format_tag::nc, {2, 1000}, 0},
            softmax_fwd_test_params_float{prop_kind::forward_scoring,
            engine::kind::cpu, memory::format_tag::nc, {2, 1000}, 1},
            softmax_fwd_test_params_float{prop_kind::forward_scoring,
            engine::kind::cpu, memory::format_tag::nc, {1, 256}, 1},
            softmax_fwd_test_params_float{prop_kind::forward_scoring,
            engine::kind::cpu, memory::format_tag::nc, {1, 13}, 1}));
}
