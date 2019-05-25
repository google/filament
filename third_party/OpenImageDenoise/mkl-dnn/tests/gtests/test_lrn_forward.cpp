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

#include <cmath>

#include "mkldnn_test_common.hpp"
#include "gtest/gtest.h"

#include "mkldnn.hpp"

namespace mkldnn {

enum {ACROSS=0,WITHIN=1};

struct test_lrn_desc_t {
    memory::dim mb, c;
    memory::dim h, w;
    memory::dim local_size;
    float alpha, beta, k;
    int kind; // 0 ac, 1 wc
};

template <typename data_t>
void check_lrn_fwd(const test_lrn_desc_t &ld,
        const memory::desc &src_d, const memory::desc &dst_d,
        const memory &src, const memory &dst)
{
    data_t *src_ptr = (data_t *)src.get_data_handle();
    data_t *dst_ptr = (data_t *)dst.get_data_handle();

    const memory::dim C = ld.c;
    const memory::dim H = ld.h;
    const memory::dim W = ld.w;
    const memory::dim size = ld.local_size;
    const memory::dim CSIZE = ld.kind == ACROSS ? size : 1;
    const memory::dim HWSIZE = size + 1 - CSIZE;
    const memory::dim summands = ld.kind == ACROSS ? size : size*size;
    const auto padded_c = src.get_desc().data.padded_dims[1];

    const mkldnn::impl::memory_desc_wrapper src_mdw(src_d.data);
    const mkldnn::impl::memory_desc_wrapper dst_mdw(dst_d.data);

    auto off = [=](memory::dim n, memory::dim c, memory::dim h, memory::dim w)
    { return ((n * padded_c + c) * ld.h + h) * ld.w + w; };

    auto ker = [=](data_t *d, memory::dim n, memory::dim oc, memory::dim oh,
            memory::dim ow)
    {
        data_t sum = 0.0;
        for (memory::dim c = oc; c < oc + CSIZE; ++c) {
            if (c < (CSIZE - 1) / 2) continue;
            if (c >= C + (CSIZE - 1) / 2) continue;
            for (memory::dim h = oh; h < oh + HWSIZE; ++h) {
                if (h < (HWSIZE - 1) / 2) continue;
                if (h >= H + (HWSIZE - 1) / 2) continue;
                for (memory::dim w = ow; w < ow + HWSIZE; ++w) {
                    if (w < (HWSIZE - 1) / 2) continue;
                    if (w >= W + (HWSIZE - 1) / 2) continue;
                    data_t s = src_ptr[src_mdw.off_l(off(n, c - (CSIZE - 1) / 2, h - (HWSIZE - 1) / 2, w - (HWSIZE - 1) / 2), true)];
                    sum += s * s;
                }
            }
        }
        data_t norm_coef = powf(static_cast<float>(ld.k + ld.alpha * sum / summands),
                                static_cast<float>(ld.beta));
        data_t ref_out = src_ptr[src_mdw.off_l(off(n, oc, oh, ow), true)]/norm_coef;
        data_t eps = static_cast<data_t>(1.e-7f*(2*summands+5));
        data_t out = d[0];
        data_t norm_max = std::max(fabs(out), fabs(ref_out));
        if (norm_max < eps) norm_max = 1.;
        EXPECT_NEAR(out, ref_out, eps*norm_max);
    };

    const memory::dim N = ld.mb;
    mkldnn::impl::parallel_nd(N, padded_c, H, W,
        [&](memory::dim n, memory::dim c, memory::dim h, memory::dim w)
        { ker(&dst_ptr[dst_mdw.off_l(off(n, c, h, w), true)], n, c, h, w); }
    );
}

struct lrn_fwd_test_params {
    prop_kind aprop_kind;
    engine::kind engine_kind;
    algorithm aalgorithm;
    memory::format_tag src_format;
    memory::format_tag dst_format;
    test_lrn_desc_t test_ld;
    bool expect_to_fail;
    mkldnn_status_t expected_status;
};

template <typename data_t>
class lrn_forward_test : public ::testing::TestWithParam<lrn_fwd_test_params> {
    lrn_fwd_test_params p;

protected:
    virtual void SetUp() {
        p = ::testing::TestWithParam<decltype(p)>::GetParam();
        catch_expected_failures([=](){Test();}, p.expect_to_fail,
                    p.expected_status);
    }

