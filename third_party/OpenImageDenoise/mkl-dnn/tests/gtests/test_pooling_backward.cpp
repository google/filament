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

struct test_pool_bwd_desc_t {
    memory::dim mb, c;
    memory::dim id, ih, iw;
    memory::dim od, oh, ow;
    memory::dim kd, kh, kw;
    memory::dim padf, padt, padl;
    memory::dim strd, strh, strw;
};

struct pool_bwd_test_params {
    engine::kind engine_kind;
    algorithm aalgorithm;
    memory::format_tag diff_src_format;
    memory::format_tag diff_dst_format;
    int ndims;
    test_pool_bwd_desc_t test_pd;
    bool expect_to_fail;
    mkldnn_status_t expected_status;
};

template <typename data_t>
void check_pool_fwd(const pool_bwd_test_params &p, const memory &src,
        const memory &dst)
{
    data_t *src_data = (data_t *)src.get_data_handle();
    data_t *dst_data = (data_t *)dst.get_data_handle();

    const memory::desc src_d = src.get_desc();
    const memory::desc dst_d = dst.get_desc();
    const mkldnn::impl::memory_desc_wrapper src_mdw(src_d.data);
    const mkldnn::impl::memory_desc_wrapper dst_mdw(dst_d.data);

    auto pd = p.test_pd;

    auto apply_offset = [=](memory::dim index, memory::dim offset) {
        return (index > offset) ? index - offset : 0;
    };
    auto padded_c = src_d.data.padded_dims[1];

    mkldnn::impl::parallel_nd(pd.mb, pd.c, pd.od, pd.oh, pd.ow,
        [&](memory::dim n, memory::dim c, memory::dim od, memory::dim oh,
            memory::dim ow) {
            memory::dim oidx = n * padded_c * pd.od * pd.oh * pd.ow
                    + c * pd.od * pd.oh * pd.ow
                    + od * pd.oh * pd.ow + oh * pd.ow + ow;
            data_t out = dst_data[dst_mdw.off_l(oidx, true)];

            // match implementation for pooling_max: padding
            // is done with lowest value and not zero, it
            // affects the case when kernel slips into
            // the padding area entirely
            data_t out_ref = (p.aalgorithm == pooling_max) ?
                    std::numeric_limits<data_t>::lowest() :
                    data_t(0);
            bool is_initialized = false;

            auto id_start = apply_offset(od*pd.strd, pd.padf);
            auto ih_start = apply_offset(oh*pd.strh, pd.padt);
            auto iw_start = apply_offset(ow*pd.strw, pd.padl);
            auto id_end = std::min(od*pd.strd - pd.padf + pd.kd, pd.id);
            auto ih_end = std::min(oh*pd.strh - pd.padt + pd.kh, pd.ih);
            auto iw_end = std::min(ow*pd.strw - pd.padl + pd.kw, pd.iw);

            auto num_summands = p.aalgorithm != pooling_avg_exclude_padding
                ? pd.kw*pd.kh*pd.kd
                : (ih_end - ih_start) * (iw_end - iw_start)
                    * (id_end - id_start);

            for (memory::dim id = id_start; id < id_end; ++id)
            for (memory::dim ih = ih_start; ih < ih_end; ++ih)
            for (memory::dim iw = iw_start; iw < iw_end; ++iw) {
                memory::dim iidx = n * padded_c * pd.id * pd.ih * pd.iw
                        + c * pd.id * pd.ih * pd.iw
                        + id * pd.ih * pd.iw
                        + ih * pd.iw + iw;

                data_t d = src_data[src_mdw.off_l(iidx, true)];
                if (p.aalgorithm == pooling_max) {
                    if (!is_initialized) {
                        out_ref = d;
                        is_initialized = true;
                    } else {
                        if (out_ref < d) out_ref = d;
                    }
                } else if (p.aalgorithm == pooling_avg_include_padding
                    || p.aalgorithm == pooling_avg_exclude_padding) {
                    out_ref += d;
                }
            }

            if (p.aalgorithm == pooling_avg_include_padding ||
                p.aalgorithm == pooling_avg_exclude_padding) {
                out_ref /= num_summands;
            }
            EXPECT_NEAR(out, out_ref, 1e-6f);
        }
    );
}

