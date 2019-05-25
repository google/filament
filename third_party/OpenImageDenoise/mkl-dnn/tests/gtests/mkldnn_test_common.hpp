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

#ifndef MKLDNN_TEST_COMMON_HPP
#define MKLDNN_TEST_COMMON_HPP

#include <limits>
#include <numeric>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <stdint.h>

#include "gtest/gtest.h"

#if defined(_MSC_VER) && !defined(__clang__) && !defined(__INTEL_COMPILER)
#define collapse(x)
#endif

#include "mkldnn.hpp"

#include "src/common/mkldnn_thread.hpp"
#include "src/common/memory_desc_wrapper.hpp"

using memory = mkldnn::memory;

template <typename data_t> struct data_traits { };
template <> struct data_traits<float> {
    static const auto data_type = memory::data_type::f32;
};
template <> struct data_traits<uint8_t> {
    static const auto data_type = memory::data_type::u8;
};
template <> struct data_traits<int8_t> {
    static const auto data_type = memory::data_type::s8;
};
template <> struct data_traits<int32_t> {
    static const auto data_type = memory::data_type::s32;
};

template <typename T> inline void assert_eq(T a, T b);
template <> inline void assert_eq<float>(float a, float b) {
    ASSERT_FLOAT_EQ(a, b);
}

#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
inline int mxcsr_round(float f) { return _mm_cvtss_si32(_mm_load_ss(&f)); }
#else
inline int mxcsr_round(float f) { return (int)nearbyintf(f); }
#endif

template <typename data_t>
data_t out_round(float x) { return (data_t)mxcsr_round(x); }
template <>
float out_round<float>(float x) { return x; }

template <typename data_t, typename out_t>
out_t saturate(const out_t &x) {
    out_t v = x;
    if (v <= std::numeric_limits<data_t>::min())
        v = std::numeric_limits<data_t>::min();
    if (v > std::numeric_limits<data_t>::max())
        v = std::numeric_limits<data_t>::max();
    return v;
}

inline memory::dim right_padding(memory::dim i, memory::dim o, memory::dim k,
        memory::dim p, memory::dim s, memory::dim d = 0) {
    return (o - 1) * s + (k - 1) * (d + 1) - (p + i - 1);
}

template <typename data_t> struct acc_t { typedef data_t type; };
template<> struct acc_t<int8_t> { typedef int type; };
template<> struct acc_t<uint8_t> { typedef int type; };

// check_zero_tail - check on zero or set to zero padded memory
template <typename data_t>
void check_zero_tail(int set_zero_flag, memory &src) {

    data_t *src_data = (data_t *)src.get_data_handle();

    const memory::desc src_d = src.get_desc();
    const int ndims = src_d.data.ndims;
    const auto *dims = src_d.data.dims;
    const auto *pdims = src_d.data.padded_dims;
    const mkldnn::impl::memory_desc_wrapper mdw(src_d.data);

    memory::dim idx[MKLDNN_MAX_NDIMS] = {}, str[MKLDNN_MAX_NDIMS] = {};
    memory::dim nelems = 1;
    int tail_flag = 0;
    for (int i = 0; i < ndims; ++i) {
        if (dims[ndims-i-1] != pdims[ndims-i-1]) tail_flag = 1;
        nelems *= pdims[ndims-i-1];
        idx[i] = 0;
        str[i] = (i==0) ? 1 : str[i-1] * pdims[ndims-i];
    }
    if (tail_flag == 0) return;

    for (memory::dim i = 0; i < nelems; ++i) {
        memory::dim off = 0;
        bool flag = 0;
        for (int j = 0; j < ndims; ++j) {
            off += idx[j] * str[j];
            if (idx[j] >= dims[ndims-j-1]) flag = 1;
        }
        if (flag == 1) {
            memory::dim blk_off = mdw.off_l(off, true);
            if (set_zero_flag) {
                src_data[blk_off] = 0.0;
            } else {
                EXPECT_EQ(src_data[blk_off], 0.0) << " blk_off = " << blk_off
                << "off = " << off;
            }
        }
        /*Update idx*/
        for (int j = 0; j < ndims; ++j) {
            idx[j] ++;
            if (idx[j] < pdims[ndims-j-1]) break;
            idx[j] = 0;
        }
    }
}

inline memory::desc create_md(memory::dims dims,
        memory::data_type data_type, memory::format_tag fmt_tag) {
    return memory::desc(dims, data_type, fmt_tag);
}

