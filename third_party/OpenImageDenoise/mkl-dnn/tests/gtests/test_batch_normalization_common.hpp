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

#define ENGINE engine::kind::cpu
#define INST_TEST_CASE(str, ...) INSTANTIATE_TEST_SUITE_P( \
        str, bnorm_test, ::testing::Values(__VA_ARGS__));

namespace mkldnn {

struct test_bnorm_sizes_t {
    memory::dim mb, c, d, h, w;
};

struct test_bnorm_format_tags_t {
    memory::format_tag data_tag;
    memory::format_tag diff_tag;
};

struct test_bnorm_params_t {
    mkldnn::engine::kind engine_kind;
    test_bnorm_format_tags_t tags;
    test_bnorm_sizes_t sizes;
    float epsilon;
    int ndims;
    bool expect_to_fail;
    mkldnn_status_t expected_status;
};

template <typename T> void fill(memory &m) {
    auto numElements = m.get_desc().get_size() / sizeof(T);
    T *dataPtr = reinterpret_cast<T *>(m.get_data_handle());
    fill_data<T>(numElements, dataPtr);
}

template <typename data_t>
class bnorm_test_common : public ::testing::TestWithParam<test_bnorm_params_t> {
private:
    std::shared_ptr<test_memory> src;
    std::shared_ptr<test_memory> dst;
    std::shared_ptr<test_memory> diff_src;
    std::shared_ptr<test_memory> diff_dst;
    std::shared_ptr<memory> weights;
    std::shared_ptr<memory> diff_weights;
    std::shared_ptr<memory> mean;
    std::shared_ptr<memory> variance;
    std::shared_ptr<memory::desc> data_d;
    std::shared_ptr<memory::desc> diff_d;
    std::shared_ptr<batch_normalization_forward::primitive_desc> bnorm_fwd_pd;
    std::shared_ptr<batch_normalization_backward::primitive_desc> bnorm_bwd_pd;
    test_bnorm_params_t p;
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
        p = ::testing::TestWithParam<decltype(p)>::GetParam();

        ASSERT_TRUE(p.engine_kind == engine::kind::cpu);
        eng.reset(new engine(p.engine_kind, 0));
        strm.reset(new stream(*eng));

        memory::data_type data_type = data_traits<data_t>::data_type;
        ASSERT_TRUE(isF32(data_type) || isS8(data_type));

        test_bnorm_sizes_t bs = p.sizes;
        bool has_spatial = (p.tags.data_tag != mkldnn_nc);
        if (has_spatial) {
            if (p.ndims == 5) {
                data_d.reset(new memory::desc({ bs.mb, bs.c, bs.d, bs.h, bs.w },
                    data_type, p.tags.data_tag));
                diff_d.reset(new memory::desc({ bs.mb, bs.c, bs.d, bs.h, bs.w },
                    data_type, p.tags.diff_tag));
            } else {
                data_d.reset(new memory::desc({ bs.mb, bs.c, bs.h, bs.w },
                    data_type, p.tags.data_tag));
                diff_d.reset(new memory::desc({ bs.mb, bs.c, bs.h, bs.w },
                    data_type, p.tags.diff_tag));
            }
        } else {
            data_d.reset(new memory::desc({ bs.mb, bs.c },
                data_type, p.tags.data_tag));
            diff_d.reset(new memory::desc({ bs.mb, bs.c },
                data_type, p.tags.diff_tag));
        }

        src.reset(new test_memory(*data_d, *eng));
        dst.reset(new test_memory(*data_d, *eng));
        diff_src.reset(new test_memory(*diff_d, *eng));
        diff_dst.reset(new test_memory(*diff_d, *eng));

        auto training = prop_kind::forward_training;
        auto inference = prop_kind::forward_inference;

        if (isF32(data_type)) {
            Forward(training);
            Forward(training, use_global_stats);
            Forward(training, use_scale_shift);
            Forward(training, use_scale_shift | use_global_stats);
            Forward(inference);
            Forward(inference, use_global_stats);
            Forward(inference, use_scale_shift);

            Backward(backward_data);
            Backward(backward_data, use_global_stats);
            Backward(backward_data, use_scale_shift);
            Backward(backward_data, use_scale_shift | use_global_stats);
            Backward(backward, use_scale_shift);
            Backward(backward, use_scale_shift | use_global_stats);
        } else if (isS8(data_type)) {
            Forward(inference, use_global_stats);
            Forward(inference, use_global_stats | use_scale_shift);
        }
    }