template <typename data_t>
void check_pool_bwd(const pool_bwd_test_params &p, const memory &diff_src,
        const memory &diff_dst, const memory &ws)
{
    data_t *diff_src_data = (data_t *)diff_src.get_data_handle();
    data_t *diff_dst_data = (data_t *)diff_dst.get_data_handle();

    auto ws_data = [=](size_t idx) -> int {
        auto w = (unsigned char *)ws.get_data_handle();
        if (w == nullptr) return -1;
        if (ws.get_desc().data.data_type == mkldnn_u8)
            return (int)w[idx];
        else
            return ((int *)w)[idx];
    };

    const memory::desc diff_src_d = diff_src.get_desc();
    const memory::desc diff_dst_d = diff_dst.get_desc();
    const memory::desc ws_d = ws.get_desc();

    const mkldnn::impl::memory_desc_wrapper diff_src_mdw(diff_src_d.data);
    const mkldnn::impl::memory_desc_wrapper diff_dst_mdw(diff_dst_d.data);
    const mkldnn::impl::memory_desc_wrapper ws_mdw(ws_d.data);

    auto pd = p.test_pd;
    if (pd.mb * pd.c * pd.id * pd.ih * pd.iw == 0) return;

    std::vector<data_t> ref_diff_src_vec(pd.mb * pd.c * pd.id * pd.ih * pd.iw);
    data_t *ref_diff_src = &ref_diff_src_vec[0];

    auto apply_offset = [=](memory::dim index, memory::dim offset) {
        return (index > offset) ? index - offset : 0;
    };

    mkldnn::impl::parallel_nd(pd.mb * pd.c * pd.id * pd.ih * pd.iw,
        [&](memory::dim i) { ref_diff_src[i] = 0.; }
    );

    mkldnn::impl::parallel_nd(pd.mb, pd.c, [&](memory::dim n, memory::dim c) {
        for (memory::dim od = 0; od < pd.od; od++)
        for (memory::dim oh = 0; oh < pd.oh; oh++)
        for (memory::dim ow = 0; ow < pd.ow; ow++) {
            memory::dim oidx = n * pd.c * pd.od * pd.oh * pd.ow
                    + c * pd.od * pd.oh * pd.ow
                    + od * pd.oh * pd.ow + oh * pd.ow + ow;
            data_t diff_dst = diff_dst_data[diff_dst_mdw.off_l( oidx, true)];
            if (p.aalgorithm == pooling_max) {
                memory::dim kw_max = ws_data(ws_mdw.off_l(oidx, true)) % pd.kw;
                memory::dim kh_max = (ws_data(ws_mdw.off_l(oidx, true)) / pd.kw) % pd.kh;
                memory::dim kd_max = (ws_data(ws_mdw.off_l(oidx, true)) / pd.kw) / pd.kh;
                for (memory::dim kd = 0; kd < pd.kd; kd++)
                for (memory::dim kh = 0; kh < pd.kh; kh++)
                for (memory::dim kw = 0; kw < pd.kw; kw++) {
                    memory::dim iw = ow * pd.strw - pd.padl + kw;
                    memory::dim ih = oh * pd.strh - pd.padt + kh;
                    memory::dim id = od * pd.strd - pd.padf + kd;
                    if (iw < 0 || iw >= pd.iw) continue;
                    if (ih < 0 || ih >= pd.ih) continue;
                    if (id < 0 || id >= pd.id) continue;
                    memory::dim iidx = n * pd.c * pd.id * pd.ih * pd.iw
                            + c * pd.id * pd.ih * pd.iw
                            + id * pd.ih * pd.iw
                            + ih * pd.iw + iw;

                    if (kh == kh_max && kw == kw_max && kd == kd_max)
                        ref_diff_src[iidx] += diff_dst;
                }
            } else if (p.aalgorithm == pooling_avg_include_padding
                || p.aalgorithm == pooling_avg_exclude_padding) {
                auto id_start = apply_offset(od*pd.strd, pd.padf);
                auto ih_start = apply_offset(oh*pd.strh, pd.padt);
                auto iw_start = apply_offset(ow*pd.strw, pd.padl);
                auto id_end = std::min(od*pd.strd - pd.padf + pd.kd, pd.id);
                auto ih_end = std::min(oh*pd.strh - pd.padt + pd.kh, pd.ih);
                auto iw_end = std::min(ow*pd.strw - pd.padl + pd.kw, pd.iw);

                auto num_summands = (p.aalgorithm != pooling_avg_exclude_padding)
                    ? pd.kw*pd.kh*pd.kd
                    : (ih_end - ih_start) * (iw_end - iw_start)
                        * (id_end - id_start);

                for (int id = id_start; id < id_end; id++)
                for (int ih = ih_start; ih < ih_end; ih++)
                for (int iw = iw_start; iw < iw_end; iw++)
                {
                    memory::dim iidx = n * pd.c * pd.id * pd.ih * pd.iw
                            + c * pd.id * pd.ih * pd.iw
                            + id * pd.ih * pd.iw
                            + ih * pd.iw + iw;
                    ref_diff_src[iidx] += diff_dst / num_summands;
                }
            }
        }
    });

    mkldnn::impl::parallel_nd(pd.mb * pd.c * pd.id * pd.ih * pd.iw,
        [&](memory::dim i) {
            EXPECT_NEAR(ref_diff_src[i],
                    diff_src_data[diff_src_mdw.off_l( i, true)], 1e-5f);
        }
    );
}

