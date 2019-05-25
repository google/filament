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

#include <utility>
#include <numeric>

#include "gtest/gtest.h"
#include "mkldnn_test_common.hpp"

#include "mkldnn.hpp"

namespace mkldnn {

template <typename data_i_t, typename data_o_t>
inline void check_reorder(const memory::desc &md_i, const memory::desc &md_o,
        const data_i_t *src, const data_o_t *dst)
{
    const auto ndims = md_i.data.ndims;
    const auto *dims = md_i.data.dims;
    const size_t nelems = std::accumulate(
            dims, dims + ndims, size_t(1), std::multiplies<size_t>());

    const mkldnn::impl::memory_desc_wrapper mdw_i(md_i.data);
    const mkldnn::impl::memory_desc_wrapper mdw_o(md_o.data);
    for (size_t i = 0; i < nelems; ++i) {
        data_i_t s_raw = src[mdw_i.off_l(i, false)];
        data_o_t s = static_cast<data_o_t>(s_raw);
        data_o_t d = dst[mdw_o.off_l(i, false)];
        ASSERT_EQ(s, d) << "mismatch at position " << i;
    }
}

template <typename reorder_types>
struct test_simple_params {
    engine::kind engine_kind;
    memory::format_tag fmt_i;
    memory::format_tag fmt_o;
    memory::dims dims;
    bool expect_to_fail;
    mkldnn_status_t expected_status;
};

template <typename reorder_types>
class reorder_simple_test:
    public ::testing::TestWithParam<test_simple_params<reorder_types>>
{
protected:
    virtual void SetUp() {
        test_simple_params<reorder_types> p
            = ::testing::TestWithParam<decltype(p)>::GetParam();
        catch_expected_failures([=](){Test();}, p.expect_to_fail,
                    p.expected_status);
    }

    void Test() {
        using data_i_t = typename reorder_types::first_type;
        using data_o_t = typename reorder_types::second_type;

        test_simple_params<reorder_types> p
            = ::testing::TestWithParam<decltype(p)>::GetParam();

        ASSERT_TRUE(p.engine_kind == engine::kind::cpu);
        auto eng = engine(p.engine_kind, 0);
        auto strm = stream(eng);

        const size_t nelems = std::accumulate(p.dims.begin(), p.dims.end(),
                size_t(1), std::multiplies<size_t>());

        memory::data_type prec_i = data_traits<data_i_t>::data_type;
        memory::data_type prec_o = data_traits<data_o_t>::data_type;
        auto md_i = memory::desc(p.dims, prec_i, p.fmt_i);
        auto md_o = memory::desc(p.dims, prec_o, p.fmt_o);

        auto src_data = new data_i_t[md_i.get_size()];
        auto dst_data = new data_o_t[md_o.get_size()];

        /* initialize input data */
        const mkldnn::impl::memory_desc_wrapper mdw_i(md_i.data);
        for (size_t i = 0; i < nelems; ++i)
            src_data[mdw_i.off_l(i, false)] = data_i_t(i);

        auto src = memory(md_i, eng, src_data);
        auto dst = memory(md_o, eng, dst_data);

        reorder(src, dst).execute(strm, src, dst);

        check_reorder(md_i, md_o, src_data, dst_data);
        check_zero_tail<data_o_t>(0, dst);

        delete[] src_data;
        delete[] dst_data;
    }
};

using f32_f32 = std::pair<float, float>;
using s32_s32 = std::pair<int32_t, int32_t>;
using s8_s8 = std::pair<int8_t, int8_t>;

using reorder_simple_corner_cases_f32_f32 = reorder_simple_test<f32_f32>;
using reorder_padded_test_data_f32_f32 = reorder_simple_test<f32_f32>;
using reorder_padded_test_weights_f32_f32 = reorder_simple_test<f32_f32>;
using reorder_3d_test_data_f32_f32 = reorder_simple_test<f32_f32>;
using reorder_3d_test_weights_f32_f32 = reorder_simple_test<f32_f32>;
using reorder_simple_test_data_f32_f32 = reorder_simple_test<f32_f32>;
using reorder_simple_test_weights_f32_f32_0 = reorder_simple_test<f32_f32>;
using reorder_simple_test_weights_f32_f32_1 = reorder_simple_test<f32_f32>;
using reorder_simple_test_weights_f32_f32_IOhw16o16i = reorder_simple_test<f32_f32>;
using reorder_simple_test_s32_s32 = reorder_simple_test<s32_s32>;
using reorder_simple_test_s8_s8 = reorder_simple_test<s8_s8>;

using eng = engine::kind;
using fmt = memory::format_tag;

using test_simple_params_s32_s32 = test_simple_params<s32_s32>;
using test_simple_params_f32_f32 = test_simple_params<f32_f32>;
using test_simple_params_s8_s8 = test_simple_params<s8_s8>;

using cfg_f32= test_simple_params_f32_f32;
using cfg_s32= test_simple_params_s32_s32;
using cfg_s8= test_simple_params_s8_s8;

TEST_P(reorder_simple_corner_cases_f32_f32, TestsReorder) { }
INSTANTIATE_TEST_SUITE_P(TestReorder, reorder_simple_corner_cases_f32_f32,
        ::testing::Values(
            cfg_f32{eng::cpu, fmt::nchw, fmt::nc, {2, 16, 8, 8}, true, mkldnn_invalid_arguments},
            cfg_f32{eng::cpu, fmt::nchw, fmt::nchw, {0, 16, 8, 8}},
            cfg_f32{eng::cpu, fmt::nchw, fmt::nChw8c, {0, 5, 8, 8}},
            cfg_f32{eng::cpu, fmt::nchw, fmt::nChw16c, {0, 5, 8, 8}},
            cfg_f32{eng::cpu, fmt::OIhw8o8i, fmt::oihw, {13, 0, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIhw8i8o, fmt::OIhw8o8i, {0, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIhw16o16i, fmt::oihw, {16, 31, 0, 3}},
            cfg_f32{eng::cpu, fmt::OIhw16i16o, fmt::OIhw16o16i, {32, 16, 3, 0}}
            )
        );

TEST_P(reorder_padded_test_data_f32_f32, TestsReorder) { }
INSTANTIATE_TEST_SUITE_P(TestReorder, reorder_padded_test_data_f32_f32,
        ::testing::Values(
            cfg_f32{eng::cpu, fmt::nchw, fmt::nChw8c, {2, 28, 3, 4}},
            cfg_f32{eng::cpu, fmt::nChw8c, fmt::nchw, {2, 28, 3, 4}},
            cfg_f32{eng::cpu, fmt::chwn, fmt::nChw8c, {2, 28, 3, 4}},
            cfg_f32{eng::cpu, fmt::nChw8c, fmt::chwn, {2, 28, 3, 4}},
            cfg_f32{eng::cpu, fmt::nhwc, fmt::nChw8c, {3, 28, 3, 4}},
            cfg_f32{eng::cpu, fmt::nChw8c, fmt::nhwc, {3, 28, 3, 4}},

            cfg_f32{eng::cpu, fmt::nchw, fmt::nChw16c, {2, 28, 3, 4}},
            cfg_f32{eng::cpu, fmt::nChw16c, fmt::nchw, {2, 28, 3, 4}},
            cfg_f32{eng::cpu, fmt::chwn, fmt::nChw16c, {2, 28, 3, 4}},
            cfg_f32{eng::cpu, fmt::nChw16c, fmt::chwn, {2, 28, 3, 4}},
            cfg_f32{eng::cpu, fmt::nhwc, fmt::nChw16c, {3, 28, 3, 4}},
            cfg_f32{eng::cpu, fmt::nChw16c, fmt::nhwc, {3, 28, 3, 4}},

            cfg_f32{eng::cpu, fmt::ncdhw, fmt::nCdhw16c, {2, 28, 2, 3, 4}},
            cfg_f32{eng::cpu, fmt::nCdhw16c, fmt::ncdhw, {2, 28, 2, 3, 4}},
            // cfg_f32{eng::cpu, fmt::cdhwn, fmt::nCdhw16c, {2, 28, 2, 3, 4}},
            // cfg_f32{eng::cpu, fmt::nCdhw16c, fmt::cdhwn, {2, 28, 2, 3, 4}},
            cfg_f32{eng::cpu, fmt::ndhwc, fmt::nCdhw16c, {3, 28, 2, 3, 4}},
            cfg_f32{eng::cpu, fmt::nCdhw16c, fmt::ndhwc, {3, 28, 2, 3, 4}}
            )
        );

TEST_P(reorder_3d_test_data_f32_f32, TestsReorder) { }
INSTANTIATE_TEST_SUITE_P(TestReorder, reorder_3d_test_data_f32_f32,
        ::testing::Values(
            cfg_f32{eng::cpu, fmt::ncdhw, fmt::nCdhw16c, {2, 32, 2, 3, 4}},
            cfg_f32{eng::cpu, fmt::nCdhw16c, fmt::ncdhw, {2, 32, 2, 3, 4}},
            cfg_f32{eng::cpu, fmt::nCdhw8c, fmt::ncdhw, {2, 32, 2, 3, 4}},
            cfg_f32{eng::cpu, fmt::ndhwc, fmt::nCdhw16c, {3, 32, 2, 3, 4}},
            cfg_f32{eng::cpu, fmt::nCdhw16c, fmt::ndhwc, {3, 32, 2, 3, 4}},
            cfg_f32{eng::cpu, fmt::ndhwc, fmt::nCdhw8c, {3, 32, 2, 3, 4}},
            cfg_f32{eng::cpu, fmt::nCdhw8c, fmt::ndhwc, {3, 32, 2, 3, 4}}
            )
        );

TEST_P(reorder_padded_test_weights_f32_f32, TestsReorder) { }
INSTANTIATE_TEST_SUITE_P(TestReorder, reorder_padded_test_weights_f32_f32,
        ::testing::Values(
            // Oi(d)hw16o
            cfg_f32{eng::cpu, fmt::oihw, fmt::Oihw16o, {17, 23, 2, 3}},
            cfg_f32{eng::cpu, fmt::Oihw16o, fmt::oihw, {17, 23, 2, 3}},
            cfg_f32{eng::cpu, fmt::oidhw, fmt::Oidhw16o, {17, 23, 2, 2, 3}},
            cfg_f32{eng::cpu, fmt::Oidhw16o, fmt::oidhw, {17, 23, 2, 2, 3}},
            // OIhw16i16o
            cfg_f32{eng::cpu, fmt::oihw, fmt::OIhw16i16o, {17, 23, 2, 3}},
            cfg_f32{eng::cpu, fmt::OIhw16i16o, fmt::oihw, {17, 23, 2, 3}},
            cfg_f32{eng::cpu, fmt::oihw, fmt::OIhw16o16i, {17, 23, 2, 3}},
            cfg_f32{eng::cpu, fmt::OIhw16o16i, fmt::oihw, {17, 23, 2, 3}},
            cfg_f32{eng::cpu, fmt::hwio, fmt::OIhw16i16o, {17, 23, 2, 3}},
            cfg_f32{eng::cpu, fmt::OIhw16i16o, fmt::hwio, {17, 23, 2, 3}},
            // OIdhw16o16i
            cfg_f32{eng::cpu, fmt::oihw, fmt::OIhw16o16i, {17, 23, 2, 3}},
            cfg_f32{eng::cpu, fmt::OIhw16o16i, fmt::oihw, {17, 23, 2, 3}},
            // IOdhw16o16i
            cfg_f32{eng::cpu, fmt::oihw, fmt::IOhw16o16i, {17, 23, 2, 3}},
            cfg_f32{eng::cpu, fmt::IOhw16o16i, fmt::oihw, {17, 23, 2, 3}},
            // gOdhwi16o
            cfg_f32{eng::cpu, fmt::goidhw, fmt::gOdhwi16o, {2, 17, 23, 2, 2, 3}},
            cfg_f32{eng::cpu, fmt::gOdhwi16o, fmt::goidhw, {2, 17, 23, 3, 2, 3}},
            // gOIdhw16i16o
            cfg_f32{eng::cpu, fmt::goidhw, fmt::gOIdhw16i16o, {2, 17, 23, 2, 2, 3}},
            cfg_f32{eng::cpu, fmt::gOIdhw16i16o, fmt::goidhw, {2, 17, 23, 3, 2, 3}},
            // gOIdhw16o16i
            cfg_f32{eng::cpu, fmt::goidhw, fmt::gOIdhw16o16i, {2, 17, 23, 2, 2, 3}},
            cfg_f32{eng::cpu, fmt::gOIdhw16o16i, fmt::goidhw, {2, 17, 23, 3, 2, 3}},
            // Oihw16o
            cfg_f32{eng::cpu, fmt::oihw, fmt::Oihw16o, {17, 23, 2, 3}},
            cfg_f32{eng::cpu, fmt::Oihw16o, fmt::oihw, {17, 23, 2, 3}},
            // OIhw8i8o
            cfg_f32{eng::cpu, fmt::oihw, fmt::OIhw8i8o, {17, 23, 2, 3}},
            cfg_f32{eng::cpu, fmt::OIhw8i8o, fmt::oihw, {17, 23, 2, 3}},
            cfg_f32{eng::cpu, fmt::oihw, fmt::OIhw8o8i, {17, 23, 2, 3}},
            cfg_f32{eng::cpu, fmt::OIhw8o8i, fmt::oihw, {17, 23, 2, 3}},
            cfg_f32{eng::cpu, fmt::hwio, fmt::OIhw8i8o, {17, 23, 2, 3}},
            cfg_f32{eng::cpu, fmt::OIhw8i8o, fmt::hwio, {17, 23, 2, 3}}
));

TEST_P(reorder_3d_test_weights_f32_f32, TestsReorder) { }
INSTANTIATE_TEST_SUITE_P(TestReorder, reorder_3d_test_weights_f32_f32,
        ::testing::Values(
            cfg_f32{eng::cpu, fmt::oidhw, fmt::OIdhw8i8o, {16, 24, 2, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIdhw8i8o, fmt::oidhw, {16, 24, 2, 3, 3}},
            cfg_f32{eng::cpu, fmt::oidhw, fmt::OIdhw8o8i, {16, 24, 2, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIdhw8o8i, fmt::oidhw, {16, 24, 2, 3, 3}},
            cfg_f32{eng::cpu, fmt::dhwio, fmt::OIdhw8i8o, {16, 24, 2, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIdhw8i8o, fmt::dhwio, {16, 24, 2, 3, 3}},
            cfg_f32{eng::cpu, fmt::goidhw, fmt::gOdhwi8o, {2, 16, 24, 2, 2, 3}},
            cfg_f32{eng::cpu, fmt::gOdhwi8o, fmt::goidhw, {2, 16, 24, 3, 2, 3}},
            cfg_f32{eng::cpu, fmt::goidhw, fmt::gOIdhw8i8o, {2, 16, 24, 2, 2, 3}},
            cfg_f32{eng::cpu, fmt::gOIdhw8i8o, fmt::goidhw, {2, 16, 24, 3, 2, 3}},
            cfg_f32{eng::cpu, fmt::goidhw, fmt::gOIdhw8o8i, {2, 16, 24, 2, 2, 3}},
            cfg_f32{eng::cpu, fmt::gOIdhw8o8i, fmt::goidhw, {2, 16, 24, 3, 2, 3}}
));

TEST_P(reorder_simple_test_data_f32_f32, TestsReorder) { }
INSTANTIATE_TEST_SUITE_P(TestReorder, reorder_simple_test_data_f32_f32,
        ::testing::Values(
            cfg_f32{eng::cpu, fmt::nchw, fmt::nchw, {10, 10, 13, 13}},
            cfg_f32{eng::cpu, fmt::nchw, fmt::nhwc, {10, 10, 10, 10}},
            cfg_f32{eng::cpu, fmt::nhwc, fmt::nchw, {10, 10, 10, 10}},
            cfg_f32{eng::cpu, fmt::nchw, fmt::chwn, {28, 3, 10, 10}},
            cfg_f32{eng::cpu, fmt::chwn, fmt::nchw, {28, 3, 10, 10}},
            cfg_f32{eng::cpu, fmt::nhwc, fmt::nhwc, {10, 10, 13, 13}},
            cfg_f32{eng::cpu, fmt::nchw, fmt::nChw8c, {2, 32, 4, 4}},
            cfg_f32{eng::cpu, fmt::nChw8c, fmt::nchw, {2, 32, 4, 4}},
            cfg_f32{eng::cpu, fmt::chwn, fmt::nChw8c, {28, 96, 10, 10}},
            cfg_f32{eng::cpu, fmt::nChw8c, fmt::chwn, {28, 96, 10, 10}},
            cfg_f32{eng::cpu, fmt::nhwc, fmt::nChw8c, {3, 64, 16, 16}},
            cfg_f32{eng::cpu, fmt::nChw8c, fmt::nhwc, {3, 64, 16, 16}},
            cfg_f32{eng::cpu, fmt::nChw8c, fmt::nChw16c, {10, 96, 27, 27}},
            cfg_f32{eng::cpu, fmt::nChw16c, fmt::nChw8c, {10, 96, 27, 27}},
            cfg_f32{eng::cpu, fmt::nchw, fmt::nChw16c, {2, 64, 4, 4}},
            cfg_f32{eng::cpu, fmt::nChw16c, fmt::nchw, {2, 64, 4, 4}},
            cfg_f32{eng::cpu, fmt::chwn, fmt::nChw16c, {28, 96, 10, 10}},
            cfg_f32{eng::cpu, fmt::nChw16c, fmt::chwn, {28, 96, 10, 10}},
            cfg_f32{eng::cpu, fmt::nhwc, fmt::nChw16c, {2, 64, 4, 4}},
            cfg_f32{eng::cpu, fmt::nChw16c, fmt::nhwc, {2, 64, 4, 4}}
            )
        );

TEST_P(reorder_simple_test_weights_f32_f32_0, TestsReorder) { }
INSTANTIATE_TEST_SUITE_P(TestReorder, reorder_simple_test_weights_f32_f32_0,
        ::testing::Values(
            cfg_f32{eng::cpu, fmt::hwio, fmt::oihw, {32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::oihw, fmt::hwio, {32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::hwio, fmt::Ohwi8o, {32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::Ohwi8o, fmt::hwio, {32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::hwio, fmt::Ohwi16o, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::Ohwi16o, fmt::hwio, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::oihw, fmt::OIhw8i8o, {32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIhw8i8o, fmt::oihw, {32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::ihwo, fmt::OIhw8i8o, {32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIhw8i8o, fmt::ihwo, {32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::oihw, fmt::OIhw8o8i, {32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIhw8o8i, fmt::oihw, {32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIhw8i8o, fmt::OIhw8o8i, {32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIhw8o8i, fmt::OIhw8i8o, {32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::hwio, fmt::OIhw8i8o, {32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIhw8i8o, fmt::hwio, {32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::goihw, fmt::hwigo, {2, 32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::hwigo, fmt::goihw, {2, 32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::goihw, fmt::gOIhw8i8o, {2, 32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::gOIhw8i8o, fmt::goihw, {2, 32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::goihw, fmt::gOIhw8o8i, {2, 32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::gOIhw8o8i, fmt::goihw, {2, 32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::gOIhw8i8o, fmt::gOIhw8o8i, {2, 32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::gOIhw8o8i, fmt::gOIhw8i8o, {2, 32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::oihw, fmt::OIhw16i16o, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIhw16i16o, fmt::oihw, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::ihwo, fmt::OIhw16i16o, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIhw16i16o, fmt::ihwo, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::oihw, fmt::OIhw16o16i, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIhw16o16i, fmt::oihw, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::hwio, fmt::OIhw16i16o, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIhw16i16o, fmt::hwio, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::goihw, fmt::gOIhw16i16o, {2, 64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::gOIhw16i16o, fmt::goihw, {2, 64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::goihw, fmt::gOIhw16o16i, {2, 64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::gOIhw16o16i, fmt::goihw, {2, 64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIhw16i16o, fmt::OIhw16o16i, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIhw16o16i, fmt::OIhw16i16o, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::gOIhw16i16o, fmt::gOIhw16o16i, {2, 64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::gOIhw16o16i, fmt::gOIhw16i16o, {2, 64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::oihw, fmt::Oihw16o, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::Oihw16o, fmt::oihw, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::goihw, fmt::gOihw16o, {2, 64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::gOihw16o, fmt::goihw, {2, 64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::Ohwi16o, fmt::Oihw16o, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::Oihw16o, fmt::Ohwi16o, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::gOhwi16o, fmt::gOihw16o, {2, 64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::gOihw16o, fmt::gOhwi16o, {2, 64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::goihw, fmt::Goihw8g, {16, 16, 16, 3, 3}},
            cfg_f32{eng::cpu, fmt::Goihw8g, fmt::goihw, {16, 16, 16, 3, 3}}
            )
        );

TEST_P(reorder_simple_test_weights_f32_f32_1, TestsReorder) { }
INSTANTIATE_TEST_SUITE_P(TestReorder, reorder_simple_test_weights_f32_f32_1,
        ::testing::Values(
            cfg_f32{eng::cpu, fmt::goihw, fmt::Goihw16g, {32, 32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::Goihw16g, fmt::goihw, {32, 32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::oihw, fmt::iohw, {32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::iohw, fmt::oihw, {32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::goihw, fmt::giohw, {2, 32, 32, 3, 3}},
            cfg_f32{eng::cpu, fmt::giohw, fmt::goihw, {2, 32, 32, 3, 3}}
            )
        );

TEST_P(reorder_simple_test_weights_f32_f32_IOhw16o16i, TestsReorder) { }
INSTANTIATE_TEST_SUITE_P(TestReorder, reorder_simple_test_weights_f32_f32_IOhw16o16i,
        ::testing::Values(
            cfg_f32{eng::cpu, fmt::oihw, fmt::IOhw16o16i, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::IOhw16o16i, fmt::oihw, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::OIhw16i16o, fmt::IOhw16o16i, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::IOhw16o16i, fmt::OIhw16i16o, {64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::goihw, fmt::gOIhw16o16i, {2, 64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::gIOhw16o16i, fmt::goihw, {2, 64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::gOIhw16i16o, fmt::gIOhw16o16i, {2, 64, 64, 3, 3}},
            cfg_f32{eng::cpu, fmt::gIOhw16o16i, fmt::gOIhw16i16o, {2, 64, 64, 3, 3}}
            )
        );


TEST_P(reorder_simple_test_s32_s32, TestsReorder) { }
INSTANTIATE_TEST_SUITE_P(TestReorder, reorder_simple_test_s32_s32,
        ::testing::Values(
            cfg_s32{eng::cpu, fmt::nchw, fmt::nChw16c, {2, 64, 4, 4}},
            cfg_s32{eng::cpu, fmt::nChw16c, fmt::nchw, {2, 64, 4, 4}}
            )
        );

TEST_P(reorder_simple_test_s8_s8, TestsReorder) { }
INSTANTIATE_TEST_SUITE_P(TestReorder, reorder_simple_test_s8_s8,
        ::testing::Values(
            cfg_s8{eng::cpu, fmt::oihw, fmt::OIhw4i16o4i, {64, 64, 3, 3}},
            cfg_s8{eng::cpu, fmt::OIhw4i16o4i, fmt::oihw, {64, 64, 3, 3}},
            cfg_s8{eng::cpu, fmt::goihw, fmt::gOIhw4i16o4i, {2, 64, 64, 3, 3}},
            cfg_s8{eng::cpu, fmt::gOIhw4i16o4i, fmt::goihw, {2, 64, 64, 3, 3}}
            )
        );
}