    void Forward(prop_kind pk, unsigned flags = 0u) {
        bool useScaleShift = flags & use_scale_shift;
        bool useGlobalStats = flags & use_global_stats;
        bool isTraining = pk == prop_kind::forward_training;

        auto bnorm_fwd_d = batch_normalization_forward::desc(pk,
                    *data_d, p.epsilon, flags);

        bnorm_fwd_pd.reset(new batch_normalization_forward::primitive_desc(
                    bnorm_fwd_d, *eng));

        weights.reset(new memory(bnorm_fwd_pd->weights_desc(), *eng));
        if (isTraining || useGlobalStats) {
            mean.reset(new memory(bnorm_fwd_pd->mean_desc(), *eng));
            variance.reset(new memory(bnorm_fwd_pd->variance_desc(), *eng));
        }

        fill<data_t>(src->get());
        fill<data_t>(dst->get());
        if (useScaleShift)
            fill<float>(*weights);
        if (useGlobalStats) {
            fill<float>(*mean);
            fill<float>(*variance);
        }
        check_zero_tail<data_t>(1, src->get());
        check_zero_tail<data_t>(1, dst->get());

        execBnormFwd(isTraining, useGlobalStats, useScaleShift);

        check_zero_tail<data_t>(0, dst->get());

        check_bnorm_fwd(p, src->get(), *mean, *variance, *weights, dst->get(),
                flags, pk);
    }

    void Backward(prop_kind pk, unsigned flags = 0u) {
        bool useScaleShift = flags & use_scale_shift;

        auto bnorm_fwd_d = batch_normalization_forward::desc(
                prop_kind::forward_training, *data_d, p.epsilon, flags);
        bnorm_fwd_pd.reset(new batch_normalization_forward::primitive_desc(
                    bnorm_fwd_d, *eng));

        auto bnorm_bwd_d = batch_normalization_backward::desc(
                pk, *diff_d, *data_d, p.epsilon, flags);
        bnorm_bwd_pd.reset(
                new batch_normalization_backward::primitive_desc(
                bnorm_bwd_d, *eng, *bnorm_fwd_pd));

        if (useScaleShift)
            weights.reset(new memory(bnorm_bwd_pd->weights_desc(), *eng));
        diff_weights.reset(new memory(bnorm_bwd_pd->diff_weights_desc(), *eng));
        mean.reset(new memory(bnorm_bwd_pd->mean_desc(), *eng));
        variance.reset(new memory(bnorm_bwd_pd->variance_desc(), *eng));

        if (useScaleShift)
            fill<float>(*weights);
        fill<float>(diff_src->get());
        fill<float>(diff_dst->get());
        fill<float>(*mean);
        fill<float>(*variance);
        check_zero_tail<data_t>(1, diff_src->get());
        check_zero_tail<data_t>(1, diff_dst->get());

        execBnormBwd(useScaleShift, pk);

        check_bnorm_bwd(p, src->get(), diff_dst->get(), *mean, *variance,
                *weights, diff_src->get(), *diff_weights, flags, pk);
        check_zero_tail<data_t>(0, diff_src->get());
    }

    inline bool isF32(memory::data_type data_type) {
        return data_type == mkldnn::memory::data_type::f32;
    }

    inline bool isS8(memory::data_type data_type) {
        return data_type == mkldnn::memory::data_type::s8;
    }

    void execBnormFwd(bool isTraining, bool useGlobalStats, bool useScaleShift) {
        std::unordered_map<int, memory> args = {
            {MKLDNN_ARG_SRC, src->get()},
            {MKLDNN_ARG_DST, dst->get()},
        };

        if (useScaleShift)
            args.insert({MKLDNN_ARG_SCALE_SHIFT, *weights});

        if (isTraining || useGlobalStats) {
            args.insert({MKLDNN_ARG_MEAN, *mean});
            args.insert({MKLDNN_ARG_VARIANCE, *variance});
        }

        batch_normalization_forward(*bnorm_fwd_pd).execute(*strm, args);
    }