template <typename data_t>
class pooling_bwd_test : public ::testing::TestWithParam<pool_bwd_test_params> {
private:
    std::shared_ptr<memory::desc> src_desc;
    std::shared_ptr<memory::desc> dst_desc;
    std::shared_ptr<memory> workspace;
    std::shared_ptr<pooling_forward::primitive_desc> pool_prim_desc;
    pool_bwd_test_params p;
    memory::dims strides, ker, pad_l, pad_r;
    std::shared_ptr<engine> eng;
    std::shared_ptr<stream> strm;
    memory::data_type data_type;

protected:
    virtual void SetUp() {
        p = ::testing::TestWithParam<decltype(p)>::GetParam();
        catch_expected_failures([=](){Test();}, p.expect_to_fail,
                    p.expected_status);
    }

    void Test() {
        test_pool_bwd_desc_t pd = p.test_pd;

        ASSERT_TRUE(p.engine_kind == engine::kind::cpu);
        eng.reset(new engine(p.engine_kind, 0));
        strm.reset(new stream(*eng));
        data_type = data_traits<data_t>::data_type;
        ASSERT_EQ(data_type, mkldnn::memory::data_type::f32);

        if (p.ndims == 5)
        {
            src_desc.reset(new memory::desc(
                { pd.mb, pd.c, pd.id, pd.ih, pd.iw }, data_type,
                p.diff_src_format));
            dst_desc.reset(new memory::desc(
                { pd.mb, pd.c, pd.od, pd.oh, pd.ow }, data_type,
                p.diff_dst_format));
        } else {
            src_desc.reset(new memory::desc(
                { pd.mb, pd.c, pd.ih, pd.iw }, data_type, p.diff_src_format));
            dst_desc.reset(new memory::desc(
                { pd.mb, pd.c, pd.oh, pd.ow }, data_type, p.diff_dst_format));
        }

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

        Forward();
        Backward();
    }

    void Forward()
    {
        std::shared_ptr<memory> src;
        std::shared_ptr<memory> dst;

        auto pool_desc = pooling_forward::desc(prop_kind::forward_training,
                p.aalgorithm, *src_desc, *dst_desc, strides, ker, pad_l, pad_r,
                padding_kind::zero);
        pool_prim_desc.reset(
                new pooling_forward::primitive_desc(pool_desc, *eng));

        auto p_workspace_desc = pool_prim_desc->workspace_desc();

        src.reset(new memory({*src_desc, *eng}));
        workspace.reset(new memory(p_workspace_desc, *eng));
        dst.reset(new memory({*dst_desc, *eng}));

        fill_data<data_t>(
                src->get_desc().get_size() / sizeof(data_t),
                (data_t *)src->get_data_handle());
        fill_data<data_t>(
                dst->get_desc().get_size() / sizeof(data_t),
                (data_t *)dst->get_data_handle());
        check_zero_tail<data_t>(1, *src);
        check_zero_tail<data_t>(1, *dst);

        pooling_forward(*pool_prim_desc).execute(*strm, {
                {MKLDNN_ARG_SRC, *src},
                {MKLDNN_ARG_DST, *dst},
                {MKLDNN_ARG_WORKSPACE, *workspace}});

        check_zero_tail<data_t>(0, *dst);
        check_pool_fwd<data_t>(p, *src, *dst);
    }