    void Test() {
        ASSERT_TRUE(p.engine_kind == engine::kind::cpu);
        ASSERT_TRUE(p.aprop_kind == prop_kind::forward_training
                || p.aprop_kind == prop_kind::forward_scoring);
        auto eng = engine(p.engine_kind, 0);
        auto strm = stream(eng);
        memory::data_type data_type = data_traits<data_t>::data_type;
        ASSERT_EQ(data_type, mkldnn::memory::data_type::f32);

        test_lrn_desc_t ld = p.test_ld;
        bool with_workspace = p.aprop_kind == prop_kind::forward_training;

        auto l_src_desc = create_md({ ld.mb, ld.c, ld.h, ld.w },
                data_type, p.src_format);
        auto l_dst_desc = create_md({ ld.mb, ld.c, ld.h, ld.w },
                data_type, p.dst_format);

        auto l_src = test_memory(l_src_desc, eng);
        auto l_dst = test_memory(l_dst_desc, eng);

        // Only true for dense format
        fill_data<data_t>(l_src.get_size() / sizeof(data_t),
                (data_t *)l_src.get().get_data_handle());
        fill_data<data_t>(l_dst.get_size() / sizeof(data_t),
                (data_t *)l_dst.get().get_data_handle());
        check_zero_tail<data_t>(1, l_src.get());
        check_zero_tail<data_t>(1, l_dst.get());

        auto lrn_desc = lrn_forward::desc(p.aprop_kind, p.aalgorithm,
                l_src_desc, ld.local_size, ld.alpha, ld.beta, ld.k);
        auto lrn_prim_desc = lrn_forward::primitive_desc(lrn_desc, eng);

        std::shared_ptr<memory> workspace;

        // Execute
        auto l = lrn_forward(lrn_prim_desc);
        std::unordered_map<int, memory> args = {
            {MKLDNN_ARG_SRC, l_src.get()},
            {MKLDNN_ARG_DST, l_dst.get()}
        };
        if (with_workspace) {
            auto workspace_md = lrn_prim_desc.workspace_desc();
            workspace.reset(new memory(workspace_md, eng));
            args.insert({MKLDNN_ARG_WORKSPACE, *workspace});
        }
        l.execute(strm, args);

        check_zero_tail<data_t>(0, l_dst.get());

        check_lrn_fwd<data_t>(ld, l_src_desc, l_dst_desc, l_src.get(),
                l_dst.get());
    }
};

using lrn_forward_test_float = lrn_forward_test<float>;
using lrn_fwd_test_params_float = lrn_fwd_test_params;

TEST_P(lrn_forward_test_float, TestsLRN)
{
}

INSTANTIATE_TEST_SUITE_P(TestLRNForwardZeroDim, lrn_forward_test_float,
        ::testing::Values(
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 0, 10, 4, 4, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS }}
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, { 2, 0, 4, 4, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS }}
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nChw16c, { 2, 16, 0, 4, 5, 1.0e-4f, 0.75f, 3.0f, ACROSS }}
            ));

INSTANTIATE_TEST_SUITE_P(TestLRNForwardEF, lrn_forward_test_float,
        ::testing::Values(
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { -1, 10, 4, 4, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS },
            true, mkldnn_invalid_arguments }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 2, -10, 4, 4, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS },
            true, mkldnn_invalid_arguments }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 2, 10, -4, 4, 5, 1.0e-4f, 0.75f, 3.0f, ACROSS },
            true, mkldnn_invalid_arguments }
            ));

INSTANTIATE_TEST_SUITE_P(TestLRNForward_nChw16c_padded, lrn_forward_test_float,
        ::testing::Values(
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, { 2, 17, 4, 4, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, { 2, 19, 4, 4, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, { 2, 26, 4, 4, 5, 1.0e-4f, 0.75f, 5.7f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, { 2, 12, 4, 4, 5, 1.0e-4f, 0.75f, 5.7f, ACROSS } }
            ));

INSTANTIATE_TEST_SUITE_P(TestLRNForward_nChw8c_padded, lrn_forward_test_float,
        ::testing::Values(
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 7, 4, 4, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 9, 4, 4, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 26, 4, 4, 5, 1.0e-4f, 0.75f, 5.7f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 12, 4, 4, 5, 1.0e-4f, 0.75f, 5.7f, ACROSS } }
            ));

INSTANTIATE_TEST_SUITE_P(TestLRNForward, lrn_forward_test_float,
        ::testing::Values(
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 2, 10, 4, 4, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 2, 10, 4, 4, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 2, 10, 4, 4, 5, 1.0e-4f, 0.75f, 3.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 2, 10, 4, 4, 5, 1.0e-4f, 0.75f, 3.0f, ACROSS } }
            ));

INSTANTIATE_TEST_SUITE_P(TestLRNForwardNHWC, lrn_forward_test_float,
        ::testing::Values(
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nhwc,
            memory::format_tag::nhwc, { 2, 10, 4, 4, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nhwc,
            memory::format_tag::nhwc, { 2, 10, 4, 4, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nhwc,
            memory::format_tag::nhwc, { 2, 10, 4, 4, 5, 1.0e-4f, 0.75f, 4.85f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nhwc,
            memory::format_tag::nhwc, { 2, 10, 4, 4, 5, 1.0e-4f, 0.75f, 4.85f, ACROSS } }
            ));

INSTANTIATE_TEST_SUITE_P(TestLRNForward_nChw8c, lrn_forward_test_float,
        ::testing::Values(
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 16, 4, 4, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 16, 4, 4, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 16, 4, 4, 5, 1.0e-4f, 0.75f, 5.7f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 16, 4, 4, 5, 1.0e-4f, 0.75f, 5.7f, ACROSS } }
            ));