    void execBnormBwd(bool useScaleShift, prop_kind pk) {
        std::unordered_map<int, memory> args = {
            {MKLDNN_ARG_SRC, src->get()},
            {MKLDNN_ARG_DIFF_DST, diff_dst->get()},
            {MKLDNN_ARG_MEAN, *mean},
            {MKLDNN_ARG_VARIANCE, *variance},
            {MKLDNN_ARG_DIFF_SRC, diff_src->get()},
        };

        if (useScaleShift) {
            args.insert({MKLDNN_ARG_SCALE_SHIFT, *weights});
            if (pk == prop_kind::backward)
                args.insert({MKLDNN_ARG_DIFF_SCALE_SHIFT, *diff_weights});
        }

        batch_normalization_backward(*bnorm_bwd_pd).execute(*strm, args);
    }

    void check_bnorm_fwd(const test_bnorm_params_t &p, const memory &src,
            const memory &mean, const memory &variance, const memory &weights,
            const memory &dst, unsigned flags, prop_kind pk) {
        memory::data_type data_type = data_traits<data_t>::data_type;
        const test_bnorm_sizes_t &bp = p.sizes;
        if (bp.mb * bp.c * bp.d * bp.h * bp.w == 0)
            return;

        const bool use_weights = flags & use_scale_shift;
        const bool calculate_stats = !(flags & use_global_stats);
        const bool is_training = pk == prop_kind::forward_training;

        const data_t *src_data = (const data_t *)src.get_data_handle();
        const data_t *dst_data = (const data_t *)dst.get_data_handle();
        const float *weights_data = use_weights ?
                (const float *)weights.get_data_handle() : nullptr;
        const float *mean_data = (!calculate_stats || is_training) ?
                (const float *)mean.get_data_handle() : nullptr;
        const float *variance_data = (!calculate_stats || is_training) ?
                (const float *)variance.get_data_handle() : nullptr;

        const memory::desc src_d = src.get_desc();
        const memory::desc dst_d = dst.get_desc();
        const memory::desc weights_d = use_weights ?
            weights.get_desc() : memory::desc();

        const mkldnn::impl::memory_desc_wrapper src_mdw(src_d.data);
        const mkldnn::impl::memory_desc_wrapper dst_mdw(dst_d.data);
        const mkldnn::impl::memory_desc_wrapper weights_mdw(weights_d.data);

        float eps = static_cast<float>(1.e-4 * bp.mb * bp.d * bp.h * bp.w);

        auto padded_c = src_d.data.padded_dims[1];

        mkldnn::impl::parallel_nd(bp.c, [&](memory::dim c) {
            float ref_mean = calculate_stats ? float(0) : mean_data[c];
            float ref_variance = calculate_stats ? float(0) :
                    variance_data[c];
            if (calculate_stats) {
                for (memory::dim n = 0; n < bp.mb; n++)
                for (memory::dim d = 0; d < bp.d; d++)
                for (memory::dim h = 0; h < bp.h; h++)
                for (memory::dim w = 0; w < bp.w; w++) {
                    size_t sidx = n * padded_c * bp.d * bp.h * bp.w +
                        c * bp.d * bp.h * bp.w + d * bp.h * bp.w + h * bp.w + w;
                    ref_mean += src_data[src_mdw.off_l(sidx, true)];
                }
                ref_mean /= bp.mb * bp.d * bp.h * bp.w;
                if (is_training) {
                    float mean_norm_max =
                        std::max(std::abs(mean_data[c]), std::abs(ref_mean));
                    if (mean_norm_max < eps)
                        mean_norm_max = float(1);
                    EXPECT_NEAR((mean_data[c] - ref_mean) / mean_norm_max,
                        0., eps);
                }

                for (memory::dim n = 0; n < bp.mb; n++)
                for (memory::dim d = 0; d < bp.d; d++)
                for (memory::dim h = 0; h < bp.h; h++)
                for (memory::dim w = 0; w < bp.w; w++) {
                    size_t sidx = n * padded_c * bp.d * bp.h * bp.w
                    + c * bp.d * bp.h * bp.w + d * bp.h * bp.w + h * bp.w + w;
                    float tmp = src_data[src_mdw.off_l(sidx, true)] - ref_mean;
                    ref_variance += tmp * tmp;
                }
                ref_variance /= bp.mb * bp.d * bp.h * bp.w;
                if (is_training) {
                    float variance_norm_max = std::max(
                            std::abs(variance_data[c]), std::abs(ref_variance));
                    if (variance_norm_max < eps)
                        variance_norm_max = float(1);
                    EXPECT_NEAR((variance_data[c] - ref_variance) /
                            variance_norm_max, 0., eps);
                }
            }
            float ref_sqrt_variance =
                static_cast<float>(sqrt(ref_variance + p.epsilon));
            float ref_rsqrt_variance = float(1) / (ref_sqrt_variance);

            for (memory::dim n = 0; n < bp.mb; n++)
            for (memory::dim d = 0; d < bp.d; d++)
            for (memory::dim h = 0; h < bp.h; h++)
            for (memory::dim w = 0; w < bp.w; w++) {
                size_t sdidx = n * padded_c * bp.d * bp.h * bp.w +
                    c * bp.d * bp.h * bp.w + d * bp.h * bp.w + h * bp.w + w;
                data_t ref_dst = data_t(0);
                float tmp_dst = float(0);
                if (use_weights) {
                    tmp_dst = weights_data[weights_mdw.off_l(c, true)] *
                        ((float)src_data[src_mdw.off_l(sdidx, true)] - ref_mean) *
                        ref_rsqrt_variance +
                        weights_data[weights_mdw.off_l(bp.c + c, true)];
                } else {
                    tmp_dst = ((float)src_data[src_mdw.off_l(sdidx, true)] -
                        ref_mean) * ref_rsqrt_variance;
                }

                if (isF32(data_type)) {
                    ref_dst = tmp_dst;
                } else if (isS8(data_type)) {
                    ref_dst = out_round<data_t>(
                        saturate<data_t, float>(tmp_dst));
                }

                data_t out = dst_data[dst_mdw.off_l(sdidx, true)];
                float norm_max = std::max(std::abs(out), std::abs(ref_dst));
                if (norm_max < 1e-2 || isS8(data_type))
                    norm_max = 1.;
                EXPECT_NEAR((out - ref_dst) / norm_max, 0., eps);
            }
        });
    }