    void Backward()
    {
        std::shared_ptr<memory> diff_src;
        std::shared_ptr<memory> diff_dst;

        auto pool_bwd_desc = pooling_backward::desc(p.aalgorithm, *src_desc,
                *dst_desc, strides, ker, pad_l, pad_r, padding_kind::zero);
        auto pool_bwd_prim_desc = pooling_backward::primitive_desc(
                pool_bwd_desc, *eng, *pool_prim_desc);

        diff_src.reset(new memory({*src_desc, *eng}));
        diff_dst.reset(new memory({*dst_desc, *eng}));

        fill_data<data_t>(
                diff_dst->get_desc().get_size()/ sizeof(data_t),
                (data_t *)diff_dst->get_data_handle());
        fill_data<data_t>(
                diff_src->get_desc().get_size()/ sizeof(data_t),
                (data_t *)diff_src->get_data_handle());
        check_zero_tail<data_t>(1, *diff_dst);
        check_zero_tail<data_t>(1, *diff_src);

        pooling_backward(pool_bwd_prim_desc).execute(*strm, {
                {MKLDNN_ARG_DIFF_DST, *diff_dst},
                {MKLDNN_ARG_DIFF_SRC, *diff_src},
                {MKLDNN_ARG_WORKSPACE, *workspace}});

        check_zero_tail<data_t>(0, *diff_src);
        check_pool_bwd<data_t>(p, *diff_src, *diff_dst, *workspace);
    }
};

using pooling_bwd_test_float = pooling_bwd_test<float>;
using pool_bwd_test_params_float = pool_bwd_test_params;

#define EXPAND_SIZES_3D(...) 5, { __VA_ARGS__ }
#define EXPAND_SIZES_2D(mb,ic,ih,iw,oh,ow,kh,kw,padt,padl,strh,strw) \
    4, { mb,ic,1,ih,iw,1,oh,ow,1,kh,kw,0,padt,padl,1,strh,strw }

TEST_P(pooling_bwd_test_float, TestsPoolingBackward)
{
}

