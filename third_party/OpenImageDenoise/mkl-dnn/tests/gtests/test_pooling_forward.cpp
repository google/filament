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

struct test_pool_desc_t {
    memory::dim mb, c;
    memory::dim id, ih, iw;
    memory::dim od, oh, ow;
    memory::dim kd, kh, kw;
    memory::dim padf, padt, padl;
    memory::dim strd, strh, strw;
};

struct pool_test_params {
    prop_kind aprop_kind;
    engine::kind engine_kind;
    algorithm aalgorithm;
    memory::format_tag src_format;
    memory::format_tag dst_format;
    int ndims;
    test_pool_desc_t test_pd;
    bool expect_to_fail;
    mkldnn_status_t expected_status;
};

template <typename data_t>
void check_pool_fwd(const pool_test_params &p, const memory &src,
        const memory &dst, const memory &ws)
{
    data_t *src_data = (data_t *)src.get_data_handle();
    data_t *dst_data = (data_t *)dst.get_data_handle();

    auto ws_data = [=](size_t idx) -> int {
        auto w = (unsigned char *)ws.get_data_handle();
        if (w == nullptr) return -1;
        if (ws.get_desc().data.data_type == mkldnn_u8)
            return (int)w[idx];
        else
            return ((int *)w)[idx];
    };

    const memory::desc src_d = src.get_desc();
    const memory::desc dst_d = dst.get_desc();
    const memory::desc ws_d  = ws.get_desc();

    const mkldnn::impl::memory_desc_wrapper src_mdw(src_d.data);
    const mkldnn::impl::memory_desc_wrapper dst_mdw(dst_d.data);
    const mkldnn::impl::memory_desc_wrapper ws_mdw(ws_d.data);

    auto pd = p.test_pd;
    size_t padded_c = src_d.data.padded_dims[1];

    mkldnn::impl::parallel_nd(pd.mb, pd.c, pd.od, pd.oh, pd.ow,
        [&](memory::dim n, memory::dim c, memory::dim od, memory::dim oh,
            memory::dim ow) {
            memory::dim oidx = n * padded_c * pd.od * pd.oh * pd.ow
                    + c * pd.od * pd.oh * pd.ow
                    + od * pd.oh * pd.ow
                    + oh * pd.ow + ow;
            data_t out = dst_data[dst_mdw.off_l(oidx, true)];
            int out_index = -1;
            if(p.aalgorithm == pooling_max
                && p.aprop_kind == prop_kind::forward_training) {
                out_index = ws_data(ws_mdw.off_l(oidx, true));
            }
            // match implementation for pooling_max: padding
            // is done with lowest value and not zero, it
            // affects the case when kernel slips into
            // the padding area entirely
            typename acc_t<data_t>::type acc_ref
                    = (p.aalgorithm == pooling_max) ?
                    std::numeric_limits<data_t>::lowest() :
                    data_t(0);
            int out_ref_index = 0;
            bool is_initialized = false;
            int num_summands = 0;

            for (memory::dim kd = 0; kd < pd.kd; ++kd)
            for (memory::dim kh = 0; kh < pd.kh; ++kh)
            for (memory::dim kw = 0; kw < pd.kw; ++kw)
            {
                const memory::dim id = od * pd.strd - pd.padf + kd;
                const memory::dim ih = oh * pd.strh - pd.padt + kh;
                const memory::dim iw = ow * pd.strw - pd.padl + kw;

                if (id < 0 || id >= pd.id) continue;
                if (ih < 0 || ih >= pd.ih) continue;
                if (iw < 0 || iw >= pd.iw) continue;

                size_t iidx
                        = (size_t)n * padded_c * pd.id * pd.ih * pd.iw
                        + (size_t)c * pd.id * pd.ih * pd.iw
                        + (size_t)id * pd.ih * pd.iw
                        + (size_t)ih * pd.iw + iw;

                data_t d = src_data[src_mdw.off_l(iidx, true)];
                if (p.aalgorithm == pooling_max) {
                    if (!is_initialized) {
                        acc_ref = d;
                        out_ref_index =
                            (int)(kd * pd.kw * pd.kh + kh * pd.kw + kw);
                        is_initialized = true;
                    } else {
                        if (acc_ref < d) {
                            acc_ref = d;
                            out_ref_index =
                                (int)(kd * pd.kw * pd.kh + kh * pd.kw + kw);
                        }
                    }
                } else if (p.aalgorithm == pooling_avg_include_padding ||
                    p.aalgorithm == pooling_avg_exclude_padding) {
                    acc_ref += d;
                    num_summands++;
                }
            }

            if (p.aalgorithm == pooling_avg_include_padding) {
                num_summands = pd.kw * pd.kh * pd.kd;
            }

            if ((p.aalgorithm == pooling_avg_include_padding ||
                p.aalgorithm == pooling_avg_exclude_padding) &&
                num_summands)  {
                acc_ref = out_round<data_t>(
                    (float)acc_ref / num_summands);
            }

            const data_t out_ref = (data_t)acc_ref;
            EXPECT_NEAR(out, out_ref, 1e-6);
            if(p.aalgorithm == pooling_max
                && p.aprop_kind == forward_training) {
                EXPECT_EQ(out_index, out_ref_index) << " n = " << n
                << " c = " << c << " od = " << od << " oh = " << oh
                << " ow = " << ow;
            }
        }
    );
}

