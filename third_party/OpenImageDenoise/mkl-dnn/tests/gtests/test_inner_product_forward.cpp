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

#include "mkldnn_test_common.hpp"
#include "gtest/gtest.h"

#include "mkldnn.hpp"

namespace mkldnn {

struct test_inner_product_descr_t {
    memory::dim mb;
    memory::dim ic;
    memory::dim oc;
    memory::dim kd, kh, kw;
};

template <typename data_t>
void compute_ref_inner_product_fwd(test_inner_product_descr_t ipd, memory &src,
        memory &weights, memory &bias, memory &dst)
{
    const bool w_bias = bias.get_desc().data.ndims != 0;
    data_t *src_data = (data_t *)src.get_data_handle();
    data_t *weights_data = (data_t *)weights.get_data_handle();
    data_t *bias_data = w_bias ? (data_t *)bias.get_data_handle() : nullptr;
    data_t *dst_data = (data_t *)dst.get_data_handle();

    const memory::desc src_d = src.get_desc();
    const memory::desc weights_d = weights.get_desc();
    const memory::desc bias_d = bias.get_desc();
    const memory::desc dst_d = dst.get_desc();
    const mkldnn::impl::memory_desc_wrapper src_mdw(src_d.data);
    const mkldnn::impl::memory_desc_wrapper weights_mdw(weights_d.data);
    const mkldnn::impl::memory_desc_wrapper bias_mdw(bias_d.data);
    const mkldnn::impl::memory_desc_wrapper dst_mdw(dst_d.data);

    auto padded_ic = src_mdw.padded_dims()[1];

    mkldnn::impl::parallel_nd(ipd.mb, ipd.oc, [&](memory::dim n, memory::dim oc) {
        memory::dim oidx = n * ipd.oc + oc;
        dst_data[dst_mdw.off_l(oidx, true)] = bias_data ?
                bias_data[bias_mdw.off_l(oc, true)] : data_t{0};
        for (memory::dim ic = 0; ic < ipd.ic; ic++) {
            for (memory::dim kd = 0; kd < ipd.kd; kd++)
            for (memory::dim kh = 0; kh < ipd.kh; kh++)
            for (memory::dim kw = 0; kw < ipd.kw; kw++) {
                memory::dim iidx = n * padded_ic * ipd.kd * ipd.kh * ipd.kw
                        + ic * ipd.kd * ipd.kh * ipd.kw
                        + kd * ipd.kh * ipd.kw + kh * ipd.kw + kw;
                memory::dim widx = oc * padded_ic * ipd.kd * ipd.kh * ipd.kw
                        + ic * ipd.kd * ipd.kh * ipd.kw
                        + kd * ipd.kh * ipd.kw + kh * ipd.kw + kw;
                dst_data[dst_mdw.off_l(oidx, true)]
                        += src_data[src_mdw.off_l(iidx, true)]
                        * weights_data[weights_mdw.off_l(widx, true)];
            }
        }
    });
}

struct inprod_test_params {
    prop_kind aprop_kind;
    const engine::kind engine_kind;
    memory::format_tag src_format;
    memory::format_tag weights_format;
    memory::format_tag bias_format;
    memory::format_tag dst_format;
    int ndims;
    test_inner_product_descr_t test_ipd;
    bool expect_to_fail;
    mkldnn_status_t expected_status;
};

template <typename data_t>
class inner_product_test : public ::testing::TestWithParam<inprod_test_params> {
protected:
    virtual void SetUp() {
        auto p = ::testing::TestWithParam<inprod_test_params>::GetParam();
        catch_expected_failures([=](){Test();}, p.expect_to_fail,
                    p.expected_status);
    }