INSTANTIATE_TEST_SUITE_P(
        TestPoolingBackwardZeroDim, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw, EXPAND_SIZES_2D( 2, 0, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 )},
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw, EXPAND_SIZES_2D( 0, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 )},
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw, EXPAND_SIZES_2D( 2, 4, 0, 4, 4, 4, 3, 3, 1, 1, 1, 1 )}
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingBackwardEF, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw, EXPAND_SIZES_2D( 2, -4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ),
            true, mkldnn_invalid_arguments},
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw, EXPAND_SIZES_2D( -2, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ),
            true, mkldnn_invalid_arguments},
            pool_bwd_test_params_float{ engine::kind::cpu,
            eltwise_square, memory::format_tag::nchw,
            memory::format_tag::nchw, EXPAND_SIZES_2D( 2, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ),
            true, mkldnn_invalid_arguments}
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPooling_nChw16c_padded, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D(4, 17,  6,  6,  7,  7, 2, 2, 1, 1, 1, 1) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D(4, 23, 60, 60, 31, 31, 3, 4, 1, 1, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D(4, 14, 60, 60, 31, 31, 3, 2, 1, 1, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D(4, 17, 60, 60, 31, 31, 4, 3, 1, 1, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D(4, 14, 60, 60, 31, 31, 2, 3, 1, 1, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D(4, 28, 60, 60, 31, 31, 4, 2, 1, 1, 2, 2) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPooling_nChw8c_padded, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(4, 5,  6,  6,  7,  7, 2, 2, 1, 1, 1, 1) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(4, 23, 60, 60, 31, 31, 3, 4, 1, 1, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(4, 14, 60, 60, 31, 31, 3, 2, 1, 1, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(4, 17, 60, 60, 31, 31, 4, 3, 1, 1, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(4, 14, 60, 60, 31, 31, 2, 3, 1, 1, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(4, 28, 60, 60, 31, 31, 4, 2, 1, 1, 2, 2) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingBackwardMaxKernelSlipsToPadding, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw, EXPAND_SIZES_2D( 1, 16, 10, 10, 6, 6, 5, 5, 10, 10, 5, 5 ) },
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nhwc,
            memory::format_tag::nhwc, EXPAND_SIZES_2D( 1, 16, 10, 10, 6, 6, 5, 5, 10, 10, 5, 5 ) },
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 1, 16, 10, 10, 6, 6, 5, 5, 10, 10, 5, 5 ) },
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 1, 16, 10, 10, 6, 6, 5, 5, 10, 10, 5, 5 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPooling3D_nCdhw16c, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::nCdhw16c,
            memory::format_tag::nCdhw16c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 30, 30, 2, 3, 4, 1, 1, 1, 2, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::nCdhw16c,
            memory::format_tag::nCdhw16c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 30, 30, 31, 4, 3, 2, 1, 1, 1, 2, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::nCdhw16c,
            memory::format_tag::nCdhw16c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 30, 31, 30, 4, 2, 3, 1, 1, 1, 2, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::nCdhw16c,
            memory::format_tag::nCdhw16c, EXPAND_SIZES_3D(2, 32, 30, 30, 30, 30, 30, 30, 3, 3, 3, 1, 1, 1, 1, 1, 1) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::nCdhw16c,
            memory::format_tag::nCdhw16c, EXPAND_SIZES_3D(2, 32, 30, 30, 30, 30, 30, 30, 3, 3, 3, 1, 1, 1, 1, 1, 1) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::nCdhw16c,
            memory::format_tag::nCdhw16c, EXPAND_SIZES_3D(2, 32, 30, 30, 30, 30, 30, 30, 3, 3, 3, 1, 1, 1, 1, 1, 1) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPooling3D_ncdhw, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 30, 30, 2, 3, 4, 1, 1, 1, 2, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 30, 30, 31, 4, 3, 2, 1, 1, 1, 2, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 30, 31, 30, 4, 2, 3, 1, 1, 1, 2, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(2, 32, 30, 30, 30, 30, 30, 30, 3, 3, 3, 1, 1, 1, 1, 1, 1) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(2, 32, 30, 30, 30, 30, 30, 30, 3, 3, 3, 1, 1, 1, 1, 1, 1) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(2, 32, 30, 30, 30, 30, 30, 30, 3, 3, 3, 1, 1, 1, 1, 1, 1) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPooling3D_ndhwc, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 30, 30, 2, 3, 4, 1, 1, 1, 2, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 30, 30, 31, 4, 3, 2, 1, 1, 1, 2, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 30, 31, 30, 4, 2, 3, 1, 1, 1, 2, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(2, 32, 30, 30, 30, 30, 30, 30, 3, 3, 3, 1, 1, 1, 1, 1, 1) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(2, 32, 30, 30, 30, 30, 30, 30, 3, 3, 3, 1, 1, 1, 1, 1, 1) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(2, 32, 30, 30, 30, 30, 30, 30, 3, 3, 3, 1, 1, 1, 1, 1, 1) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPooling3D_nCdhw8c, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::nCdhw8c,
            memory::format_tag::nCdhw8c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 31, 30, 30, 2, 3, 4, 1, 1, 1, 2, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::nCdhw8c,
            memory::format_tag::nCdhw8c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 30, 30, 31, 4, 3, 2, 1, 1, 1, 2, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::nCdhw8c,
            memory::format_tag::nCdhw8c, EXPAND_SIZES_3D(2, 32, 60, 60, 60, 30, 31, 30, 4, 2, 3, 1, 1, 1, 2, 2, 2) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::nCdhw8c,
            memory::format_tag::nCdhw8c, EXPAND_SIZES_3D(2, 32, 30, 30, 30, 30, 30, 30, 3, 3, 3, 1, 1, 1, 1, 1, 1) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::nCdhw8c,
            memory::format_tag::nCdhw8c, EXPAND_SIZES_3D(2, 32, 30, 30, 30, 30, 30, 30, 3, 3, 3, 1, 1, 1, 1, 1, 1) },
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::nCdhw8c,
            memory::format_tag::nCdhw8c, EXPAND_SIZES_3D(2, 32, 30, 30, 30, 30, 30, 30, 3, 3, 3, 1, 1, 1, 1, 1, 1) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingBackwardMax3DunetNCDHW, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(1, 64,  64, 64, 64, 64, 64, 64, 2, 2, 2, 0, 0, 0, 1, 1, 1) },
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(1, 128, 28, 28, 28, 28, 28, 28, 2, 2, 2, 0, 0, 0, 1, 1, 1) },
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::ncdhw,
            memory::format_tag::ncdhw, EXPAND_SIZES_3D(1, 256, 12, 12, 12, 12, 12, 12, 2, 2, 2, 0, 0, 0, 1, 1, 1) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingBackwardMax3DunetNDHWC, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(1, 64,  64, 64, 64, 64, 64, 64, 2, 2, 2, 0, 0, 0, 1, 1, 1) },
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(1, 128, 28, 28, 28, 28, 28, 28, 2, 2, 2, 0, 0, 0, 1, 1, 1) },
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::ndhwc,
            memory::format_tag::ndhwc, EXPAND_SIZES_3D(1, 256, 12, 12, 12, 12, 12, 12, 2, 2, 2, 0, 0, 0, 1, 1, 1) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingBackwardMaxAlexNetNCHW, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw, EXPAND_SIZES_2D( 2, 16, 55, 55, 27, 27, 3, 3, 0, 0, 2, 2 ) },
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw, EXPAND_SIZES_2D( 2, 16, 27, 27, 13, 13, 3, 3, 0, 0, 2, 2 ) },
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw, EXPAND_SIZES_2D( 2, 16, 13, 13, 6, 6, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingBackwardMaxCIFAR10NCHW, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw, EXPAND_SIZES_2D( 2, 32, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) },
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw, EXPAND_SIZES_2D( 2, 32, 16, 16, 8, 8, 3, 3, 0, 0, 2, 2 ) },
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw, EXPAND_SIZES_2D( 2, 64, 8, 8, 4, 4, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingBackwardMax, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw, EXPAND_SIZES_2D( 1, 1, 2, 2, 1, 1, 2, 2, 0, 0, 1, 1 ) },
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw, EXPAND_SIZES_2D( 2, 2, 2, 2, 1, 1, 2, 2, 0, 0, 1, 1 ) },
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nchw,
            memory::format_tag::nchw, EXPAND_SIZES_2D( 2, 4, 4, 4, 4, 4, 3, 3, 1, 1, 1, 1 ) }
            ));