template <typename data_t>
class pooling_test : public ::testing::TestWithParam<pool_test_params> {
    pool_test_params p;

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

        test_pool_desc_t pd = p.test_pd;
        auto p_src_desc = (p.ndims == 5)
            ? create_md({ pd.mb, pd.c, pd.id, pd.ih, pd.iw }, data_type,
                p.src_format)
            : create_md({ pd.mb, pd.c, pd.ih, pd.iw }, data_type, p.src_format);
        auto p_dst_desc = (p.ndims == 5)
            ? create_md({ pd.mb, pd.c, pd.od, pd.oh, pd.ow }, data_type,
            p.dst_format)
            : create_md({ pd.mb, pd.c, pd.oh, pd.ow }, data_type, p.dst_format);

        auto p_src = memory(p_src_desc, eng);
        auto p_dst = memory(p_dst_desc, eng);

        fill_data<data_t>(p_src.get_desc().get_size()/ sizeof(data_t),
                (data_t *)p_src.get_data_handle(), 1., true);
        fill_data<data_t>(p_dst.get_desc().get_size()/ sizeof(data_t),
                (data_t *)p_dst.get_data_handle(), 1., true);
        check_zero_tail<data_t>(1, p_src);
        check_zero_tail<data_t>(1, p_dst);

        memory::dims strides, ker, pad_l, pad_r;
        if (p.ndims == 5) {
            strides = memory::dims({pd.strd, pd.strh, pd.strw});
            ker = memory::dims({pd.kd, pd.kh, pd.kw});
            pad_l = memory::dims({pd.padf, pd.padt, pd.padl});
            pad_r = memory::dims({
                right_padding(pd.id, pd.od, pd.kd, pd.padf, pd.strd),
                right_padding(pd.ih, pd.oh, pd.kh, pd.padt, pd.strh),
                right_padding(pd.iw, pd.ow, pd.kw, pd.padl, pd.strw)
            });
        } else {
            strides = memory::dims({pd.strh, pd.strw});
            ker = memory::dims({pd.kh, pd.kw});
            pad_l = memory::dims({pd.padt, pd.padl});
            pad_r = memory::dims({
                right_padding(pd.ih, pd.oh, pd.kh, pd.padt, pd.strh),
                right_padding(pd.iw, pd.ow, pd.kw, pd.padl, pd.strw)
            });
        }

        auto pool_desc = pooling_forward::desc(p.aprop_kind, p.aalgorithm,
                p_src_desc, p_dst_desc, strides, ker, pad_l, pad_r,
                padding_kind::zero);
        auto pool_prim_desc
            = pooling_forward::primitive_desc(pool_desc, eng);