template <typename data_t>
static inline data_t set_value(memory::dim index, data_t mean, data_t deviation,
        double sparsity)
{
    if (data_traits<data_t>::data_type == memory::data_type::f32) {
        const memory::dim group_size = (memory::dim)(1. / sparsity);
        const memory::dim group = index / group_size;
        const memory::dim in_group = index % group_size;
        const bool fill = in_group == ((group % 1637) % group_size);
        return fill ? static_cast<data_t>(mean + deviation * sinf(float(index % 37)))
            : data_t{0};
    } else if (data_traits<data_t>::data_type == memory::data_type::s32
        || data_traits<data_t>::data_type == memory::data_type::s8) {
        return data_t(rand() % 21 - 10);
    } else if (data_traits<data_t>::data_type == memory::data_type::u8) {
        return data_t(rand() % 17);
    } else {
        return data_t(0);
    }
}

template <typename data_t>
static void fill_data(const memory::dim size, data_t *data, data_t mean,
        data_t deviation, double sparsity = 1.)
{
    mkldnn::impl::parallel_nd(size, [&](memory::dim n) {
            data[n] = set_value<data_t>(n, mean, deviation, sparsity);
    });
}

template <typename data_t>
static void fill_data(const memory::dim size, data_t *data,
        double sparsity = 1., bool init_negs = false)
{
    mkldnn::impl::parallel_nd(size, [&](memory::dim n) {
        data[n] = set_value<data_t>(n, data_t(1), data_t(2e-1), sparsity);

        if (init_negs && n%4 == 0)
            data[n] = static_cast<data_t>(-data[n]); // weird for unsigned types!
    });
}

template <typename data_t>
static void compare_data(memory& ref, memory& dst,
        data_t threshold = (data_t)1e-4)
{
    using data_type = memory::data_type;

    ASSERT_TRUE(data_traits<data_t>::data_type == data_type::f32 ||
            data_traits<data_t>::data_type == data_type::s32);

    /* Note: size_t incompatible with MSVC++ */
    auto ref_desc = ref.get_desc();
    auto dst_desc = dst.get_desc();
    const mkldnn::impl::memory_desc_wrapper mdw_ref(ref_desc.data);
    const mkldnn::impl::memory_desc_wrapper mdw_dst(dst_desc.data);

    ASSERT_TRUE(ref_desc.data.ndims == dst_desc.data.ndims);

    auto ndims = ref_desc.data.ndims;

    for (auto d = 0; d < ndims; ++d) {
        ASSERT_TRUE(ref_desc.data.dims[d] == dst_desc.data.dims[d]);
    }

    auto dims = ref_desc.data.dims;

    memory::dim num = 1;
    for (auto d = 0; d < ndims; ++d) {
        num *= dims[d];
    }

    data_t *ref_data = (data_t *)ref.get_data_handle();
    data_t *dst_data = (data_t *)dst.get_data_handle();

    mkldnn::impl::parallel_nd(num, [&](memory::dim i) {
        data_t ref = ref_data[mdw_ref.off_l(i, true)];
        data_t got = dst_data[mdw_dst.off_l(i, true)];

        if (data_traits<data_t>::data_type == data_type::f32) {
            data_t diff = got - ref;
            data_t e = (std::abs(ref) > threshold) ? diff / ref : diff;
            EXPECT_NEAR(e, (data_t)0.0, threshold)
                << "Index: " << i << " Total: " << num;
        } else {
            EXPECT_EQ(ref, got) << "Index: " << i << " Total: " << num;
        }
    });
}

inline const char *query_impl_info(const_mkldnn_primitive_desc_t pd) {
    const char *str;
    mkldnn_primitive_desc_query(pd, mkldnn_query_impl_info_str, 0, &str);
    return str;
};

mkldnn_status_t get_conv_impl_status(const_mkldnn_primitive_desc_t pd, const char *match_str){
    const char* conv_str = query_impl_info(pd);

    if( strstr(conv_str, match_str) != NULL)
        return mkldnn_status_t::mkldnn_success;
    return mkldnn_status_t::mkldnn_unimplemented;
};