    void check_bnorm_bwd(const test_bnorm_params_t &p, const memory &src,
            const memory &diff_dst, const memory &mean, const memory &variance,
            const memory &weights, const memory &diff_src,
            const memory &diff_weights, unsigned flags, prop_kind pk) {
        const test_bnorm_sizes_t &bp = p.sizes;
        const bool use_weights = flags & use_scale_shift;
        const bool calculate_diff_stats = !(flags & use_global_stats);

        const float *src_data = (const float *)src.get_data_handle();
        const float *weights_data = use_weights ?
            (const float *)weights.get_data_handle() : nullptr;
        const float *diff_dst_data =
            (const float *)diff_dst.get_data_handle();
        const float *mean_data = (const float *)mean.get_data_handle();
        const float *variance_data =
            (const float *)variance.get_data_handle();
        const float *diff_src_data = (float *)diff_src.get_data_handle();
        const float *diff_weights_data = (pk == prop_kind::backward) ?
            (float *)diff_weights.get_data_handle() : nullptr;

        const memory::desc src_d = src.get_desc();
        const memory::desc diff_dst_d = diff_dst.get_desc();
        const memory::desc weights_d = weights.get_desc();
        const memory::desc diff_src_d = diff_src.get_desc();
        const memory::desc diff_weights_d = diff_weights.get_desc();

        const mkldnn::impl::memory_desc_wrapper src_mdw(src_d.data);
        const mkldnn::impl::memory_desc_wrapper diff_dst_mdw(diff_dst_d.data);
        const mkldnn::impl::memory_desc_wrapper weights_mdw(weights_d.data);
        const mkldnn::impl::memory_desc_wrapper diff_src_mdw(diff_src_d.data);
        const mkldnn::impl::memory_desc_wrapper diff_weights_mdw(diff_weights_d.data);

        if (bp.mb * bp.c * bp.d * bp.h * bp.w == 0) {
            if (pk == backward) {
                for (memory::dim c = 0; c < bp.c; ++c) {
                    auto dg = diff_weights_data[diff_weights_mdw.off_l(c, true)];
                    auto db = diff_weights_data[diff_weights_mdw.off_l(bp.c + c, true)];
                    EXPECT_NEAR(dg, 0., 1e-7);
                    EXPECT_NEAR(db, 0., 1e-7);
                }
            }
            return;
        }

        const float eps =
            static_cast<float>(1.e-4 * bp.mb * bp.d * bp.h * bp.w);

        auto padded_c = src_d.data.padded_dims[1];

        mkldnn::impl::parallel_nd(bp.c, [&](memory::dim c) {
            float ref_diff_gamma = float(0);
            float ref_diff_beta = float(0);

            auto v_mean = mean_data[c];
            auto v_variance = variance_data[c];
            const float sqrt_variance = 1.0f / sqrt(v_variance + p.epsilon);

            auto gamma =
                use_weights ? weights_data[weights_mdw.off_l(c, true)] : 1;

            for (memory::dim n = 0; n < bp.mb; n++)
            for (memory::dim d = 0; d < bp.d; d++)
            for (memory::dim h = 0; h < bp.h; h++)
            for (memory::dim w = 0; w < bp.w; w++) {
                size_t sidx = n * padded_c * bp.d * bp.h * bp.w +
                    c * bp.d * bp.h * bp.w + d * bp.h * bp.w + h * bp.w + w;
                ref_diff_gamma += (src_data[src_mdw.off_l(sidx, true)] - v_mean)
                    * diff_dst_data[diff_dst_mdw.off_l(sidx, true)];
                ref_diff_beta += diff_dst_data[diff_dst_mdw.off_l(sidx, true)];
            }
            ref_diff_gamma *= sqrt_variance;

            if (pk == backward) {
                auto diff_gamma =
                    diff_weights_data[diff_weights_mdw.off_l(c, true)];
                float norm_max =
                    std::max(std::abs(diff_gamma), std::abs(ref_diff_gamma));
                if (norm_max < 1e-2)
                    norm_max = float(1);
                EXPECT_NEAR((diff_gamma - ref_diff_gamma) / norm_max, 0., eps);

                auto diff_beta =
                    diff_weights_data[diff_weights_mdw.off_l(bp.c + c, true)];
                norm_max =
                    std::max(std::abs(diff_beta), std::abs(ref_diff_beta));
                if (norm_max < 1e-2)
                    norm_max = float(1);
                EXPECT_NEAR((diff_beta - ref_diff_beta) / norm_max, 0., eps);
            }

            for (memory::dim n = 0; n < bp.mb; n++)
            for (memory::dim d = 0; d < bp.d; d++)
            for (memory::dim h = 0; h < bp.h; h++)
            for (memory::dim w = 0; w < bp.w; w++) {
                size_t sidx = n * padded_c * bp.d * bp.h * bp.w +
                    c * bp.d * bp.h * bp.w + d * bp.h * bp.w + h * bp.w + w;
                float ref_diff_src =
                    diff_dst_data[diff_dst_mdw.off_l(sidx, true)];
                if (calculate_diff_stats) {
                    ref_diff_src -= ref_diff_beta/(bp.mb*bp.d*bp.h*bp.w)
                    + (src_data[src_mdw.off_l(sidx, true)] - v_mean)
                    *ref_diff_gamma*sqrt_variance/(bp.mb*bp.d*bp.h*bp.w);
                }
                ref_diff_src *= gamma*sqrt_variance;
                float out_diff_src =
                    diff_src_data[diff_src_mdw.off_l(sidx, true)];
                float norm_max =
                    std::max(std::abs(out_diff_src), std::abs(ref_diff_src));
                if (norm_max < eps)
                    norm_max = float(1);
                EXPECT_NEAR((out_diff_src - ref_diff_src) / norm_max, 0., eps);
            }
        });
    }
};

}