        auto workspace_desc = pool_prim_desc.workspace_desc();
        std::shared_ptr<memory> p_workspace;
        p_workspace.reset(new memory(workspace_desc, eng));

        pooling_forward(pool_prim_desc).execute(strm, {
                {MKLDNN_ARG_SRC, p_src},
                {MKLDNN_ARG_DST, p_dst},
                {MKLDNN_ARG_WORKSPACE, *p_workspace}});

        check_pool_fwd<data_t>(p, p_src, p_dst, *p_workspace);
        check_zero_tail<data_t>(0, p_dst);
    }
};

using pooling_test_float = pooling_test<float>;
using pooling_test_s8 = pooling_test<int8_t>;
using pooling_test_u8 = pooling_test<uint8_t>;
using pooling_test_s32 = pooling_test<int32_t>;
using pool_test_params_float = pool_test_params;

//#define EXPAND_NDIMS(ndims, ...) ndims, {__VA_ARGS__}

#define EXPAND_SIZES_3D(...) 5, {__VA_ARGS__}
#define EXPAND_SIZES_2D(mb,ic,ih,iw,oh,ow,kh,kw,padt,padl,strh,strw) \
    4, {mb,ic,1,ih,iw,1,oh,ow,1,kh,kw,0,padt,padl,1,strh,strw}

TEST_P(pooling_test_s8, TestsPooling)
{
}