struct test_convolution_sizes_t {
    test_convolution_sizes_t(
        memory::dim mb,
        memory::dim ng,
        memory::dim ic, memory::dim ih, memory::dim iw,
        memory::dim oc, memory::dim oh, memory::dim ow,
        memory::dim kh, memory::dim kw,
        memory::dim padh, memory::dim padw,
        memory::dim strh, memory::dim strw,
        memory::dim dilh=0, memory::dim dilw=0
    ) :
        mb(mb),
        ng(ng),
        ic(ic), ih(ih), iw(iw),
        oc(oc), oh(oh), ow(ow),
        kh(kh), kw(kw),
        padh(padh), padw(padw),
        strh(strh), strw(strw),
        dilh(dilh), dilw(dilw) {}
    memory::dim mb;
    memory::dim ng;
    memory::dim ic, ih, iw;
    memory::dim oc, oh, ow;
    memory::dim kh, kw;
    memory::dim padh, padw;
    memory::dim strh, strw;
    memory::dim dilh, dilw;
};

struct test_convolution_attr_t {
    struct scale_t {
        enum policy_t { NONE = 0, COMMON };

        bool is_def() const { return policy != NONE; }

        scale_t (float s, policy_t p = NONE) :
            scale(s) { policy = p; }

        policy_t policy;
        float scale;
    };

    void mkldnn_attr_recreate() {
        mkl_attr = mkldnn::primitive_attr();
        if (oscale.is_def()) {
            const memory::dim count = 1;
            const int mask = 0;
            std::vector<float> s(count, oscale.scale);
            mkl_attr.set_output_scales(mask, s);
        }
    }

    test_convolution_attr_t(float s,
            scale_t::policy_t p = scale_t::policy_t::NONE)
        : oscale(s, p), mkl_attr() {}

    test_convolution_attr_t(): test_convolution_attr_t(1.f) {}

    scale_t oscale;
    mkldnn::primitive_attr mkl_attr;
};

struct test_convolution_formats_t {
    memory::format_tag src_format;
    memory::format_tag weights_format;
    memory::format_tag bias_format;
    memory::format_tag dst_format;
};

struct test_convolution_params_t {
    const mkldnn::engine::kind engine_kind;
    mkldnn::algorithm aalgorithm;
    test_convolution_formats_t formats;
    test_convolution_attr_t attr;
    test_convolution_sizes_t sizes;
    bool expect_to_fail;
    mkldnn_status_t expected_status;
};

struct test_convolution_eltwise_params_t {
    const mkldnn::algorithm alg;
    const mkldnn::engine::kind engine_kind;
    mkldnn::algorithm aalgorithm;
    const float eltwise_alpha;
    const float eltwise_beta;
    test_convolution_formats_t formats;
    test_convolution_attr_t attr;
    test_convolution_sizes_t sizes;
    bool expect_to_fail;
    mkldnn_status_t expected_status;
};

template<typename F> bool catch_expected_failures(const F &f,
        bool expect_to_fail, mkldnn_status_t expected_status, bool ignore_unimplemented = true)
{
    try {
        f();
    } catch (const mkldnn::error &e) {
        // Rethrow the exception if it is not expected or the error status did
        // not match.
        if (!(expect_to_fail) || e.status != (expected_status)) {
            // Ignore unimplemented
            if ( ignore_unimplemented && (e.status == mkldnn_unimplemented))
                return true;
            else
                throw e;
        }
        // Return normally if the failure is expected
        if (expect_to_fail)
            return true;
    }

    // Throw an exception if the failure is expected but did not happen
    if (expect_to_fail)
        throw std::exception();

    return false;
}

#define TEST_MALLOC_OFFSET 8
char *test_malloc(size_t size) {
    void *ptr;
    const size_t align = 64;
    const size_t padded_size = TEST_MALLOC_OFFSET + size;
#ifdef _WIN32
    ptr = _aligned_malloc(padded_size, align);
    int rc = ((ptr) ? 0 : errno);
#else
    int rc = ::posix_memalign(&ptr, align, padded_size);
#endif /* _WIN32 */
    return rc == 0 ? (char*)ptr + TEST_MALLOC_OFFSET: 0;
}

void test_free(char *ptr) {
    char *base_ptr = ptr - TEST_MALLOC_OFFSET;
#ifdef _WIN32
    _aligned_free(base_ptr);
#else
    return ::free(base_ptr);
#endif /* _WIN32 */
}
#undef TEST_MALLOC_OFFSET

class test_memory {
public:
    test_memory(const memory::desc &d, const mkldnn::engine &e) {
        size_ = d.get_size();
        data_.reset(test_malloc(size_), test_free);
        mem_.reset(new memory(d, e, data_.get()));
    }
    size_t get_size() const { return size_; }
    memory &get() { return *mem_; }

private:
    std::shared_ptr<memory> mem_;
    std::shared_ptr<char> data_;
    size_t size_;
};

#endif