    void Test() {
        auto p = ::testing::TestWithParam<inprod_test_params>::GetParam();
        test_inner_product_descr_t ipd = p.test_ipd;
        bool has_spatial = ipd.kh > 1 || ipd.kw > 1;
        if (p.ndims == 5) has_spatial = has_spatial || ipd.kd > 1;
        bool with_bias = p.bias_format != memory::format_tag::format_tag_undef;

        ASSERT_TRUE(p.engine_kind == engine::kind::cpu);
        ASSERT_EQ(p.aprop_kind, prop_kind::forward);
        auto eng = engine(p.engine_kind, 0);
        auto strm = stream(eng);
        memory::data_type data_type = data_traits<data_t>::data_type;
        ASSERT_EQ(data_type, mkldnn::memory::data_type::f32);

        auto ip_src_desc = has_spatial ? p.ndims == 5
                ? create_md({ ipd.mb, ipd.ic, ipd.kd, ipd.kh, ipd.kw },
                    data_type, p.src_format)
                : create_md({ ipd.mb, ipd.ic, ipd.kh, ipd.kw }, data_type,
                        p.src_format) :
                create_md({ ipd.mb, ipd.ic }, data_type, p.src_format);
        auto ip_weights_desc = has_spatial ? p.ndims == 5
                ? create_md({ ipd.oc, ipd.ic, ipd.kd, ipd.kh, ipd.kw },
                    data_type, p.weights_format)
                : create_md({ ipd.oc, ipd.ic, ipd.kh, ipd.kw }, data_type,
                        p.weights_format) :
                create_md({ ipd.oc, ipd.ic }, data_type, p.weights_format);
        auto ip_bias_desc = with_bias ?
                create_md({ ipd.oc }, data_type, p.bias_format) :
                create_md({}, data_type, p.bias_format);
        auto ip_dst_desc = create_md({ ipd.mb, ipd.oc }, data_type,
            p.dst_format);

        std::shared_ptr<memory> ip_src, ip_weights, ip_dst, ip_bias, dst_ref;

        auto ip_desc = with_bias
            ? inner_product_forward::desc(p.aprop_kind, ip_src_desc,
                    ip_weights_desc, ip_bias_desc, ip_dst_desc)
            : inner_product_forward::desc(p.aprop_kind, ip_src_desc,
                    ip_weights_desc, ip_dst_desc);

        auto ip_primitive_desc = inner_product_forward::primitive_desc(
                ip_desc, eng);

        ip_src.reset(new memory(ip_primitive_desc.src_desc(), eng));
        ip_weights.reset(new memory(ip_primitive_desc.weights_desc(), eng));
        ip_bias.reset(new memory(ip_primitive_desc.bias_desc(), eng));
        ip_dst.reset(new memory(ip_primitive_desc.dst_desc(), eng));
        dst_ref.reset(new memory(ip_primitive_desc.dst_desc(), eng));

        fill_data<data_t>(
                ip_src->get_desc().get_size() / sizeof(data_t),
                (data_t *)ip_src->get_data_handle());
        fill_data<data_t>(
                ip_weights->get_desc().get_size() / sizeof(data_t),
                (data_t *)ip_weights->get_data_handle());
        if (with_bias) {
            fill_data<data_t>(
                    ip_bias->get_desc().get_size() / sizeof(data_t),
                    (data_t *)ip_bias->get_data_handle());
        }
        check_zero_tail<data_t>(1, *ip_src);
        check_zero_tail<data_t>(1, *ip_weights);

        inner_product_forward(ip_primitive_desc).execute(strm, {
                {MKLDNN_ARG_SRC, *ip_src},
                {MKLDNN_ARG_WEIGHTS, *ip_weights},
                {MKLDNN_ARG_BIAS, *ip_bias},
                {MKLDNN_ARG_DST, *ip_dst}});

        compute_ref_inner_product_fwd<data_t>(ipd, *ip_src, *ip_weights,
                *ip_bias, *dst_ref);
        check_zero_tail<data_t>(1, *dst_ref);
        compare_data<data_t>(*dst_ref, *ip_dst);

        check_zero_tail<data_t>(0, *ip_dst);
    }
};

using inner_product_test_float = inner_product_test<float>;
using inprod_test_params_float = inprod_test_params;

#define EXPAND_SIZES_3D(...) 5, { __VA_ARGS__ }
#define EXPAND_SIZES_2D(mb,ic,oc,kh,kw) \
    4, { mb,ic,oc,1,kh,kw }

TEST_P(inner_product_test_float, TestsInnerProduct)
{
}

INSTANTIATE_TEST_SUITE_P(
        TestInnerProductForwardZeroDim, inner_product_test_float,
        ::testing::Values(
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::any, memory::format_tag::any,
                        memory::format_tag::any, memory::format_tag::any,
                        EXPAND_SIZES_2D( 0, 32, 48, 6, 6 )}));

INSTANTIATE_TEST_SUITE_P(
        TestInnerProductForwardEF, inner_product_test_float,
        ::testing::Values(
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::any, memory::format_tag::any,
                        memory::format_tag::any, memory::format_tag::any,
                        EXPAND_SIZES_2D( 2, 0, 48, 6, 6 ),
                        true, mkldnn_invalid_arguments},
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::any, memory::format_tag::any,
                        memory::format_tag::any, memory::format_tag::any,
                        EXPAND_SIZES_2D( -1, 32, 48, 6, 6 ),
                        true, mkldnn_invalid_arguments},
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::any, memory::format_tag::any,
                        memory::format_tag::any, memory::format_tag::any,
                        EXPAND_SIZES_2D( 2, -1, 48, 6, 6 ),
                        true, mkldnn_invalid_arguments}));