INSTANTIATE_TEST_SUITE_P(
        TestPoolingAlexnetForwardS8, pooling_test_s8, ::testing::Values(
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc, EXPAND_SIZES_2D(1, 96, 55, 55, 27, 27, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc, EXPAND_SIZES_2D(1, 256, 27, 27, 13, 13, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc, EXPAND_SIZES_2D(1, 256, 13, 13, 6, 6, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingForwardMaxS8, pooling_test_s8, ::testing::Values(
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc,  EXPAND_SIZES_2D(2, 128, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc,  EXPAND_SIZES_2D(2, 64, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc,  EXPAND_SIZES_2D(2, 96, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc,  EXPAND_SIZES_2D(2, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference, engine::kind::cpu,
            algorithm::pooling_max, memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingForwardAvgS8, pooling_test_s8, ::testing::Values(
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 128, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 128, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 64, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 64, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 96, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 96, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference, engine::kind::cpu,
            algorithm::pooling_avg_include_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params{ prop_kind::forward_inference, engine::kind::cpu,
            algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            ));

TEST_P(pooling_test_u8, TestsPooling)
{
}

INSTANTIATE_TEST_SUITE_P(
        TestPoolingForwardMaxU8, pooling_test_u8, ::testing::Values(
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc,  EXPAND_SIZES_2D(2, 128, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc,  EXPAND_SIZES_2D(2, 64, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc,  EXPAND_SIZES_2D(2, 96, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc,  EXPAND_SIZES_2D(2, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference, engine::kind::cpu,
            algorithm::pooling_max, memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingForwardAvgU8, pooling_test_u8, ::testing::Values(
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 128, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 128, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 64, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 64, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 96, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 96, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference, engine::kind::cpu,
            algorithm::pooling_avg_include_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params{ prop_kind::forward_inference, engine::kind::cpu,
            algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            ));

TEST_P(pooling_test_s32, TestsPooling)
{
}

INSTANTIATE_TEST_SUITE_P(
        TestPoolingAlexnetForwardS32, pooling_test_s32, ::testing::Values(
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc,  EXPAND_SIZES_2D(1, 96, 55, 55, 27, 27, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc,  EXPAND_SIZES_2D(1, 256, 27, 27, 13, 13, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc,  EXPAND_SIZES_2D(1, 256, 13, 13, 6, 6, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingForwardMaxS32, pooling_test_s32, ::testing::Values(
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc,  EXPAND_SIZES_2D(2, 128, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc,  EXPAND_SIZES_2D(2, 64, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc,  EXPAND_SIZES_2D(2, 96, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc,  EXPAND_SIZES_2D(2, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference, engine::kind::cpu,
            algorithm::pooling_max, memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingForwardAvgS32, pooling_test_s32, ::testing::Values(
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 128, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 128, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 64, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 64, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 96, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 96, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(2, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params{ prop_kind::forward_inference, engine::kind::cpu,
            algorithm::pooling_avg_include_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params{ prop_kind::forward_inference, engine::kind::cpu,
            algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nhwc, memory::format_tag::nhwc,
             EXPAND_SIZES_2D(16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            ));

TEST_P(pooling_test_float, TestsPooling)
{
}

INSTANTIATE_TEST_SUITE_P(
        TestPoolingForwardZeroDim, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 0, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 )},
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc,  EXPAND_SIZES_2D( 0, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 )},
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 4, 0, 4, 4, 4, 3, 3, 1, 1, 1, 1 )}
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingForwardEF, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, -4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ),
            true, mkldnn_invalid_arguments},
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( -1, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ),
            true, mkldnn_invalid_arguments},
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::eltwise_square, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ),
            true, mkldnn_invalid_arguments}
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPooling_nChw16c_with_padded, pooling_test_float, ::testing::Values(
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D(4, 17,  6,  6,  7,  7, 2, 2, 1, 1, 1, 1) },
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D(4, 23, 60, 60, 31, 31, 3, 4, 1, 1, 2, 2) },
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D(4, 14, 60, 60, 31, 31, 3, 2, 1, 1, 2, 2) },
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D(4, 17, 60, 60, 31, 31, 4, 3, 1, 1, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D(4, 14, 60, 60, 31, 31, 2, 3, 1, 1, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D(4, 25, 60, 60, 31, 31, 2, 4, 1, 1, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D(4, 28, 60, 60, 31, 31, 4, 2, 1, 1, 2, 2) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPooling_nChw8c_with_padded, pooling_test_float, ::testing::Values(
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(4, 5,  6,  6,  7,  7, 2, 2, 1, 1, 1, 1) },
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(4, 9, 60, 60, 31, 31, 3, 4, 1, 1, 2, 2) },
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(4, 14, 60, 60, 31, 31, 3, 2, 1, 1, 2, 2) },
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(4, 17, 60, 60, 31, 31, 4, 3, 1, 1, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(4, 14, 60, 60, 31, 31, 2, 3, 1, 1, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(4, 25, 60, 60, 31, 31, 2, 4, 1, 1, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(4, 28, 60, 60, 31, 31, 4, 2, 1, 1, 2, 2) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingForwardMaxKernelSlipsToPadding, pooling_test_float, ::testing::Values(
            pool_test_params{ prop_kind::forward_training, engine::kind::cpu,
            algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw, EXPAND_SIZES_2D( 1, 16, 10, 10, 6, 6, 5, 5, 10, 10, 5, 5 ) },
            pool_test_params{ prop_kind::forward_training, engine::kind::cpu,
            algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nhwc, EXPAND_SIZES_2D( 1, 16, 10, 10, 6, 6, 5, 5, 10, 10, 5, 5 ) },
            pool_test_params{ prop_kind::forward_training, engine::kind::cpu,
            algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 1, 16, 10, 10, 6, 6, 5, 5, 10, 10, 5, 5 ) },
            pool_test_params{ prop_kind::forward_training, engine::kind::cpu,
            algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 1, 16, 10, 10, 6, 6, 5, 5, 10, 10, 5, 5 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPooling3D_nCdhw16c, pooling_test_float, ::testing::Values(
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nCdhw16c,
            memory::format_tag::nCdhw16c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 2, 3, 4, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding, memory::format_tag::nCdhw16c,
            memory::format_tag::nCdhw16c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 4, 3, 2, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding, memory::format_tag::nCdhw16c,
            memory::format_tag::nCdhw16c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 2, 4, 3, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nCdhw16c,
            memory::format_tag::nCdhw16c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 4, 2, 3, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding, memory::format_tag::nCdhw16c,
            memory::format_tag::nCdhw16c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 3, 2, 4, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding, memory::format_tag::nCdhw16c,
            memory::format_tag::nCdhw16c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 3, 4, 2, 1, 1, 1, 2, 2, 2) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPooling3D_nCdhw8c, pooling_test_float, ::testing::Values(
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nCdhw8c,
            memory::format_tag::nCdhw8c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 2, 3, 4, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding, memory::format_tag::nCdhw8c,
            memory::format_tag::nCdhw8c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 4, 3, 2, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding, memory::format_tag::nCdhw8c,
            memory::format_tag::nCdhw8c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 2, 4, 3, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nCdhw8c,
            memory::format_tag::nCdhw8c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 4, 2, 3, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding, memory::format_tag::nCdhw8c,
            memory::format_tag::nCdhw8c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 3, 2, 4, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding, memory::format_tag::nCdhw8c,
            memory::format_tag::nCdhw8c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 3, 4, 2, 1, 1, 1, 2, 2, 2) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPooling3D_ndhwc, pooling_test_float, ::testing::Values(
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 2, 3, 4, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 4, 3, 2, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 2, 4, 3, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 4, 2, 3, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 3, 2, 4, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 3, 4, 2, 1, 1, 1, 2, 2, 2) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPooling3D_ncdhw, pooling_test_float, ::testing::Values(
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 2, 3, 4, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 4, 3, 2, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 2, 4, 3, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 4, 2, 3, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 3, 2, 4, 1, 1, 1, 2, 2, 2) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 31, 31, 3, 4, 2, 1, 1, 1, 2, 2, 2) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPooling3Dunet_ncdhw, pooling_test_float, ::testing::Values(
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(1, 64, 64, 64, 64, 64, 64, 64, 2, 2, 2, 0, 0, 0, 1, 1, 1) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(1, 128, 28, 28, 28, 28, 28, 28, 2, 2, 2, 0, 0, 0, 1, 1, 1) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(1, 256, 12, 12, 12, 12, 12, 12, 2, 2, 2, 0, 0, 0, 1, 1, 1) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPooling3Dunet_ndhwc, pooling_test_float, ::testing::Values(
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(1, 64, 64, 64, 64, 64, 64, 64, 2, 2, 2, 0, 0, 0, 1, 1, 1) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(1, 128, 28, 28, 28, 28, 28, 28, 2, 2, 2, 0, 0, 0, 1, 1, 1) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(1, 256, 12, 12, 12, 12, 12, 12, 2, 2, 2, 0, 0, 0, 1, 1, 1) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPooling3Dunet_blocked, pooling_test_float, ::testing::Values(
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nCdhw16c,
            memory::format_tag::nCdhw16c, EXPAND_SIZES_3D(1, 64, 64, 64, 64, 64, 64, 64, 2, 2, 2, 0, 0, 0, 1, 1, 1) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nCdhw16c,
            memory::format_tag::nCdhw16c, EXPAND_SIZES_3D(1, 128, 28, 28, 28, 28, 28, 28, 2, 2, 2, 0, 0, 0, 1, 1, 1) },
            pool_test_params{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nCdhw16c,
            memory::format_tag::nCdhw16c, EXPAND_SIZES_3D(1, 256, 12, 12, 12, 12, 12, 12, 2, 2, 2, 0, 0, 0, 1, 1, 1) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingForwardMax, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 4, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 4, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) }
            ));


INSTANTIATE_TEST_SUITE_P(
        TestPoolingForwardMaxNHWC, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc,  EXPAND_SIZES_2D( 2, 4, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingForwardMaxBlocked, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 32, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 32, 13, 13, 12, 12, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 32, 4, 4, 4, 4, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 32, 3, 3, 4, 4, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 32, 3, 3, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 122, 32, 32, 2, 32, 2, 3, 3, 1, 1, 1, 1 ) }

            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingForwardMaxBlockedPerf, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training, engine::kind::cpu,
            algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            , pool_test_params_float{ prop_kind::forward_training, engine::kind::cpu,
            algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingForwardAvgBlockedPerf, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training, engine::kind::cpu,
            algorithm::pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 1, 8, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1 ) }
            , pool_test_params_float{ prop_kind::forward_training, engine::kind::cpu,
            algorithm::pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            , pool_test_params_float{ prop_kind::forward_training, engine::kind::cpu,
            algorithm::pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            , pool_test_params_float{ prop_kind::forward_training, engine::kind::cpu,
            algorithm::pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            , pool_test_params_float{ prop_kind::forward_training, engine::kind::cpu,
            algorithm::pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingForwardMaxBlocked16, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 32, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 32, 13, 13, 12, 12, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 32, 4, 4, 4, 4, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 32, 3, 3, 4, 4, 3, 3, 1, 1, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 32, 3, 3, 2, 2, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 122, 32, 32, 2, 32, 2, 3, 3, 1, 1, 1, 1 ) }

            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingForwardMaxBlocked16Perf, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training, engine::kind::cpu,
            algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            , pool_test_params_float{ prop_kind::forward_training, engine::kind::cpu,
            algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingForwardAvgBlocked16Perf, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training, engine::kind::cpu,
            algorithm::pooling_avg_include_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            , pool_test_params_float{ prop_kind::forward_training, engine::kind::cpu,
            algorithm::pooling_avg_exclude_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            , pool_test_params_float{ prop_kind::forward_training, engine::kind::cpu,
            algorithm::pooling_avg_include_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            , pool_test_params_float{ prop_kind::forward_training, engine::kind::cpu,
            algorithm::pooling_avg_exclude_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            ));


INSTANTIATE_TEST_SUITE_P(
        TestPoolingAlexnetForwardMaxNCHW, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 16, 55, 55, 27, 27, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 16, 55, 55, 27, 27, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 16, 27, 27, 13, 13, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 16, 27, 27, 13, 13, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 16, 13, 13, 6, 6, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 16, 13, 13, 6, 6, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingAlexnetForwardMaxBlocked, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 16, 55, 55, 27, 27, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 16, 55, 55, 27, 27, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 16, 27, 27, 13, 13, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 16, 27, 27, 13, 13, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 16, 13, 13, 6, 6, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 16, 13, 13, 6, 6, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingAlexnetForwardMaxBlocked16, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 16, 55, 55, 27, 27, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 16, 55, 55, 27, 27, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 16, 27, 27, 13, 13, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 16, 27, 27, 13, 13, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 16, 13, 13, 6, 6, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 16, 13, 13, 6, 6, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingMaxBlockedStride1, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 16, 55, 55, 53, 53, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 16, 55, 55, 53, 53, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 16, 27, 27, 25, 25, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 16, 27, 27, 25, 25, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 16, 13, 13, 11, 11, 3, 3, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 16, 13, 13, 11, 11, 3, 3, 0, 0, 1, 1 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingMaxCIFAR10NCHW, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 32, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 32, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingAvgCIFAR10NCHW, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 32, 16, 16, 8, 8, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 32, 16, 16, 8, 8, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 32, 16, 15, 8, 8, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 32, 16, 15, 8, 8, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 64, 8, 8, 4, 4, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 64, 8, 8, 4, 4, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 64, 8, 8, 4, 4, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 64, 8, 8, 4, 4, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingMaxCIFAR10Blocked, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 32, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 32, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingAvgCIFAR10Blocked, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 32, 16, 16, 8, 8, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 32, 16, 16, 8, 8, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 32, 16, 16, 8, 8, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 32, 16, 16, 8, 8, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 64, 8, 8, 4, 4, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 64, 8, 8, 4, 4, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 64, 8, 8, 4, 4, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 64, 8, 8, 4, 4, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingMaxCIFAR10Blocked16, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 32, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 32, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingAvgCIFAR10Blocked16, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 32, 16, 16, 8, 8, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 32, 16, 16, 8, 8, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 32, 16, 16, 8, 8, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 32, 16, 16, 8, 8, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 64, 8, 8, 4, 4, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 64, 8, 8, 4, 4, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 64, 8, 8, 4, 4, 3, 3, 0, 0, 2, 2 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 64, 8, 8, 4, 4, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingMaxGoogleNetV1NCHW, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingMaxGoogleNetV1Blocked, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingMaxGoogleNetV1Blocked16, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingMaxResnet50NCHW, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw,  EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingMaxResnet50Blocked, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingMaxResnet50Blocked16, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c,  EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingAvgGoogleNetV1NCHW, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingAvgGoogleNetV1Blocked, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingAvgGoogleNetV1Blocked16, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 512, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 528, 14, 14, 4, 4, 5, 5, 0, 0, 3, 3 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 1024, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingAvgResnet50NCHW, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nchw, memory::format_tag::nchw,
             EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingAvgResnet50Blocked, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingAvgResnet50Blocked16, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_training,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) },
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw16c, memory::format_tag::nChw16c,
             EXPAND_SIZES_2D( 2, 512, 7, 7, 1, 1, 7, 7, 0, 0, 1, 1 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingAsymmPadding, pooling_test_float, ::testing::Values(
            pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D(1, 8, 3, 4, 1, 5, 3, 3, 0, 1, 1, 1) }
            ,pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D(1, 8, 3, 4, 1, 5, 3, 3, 0, 1, 1, 1) }
            ,pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D(1, 8, 3, 4, 1, 5, 3, 3, 0, 1, 1, 1) }

            ,pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D(1, 8, 3, 14, 1, 8, 3, 3, 0, 1, 1, 2) }
            ,pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D(1, 8, 3, 14, 1, 8, 3, 3, 0, 1, 1, 2) }
            ,pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D(1, 8, 3, 14, 1, 8, 3, 3, 0, 1, 1, 2) }

            ,pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D(1, 96, 3, 100, 1, 51, 3, 3, 0, 1, 1, 2) }
            ,pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D(1, 96, 3, 100, 1, 51, 3, 3, 0, 1, 1, 2) }
            ,pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D(1, 96, 3, 100, 1, 51, 3, 3, 0, 1, 1, 2) }

            ,pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D(1, 96, 3, 102, 1, 52, 3, 3, 0, 1, 1, 2) }
            ,pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D(1, 96, 3, 102, 1, 52, 3, 3, 0, 1, 1, 2) }
            ,pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D(1, 96, 3, 102, 1, 52, 3, 3, 0, 1, 1, 2) }

            ,pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D(1, 96, 9, 103, 7, 52, 3, 3, 0, 1, 1, 2) }
            ,pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D(1, 96, 9, 103, 7, 52, 3, 3, 0, 1, 1, 2) }
            ,pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D(1, 96, 9, 103, 7, 52, 3, 3, 0, 1, 1, 2) }

            ,pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c,  EXPAND_SIZES_2D(1, 96, 300, 500, 151, 251, 3, 3, 1, 1, 2, 2) }
            ,pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_include_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D(1, 96, 300, 500, 151, 251, 3, 3, 1, 1, 2, 2) }
            ,pool_test_params_float{ prop_kind::forward_inference,
            engine::kind::cpu, algorithm::pooling_avg_exclude_padding,
            memory::format_tag::nChw8c, memory::format_tag::nChw8c,
             EXPAND_SIZES_2D(1, 96, 300, 500, 151, 251, 3, 3, 1, 1, 2, 2) }

            ));
}