INSTANTIATE_TEST_SUITE_P(
        TestPoolingBackwardMaxBlocked, pooling_bwd_test_float, ::testing::Values(

            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 1, 8, 3, 3, 1, 1, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 8, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 32, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 32, 13, 13, 12, 12, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 32, 4, 4, 4, 4, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 32, 3, 3, 4, 4, 3, 3, 1, 1, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 32, 3, 3, 2, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 122, 32, 32, 2, 32, 2, 3, 3, 1, 1, 1, 1 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingBackwardAvgBlocked, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 8, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 8, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 32, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 32, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 32, 13, 13, 11, 11, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 32, 13, 13, 11, 11, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 32, 4, 4, 4, 4, 2, 2, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 32, 4, 4, 4, 4, 2, 2, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 32, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 32, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 32, 3, 3, 1, 1, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 32, 3, 3, 1, 1, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 122, 32, 32, 2, 32, 2, 3, 3, 1, 1, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 122, 32, 32, 2, 32, 2, 3, 3, 1, 1, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 122, 32, 32, 2, 32, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 122, 32, 32, 2, 32, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 32, 5, 5, 2, 2, 3, 3, 0, 0, 2, 2 ) }
            , pool_bwd_test_params_float{engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 32, 5, 5, 2, 2, 3, 3, 0, 0, 2, 2 ) }
            , pool_bwd_test_params_float{engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 8, 3, 2, 2, 2, 3, 3, 1, 1, 2, 1 ) }
            , pool_bwd_test_params_float{engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 2, 8, 3, 2, 2, 2, 3, 3, 1, 1, 2, 1 ) }

            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingBackwardMaxBlocked16, pooling_bwd_test_float, ::testing::Values(

            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 1, 16, 3, 3, 1, 1, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 16, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 32, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 32, 13, 13, 12, 12, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 32, 4, 4, 4, 4, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 32, 3, 3, 4, 4, 3, 3, 1, 1, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 32, 3, 3, 2, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 122, 32, 32, 2, 32, 2, 3, 3, 1, 1, 1, 1 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingBackwardAvgBlocked16, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 16, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 16, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 32, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 32, 4, 4, 2, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 32, 13, 13, 11, 11, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 32, 13, 13, 11, 11, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 32, 4, 4, 4, 4, 2, 2, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 32, 4, 4, 4, 4, 2, 2, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 32, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 32, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 32, 3, 3, 1, 1, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 32, 3, 3, 1, 1, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 122, 32, 32, 2, 32, 2, 3, 3, 1, 1, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 122, 32, 32, 2, 32, 2, 3, 3, 1, 1, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 122, 32, 32, 2, 32, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 122, 32, 32, 2, 32, 2, 3, 3, 0, 0, 1, 1 ) }
            , pool_bwd_test_params_float{engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 32, 5, 5, 2, 2, 3, 3, 0, 0, 2, 2 ) }
            , pool_bwd_test_params_float{engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 32, 5, 5, 2, 2, 3, 3, 0, 0, 2, 2 ) }
            , pool_bwd_test_params_float{engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 16, 3, 2, 2, 2, 3, 3, 1, 1, 2, 1 ) }
            , pool_bwd_test_params_float{engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 2, 16, 3, 2, 2, 2, 3, 3, 1, 1, 2, 1 ) }

            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingBackwardMaxBlockedPerf, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingBackwardAvgBlockedPerf, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingBackwardMaxBlocked16Perf, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_max, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingBackwardAvgBlocked16Perf, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_include_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            , pool_bwd_test_params_float{ engine::kind::cpu,
            pooling_avg_exclude_padding, memory::format_tag::nChw16c,
            memory::format_tag::nChw16c, EXPAND_SIZES_2D( 16, 64, 32, 32, 16, 16, 3, 3, 0, 0, 2, 2 ) }
            ));