INSTANTIATE_TEST_SUITE_P(TestLRNForward_nChw16c, lrn_forward_test_float,
        ::testing::Values(
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, { 2, 16, 4, 4, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, { 2, 16, 4, 4, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, { 2, 16, 4, 4, 5, 1.0e-4f, 0.75f, 5.7f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, { 2, 16, 4, 4, 5, 1.0e-4f, 0.75f, 5.7f, ACROSS } }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestLRNAlexnetForwardNCHW, lrn_forward_test_float,
        ::testing::Values(
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 2, 96, 55, 55, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 2, 96, 55, 55, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 2, 256, 27, 27, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 2, 256, 27, 27, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestLRNAlexnetForwardNHWC, lrn_forward_test_float,
        ::testing::Values(
                lrn_fwd_test_params_float{ prop_kind::forward_training,
                engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nhwc,
                memory::format_tag::nhwc, { 2, 96, 55, 55, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
                lrn_fwd_test_params_float{ prop_kind::forward_scoring,
                engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nhwc,
                memory::format_tag::nhwc, { 2, 96, 55, 55, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
                lrn_fwd_test_params_float{ prop_kind::forward_training,
                engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nhwc,
                memory::format_tag::nhwc, { 2, 256, 27, 27, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
                lrn_fwd_test_params_float{ prop_kind::forward_scoring,
                engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nhwc,
                memory::format_tag::nhwc, { 2, 256, 27, 27, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestLRNAlexnetForward_nChw8c, lrn_forward_test_float,
        ::testing::Values(
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 96, 55, 55, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
            lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 96, 55, 55, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 256, 27, 27, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
            lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 256, 27, 27, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestLRNAlexnetForward_nChw16c, lrn_forward_test_float,
        ::testing::Values(
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, { 2, 96, 55, 55, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
            lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, { 2, 96, 55, 55, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, { 2, 256, 27, 27, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
            lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, { 2, 256, 27, 27, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestLRNGoogleNetV1ForwardNCHW, lrn_forward_test_float,
        ::testing::Values(
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 2, 64, 56, 56, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
            lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 2, 64, 56, 56, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 2, 192, 56, 56, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
            lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 2, 192, 56, 56, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestLRNGoogleNetV1Forward_nChw8c, lrn_forward_test_float,
        ::testing::Values(
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 64, 56, 56, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
            lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 64, 56, 56, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 192, 56, 56, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
            lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 192, 56, 56, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestLRNGoogleNetV1Forward_nChw16c, lrn_forward_test_float,
        ::testing::Values(
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, { 2, 64, 56, 56, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
            lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, { 2, 64, 56, 56, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, { 2, 192, 56, 56, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } },
            lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, { 2, 192, 56, 56, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestLRNRCNNForwardBlocked, lrn_forward_test_float,
        ::testing::Values(
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_within_channel, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 96, 55, 55, 3, 1.0e-4f, 0.75f, 1.0f, WITHIN } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_within_channel, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 96, 55, 55, 3, 1.0e-4f, 0.75f, 1.0f, WITHIN } }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_within_channel, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 256, 27, 27, 3, 1.0e-4f, 0.75f, 1.0f, WITHIN } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_within_channel, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 256, 27, 27, 3, 1.0e-4f, 0.75f, 1.0f, WITHIN } }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_within_channel, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 96, 55, 55, 5, 1.0e-4f, 0.75f, 1.0f, WITHIN } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_within_channel, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 96, 55, 55, 5, 1.0e-4f, 0.75f, 1.0f, WITHIN } }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_within_channel, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 256, 27, 27, 5, 1.0e-4f, 0.75f, 1.0f, WITHIN } }
            , lrn_fwd_test_params_float{ prop_kind::forward_scoring,
            engine::kind::cpu, algorithm::lrn_within_channel, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, { 2, 256, 27, 27, 5, 1.0e-4f, 0.75f, 1.0f, WITHIN } }
            ));

// This tests compatibility with MKL-DNN 0.14
INSTANTIATE_TEST_SUITE_P(
        TestLRNRegressionWeightFormat, lrn_forward_test_float,
        ::testing::Values(
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::oihw,
            memory::format_tag::oihw, { 2, 64, 56, 56, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
        ));

INSTANTIATE_TEST_SUITE_P(
        TestLRNForwardNCHWTail, lrn_forward_test_float,
        ::testing::Values(
            lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 1, 64, 1, 9, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 1, 64, 2, 9, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 1, 64, 3, 9, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 1, 64, 4, 9, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 1, 64, 5, 9, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 1, 64, 9, 6, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 1, 64, 7, 9, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            , lrn_fwd_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::lrn_across_channels, memory::format_tag::nchw,
            memory::format_tag::nchw, { 1, 64, 8, 9, 5, 1.0e-4f, 0.75f, 1.0f, ACROSS } }
            ));

}