INSTANTIATE_TEST_SUITE_P(
        TestInnerProductForwardNoBias_padded, inner_product_test_float,
        ::testing::Values(
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nChw16c, memory::format_tag::aBcd16b,
                        memory::format_tag::format_tag_undef, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 4, 14, 25, 5, 5 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nChw16c, memory::format_tag::aBcd16b,
                        memory::format_tag::format_tag_undef, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 4, 20, 15, 5, 5 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nChw8c, memory::format_tag::aBcd8b,
                        memory::format_tag::format_tag_undef, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 4, 6, 15, 5, 5 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nChw8c, memory::format_tag::aBcd8b,
                        memory::format_tag::format_tag_undef, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 4, 10, 5, 5, 5 ) } ));

INSTANTIATE_TEST_SUITE_P(
        TestInnerProductForwardNoBias, inner_product_test_float,
        ::testing::Values(
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::any, memory::format_tag::any,
                        memory::format_tag::format_tag_undef, memory::format_tag::any,
                        EXPAND_SIZES_2D( 2, 32, 48, 6, 6 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::any, memory::format_tag::any,
                        memory::format_tag::format_tag_undef, memory::format_tag::any,
                        EXPAND_SIZES_2D( 2, 512, 48, 2, 2 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nhwc, memory::format_tag::hwio,
                        memory::format_tag::format_tag_undef, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 2, 32, 48, 6, 6 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nhwc, memory::format_tag::oihw,
                        memory::format_tag::format_tag_undef, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 2, 32, 48, 6, 6 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nchw, memory::format_tag::oihw,
                        memory::format_tag::format_tag_undef, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 2, 32, 48, 6, 6 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nChw8c, memory::format_tag::aBcd8b,
                        memory::format_tag::format_tag_undef, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 2, 32, 48, 6, 6 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nChw16c, memory::format_tag::aBcd16b,
                        memory::format_tag::format_tag_undef, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 2, 32, 48, 6, 6 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nc, memory::format_tag::oi,
                        memory::format_tag::format_tag_undef, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 2, 32, 1152, 1, 1 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nc, memory::format_tag::oi,
                        memory::format_tag::format_tag_undef, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 2, 2, 4, 1, 1 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nc, memory::format_tag::io,
                        memory::format_tag::format_tag_undef, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 2, 8, 16, 1, 1 ) }));

INSTANTIATE_TEST_SUITE_P(
        TestInnerProductForward3D, inner_product_test_float,
        ::testing::Values(
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::any, memory::format_tag::any,
                        memory::format_tag::format_tag_undef, memory::format_tag::any,
                        EXPAND_SIZES_3D( 2, 32, 48, 6, 6, 6 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::ncdhw, memory::format_tag::oidhw,
                        memory::format_tag::format_tag_undef, memory::format_tag::nc,
                        EXPAND_SIZES_3D( 2, 32, 48, 6, 6, 6 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nCdhw8c, memory::format_tag::aBcde8b,
                        memory::format_tag::x, memory::format_tag::nc,
                        EXPAND_SIZES_3D( 2, 32, 48, 6, 6, 6 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nCdhw16c, memory::format_tag::aBcde16b,
                        memory::format_tag::x, memory::format_tag::nc,
                        EXPAND_SIZES_3D( 2, 32, 48, 6, 6, 6 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::ndhwc, memory::format_tag::dhwio,
                        memory::format_tag::format_tag_undef, memory::format_tag::nc,
                        EXPAND_SIZES_3D( 2, 16, 48, 3, 3, 3 ) }));

INSTANTIATE_TEST_SUITE_P(
        TestInnerProductForward, inner_product_test_float,
        ::testing::Values(
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::any, memory::format_tag::any,
                        memory::format_tag::any, memory::format_tag::any,
                        EXPAND_SIZES_2D( 2, 32, 48, 6, 6 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::any, memory::format_tag::any,
                        memory::format_tag::any, memory::format_tag::any,
                        EXPAND_SIZES_2D( 2, 512, 48, 2, 2 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nhwc, memory::format_tag::oihw,
                        memory::format_tag::x, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 2, 32, 48, 6, 6 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nhwc, memory::format_tag::hwio,
                        memory::format_tag::x, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 2, 32, 48, 6, 6 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nchw, memory::format_tag::oihw,
                        memory::format_tag::x, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 2, 32, 48, 6, 6 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nChw8c, memory::format_tag::aBcd8b,
                        memory::format_tag::x, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 2, 32, 48, 6, 6 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nChw16c, memory::format_tag::aBcd16b,
                        memory::format_tag::x, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 2, 32, 48, 6, 6 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nc, memory::format_tag::oi,
                        memory::format_tag::x, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 2, 32, 1152, 1, 1 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nc, memory::format_tag::oi,
                        memory::format_tag::x, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 2, 2, 4, 1, 1 ) },
                inprod_test_params_float{ prop_kind::forward, engine::kind::cpu,
                        memory::format_tag::nc, memory::format_tag::oi,
                        memory::format_tag::x, memory::format_tag::nc,
                        EXPAND_SIZES_2D( 2, 8, 16, 1, 1 ) }));
}