INSTANTIATE_TEST_SUITE_P(
        TestPoolingBackwardAsymmPadding, pooling_bwd_test_float, ::testing::Values(
            pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 8, 3, 4, 1, 5, 3, 3, 0, 1, 1, 1) }
            ,pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 8, 3, 4, 1, 5, 3, 3, 0, 1, 1, 1) }
            ,pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 8, 3, 4, 1, 5, 3, 3, 0, 1, 1, 1) }

            ,pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 8, 3, 14, 1, 8, 3, 3, 0, 1, 1, 2) }
            ,pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 8, 3, 14, 1, 8, 3, 3, 0, 1, 1, 2) }
            ,pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 8, 3, 14, 1, 8, 3, 3, 0, 1, 1, 2) }

            ,pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 96, 3, 100, 1, 51, 3, 3, 0, 1, 1, 2) }
            ,pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 96, 3, 100, 1, 51, 3, 3, 0, 1, 1, 2) }
            ,pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 96, 3, 100, 1, 51, 3, 3, 0, 1, 1, 2) }

            ,pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 96, 3, 102, 1, 52, 3, 3, 0, 1, 1, 2) }
            ,pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 96, 3, 102, 1, 52, 3, 3, 0, 1, 1, 2) }
            ,pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 96, 3, 102, 1, 52, 3, 3, 0, 1, 1, 2) }

            ,pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 96, 9, 103, 7, 52, 3, 3, 0, 1, 1, 2) }
            ,pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 96, 9, 103, 7, 52, 3, 3, 0, 1, 1, 2) }
            ,pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 96, 9, 103, 7, 52, 3, 3, 0, 1, 1, 2) }

            ,pool_bwd_test_params_float{
            engine::kind::cpu, pooling_max, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 96, 300, 500, 151, 251, 3, 3, 1, 1, 2, 2) }
            ,pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_include_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 96, 300, 500, 151, 251, 3, 3, 1, 1, 2, 2) }
            ,pool_bwd_test_params_float{
            engine::kind::cpu, pooling_avg_exclude_padding, memory::format_tag::nChw8c,
            memory::format_tag::nChw8c, EXPAND_SIZES_2D(1, 96, 300, 500, 151, 251, 3, 3, 1, 1, 2, 2) }

            ));

}
