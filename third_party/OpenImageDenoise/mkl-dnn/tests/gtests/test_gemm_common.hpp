/*******************************************************************************
* Copyright 2018 Intel Corporation
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

#ifndef TEST_GEMM_COMMON_H
#define TEST_GEMM_COMMON_H

#include "mkldnn_test_common.hpp"
#include "gtest/gtest.h"

#include "mkldnn_types.h"
#include "mkldnn.h"

#include <type_traits>
#include <vector>

#define CONCAT_WITH_UNDERSCORE_(a,b) a ## _ ## b
#define CONCAT_WITH_UNDERSCORE(a,b) CONCAT_WITH_UNDERSCORE_(a,b)

#define INST_TEST_CASE_(str, ...) INSTANTIATE_TEST_SUITE_P( \
        str, gemm_test, ::testing::Values(__VA_ARGS__))
#define INST_TEST_CASE(str, ...) INST_TEST_CASE_( \
        CONCAT_WITH_UNDERSCORE(str,TEST_CASE_NAME_PREFIX), __VA_ARGS__)

namespace mkldnn {

struct test_igemm_params {
    char offsetc;
    bool zero_oa;
    bool zero_ob;
    bool zero_oc;
};

struct test_params {
    char transA;
    char transB;
    int64_t M;
    int64_t N;
    int64_t K;
    float alpha;
    float beta;
    int64_t lda;
    int64_t ldb;
    int64_t ldc;

    test_igemm_params igemm_params;
    bool expect_to_fail;
    mkldnn_status_t expected_status;

    bool tr_a() const { return transA == 'T' || transA == 't'; }
    bool tr_b() const { return transB == 'T' || transB == 't'; }
    int64_t sizeC() const { return N * ldc; }

    bool oc_is_R() const
    { auto c = igemm_params.offsetc; return c == 'R' || c == 'r'; }
    bool oc_is_C() const
    { auto c = igemm_params.offsetc; return c == 'C' || c == 'c'; }
    int64_t size_oc() const { return oc_is_R() ? N : oc_is_C() ? M : 1; }
};

/* Test implementation description.
 *
 * To reduce the time spent in GEMM validation the test matrices A, B, and C
 * are generated from sub-matrices (A', B', and C') of smaller size:
 * - A(M, K) <-> A'(M_test, K)
 * - B(K, N) <-> B'(K, N_test)
 * - C(M, N) <-> C'(M_test, N_test)
 *
 * The matrices A', B', and C' are generated randomly. Then:
 * - A(m, k) := A'(mapper_m[m], k),
 * - B(k, n) := B'(k, mapper_n[n]),
 * - C(m, n) := C'(mapper_m[m], mapper_n[n]);
 *
 * Here `mapper_x[]` is surjection of {0, ..., X-1} onto {0, ..., X_test-1}.
 * For simplicity mapper_x[x] = x, for x in {0, ..., X_test-1}.
 *
 * This technique allows reducing the complexity of the validation code from
 * O(M*N*K) to O(M_test * N_test * K).
 *
 * X_test := min(X, X_test_max), where X_test_max is prime number around 50.
 *
 * To make the test robust the surjective functions mapper_m and mapper_n
 * should randomly map the elements {X_test, ..., X-1} onto {0, ..., X_test-1}.
 *
 * The validation itself looks as follows:
 * 0.  Prepare mapper_m and mapper_n
 * 1.a Generate random matrices A', B', C'
 * 1.b Prepare matrices A, B, C based on A', B', and C' respectively
 * 2.  Compute C_calc := Op(M, N, K, A, B, C)
 * 3.  Compute C'_ref := Op_REF(M_test, N_test, K, A', B', C')
 * 4.  Expand C'_ref to C_ref, by applying mapper_m and mapper_n
 * 5.  Compare C_calc and C_ref
 */

const int M_test_max = 47;
const int N_test_max = 53;

/** Mapper:
 * a surjective function from {0, ..., dim-1} onto {0, ..., dim_test-1}.
 */
struct mapper_t {
    mapper_t(int64_t dim, int64_t dim_test_max,
            int64_t gen = 7, int64_t gen_start = 13)
        : dim_(dim), dim_test_(std::min(dim, dim_test_max))
        , gen_(gen), gen_start_(gen_start)
        , mapper_(dim)
    {
        for (int64_t d = 0; d < dim_test_; ++d) mapper_[d] = d;
        for (int64_t g = gen_start_ % dim_test_, d = dim_test_; d < dim_; ++d) {
            mapper_[d] = mapper_[g];
            g = g * gen_ % dim_test_;
        }
    }

    int64_t dim() const { return dim_; }
    int64_t dim_test() const { return dim_test_; }
    int64_t operator[](int64_t d) const { return mapper_[d]; }

  private:
    const int64_t dim_;
    const int64_t dim_test_;
    const int64_t gen_, gen_start_;
    std::vector<int64_t> mapper_;
};

enum class layout_t { ROW_MAJOR, COL_MAJOR };

/** Prepares matrix A or B according to the dimension mapper.
 * The K dimension is always assumed to be columns, hence:
 * - A layout = A_is_transposed ? ROW_MAJOR : COL_MAJOR
 * - B layout = B_is_transposed ? COL_MAJOR : ROW_MAJOR
 */
template <typename data_t>
void prepare_matrix(data_t *M, layout_t layout, int64_t R, int64_t C,
        int64_t LD, const mapper_t &mapper) {
    const data_t mean = (data_t)(std::is_same<data_t, float>::value ? 1.f : 4);
    const data_t var = (data_t)(std::is_same<data_t, float>::value ? 2e-1f : 3);

    ASSERT_EQ(R, mapper.dim());
    const int R_test = mapper.dim_test();

    if (layout == layout_t::COL_MAJOR) {
        mkldnn::impl::parallel_nd(C, R_test, [&](int64_t c, int64_t r) {
            const int64_t off = c * LD + r;
            M[off] = set_value<data_t>(off, mean, var, 1.);
        });
        if (R > R_test) {
            const int64_t R_rest = R - R_test;
            mkldnn::impl::parallel_nd(C, R_rest, [&](int64_t c, int64_t r_) {
                const int64_t r = R_test + r_;
                const int64_t off = c * LD + r;
                const int64_t off0 = c * LD + mapper[r];
                M[off] = M[off0];
            });
        }
    } else {
        mkldnn::impl::parallel_nd(R_test, C, [&](int64_t r, int64_t c) {
            const int64_t off = r * LD + c;
            M[off] = set_value<data_t>(off, mean, var, 1.);
        });
        if (R > R_test) {
            const int64_t R_rest = R - R_test;
            mkldnn::impl::parallel_nd(R_rest, C, [&](int64_t r_, int64_t c) {
                const int64_t r = R_test + r_;
                const int64_t off = r * LD + c;
                const int64_t off0 = mapper[r] * LD + c;
                M[off] = M[off0];
            });
        }
    }
}

/** Extends columns of the matrix M according to the mapper_c */
template <typename data_t>
void extend_matrix_cols(data_t *M, int64_t R, int64_t C, int64_t LD,
        const mapper_t &mapper_c) {
    ASSERT_EQ(C, mapper_c.dim());
    const int64_t C_test = mapper_c.dim_test();
    if (C_test == C) return;

    mkldnn::impl::parallel_nd(C - C_test, [&](int64_t c_) {
        const int64_t c = C_test + c_;
        const int64_t c0 = mapper_c[c];
        for (int64_t r = 0; r < R; ++r)
            M[c * LD + r] = M[c0 * LD + r];
    });
}

/** Extends rows of the matrix M according to the mapper_r */
template <typename data_t>
void extend_matrix_rows(data_t *M, int64_t R, int64_t C, int64_t LD,
        const mapper_t &mapper_r) {
    ASSERT_EQ(R, mapper_r.dim());
    const int64_t R_test = mapper_r.dim_test();
    if (R_test == R) return;

    mkldnn::impl::parallel_nd(C, R - R_test, [&](int64_t c, int64_t r_) {
        const int64_t r = R_test + r_;
        const int64_t r0 = mapper_r[r];
        M[c * LD + r] = M[c * LD + r0];
    });
}

/** Extends matrix M according to the mapper_r and mapper_c */
template <typename data_t>
void extend_matrix(data_t *M, int64_t R, int64_t C, int64_t LD,
        const mapper_t &mapper_r, const mapper_t &mapper_c) {
    ASSERT_EQ(R, mapper_r.dim());
    ASSERT_EQ(C, mapper_c.dim());
    extend_matrix_rows(M, R, C, LD, mapper_r);
    extend_matrix_cols(M, R, C, LD, mapper_c);
}

template <typename data_t>
void ref_gemm(const char *transa, const char *transb, int64_t m, int64_t n, int64_t k,
        const data_t alpha, const data_t *a, int64_t lda, const data_t *b,
        int64_t ldb, data_t beta, data_t *c, int64_t ldc) {

    const bool tr_a = transa && (*transa == 'T' || *transa == 't');
    const bool tr_b = transb && (*transb == 'T' || *transb == 't');

    auto pa = [=] (int64_t i, int64_t j) { return a[j*lda + i]; };
    auto pb = [=] (int64_t i, int64_t j) { return b[j*ldb + i]; };
    auto pc = [=] (int64_t i, int64_t j) { return c[j*ldc + i]; };

    mkldnn::impl::parallel_nd(m, n, [&](int64_t im, int64_t in) {
        data_t c_elem = (beta == 0.) ? 0. : pc(im, in) * beta;

        for (int64_t ik = 0; ik < k; ik++) {
            const data_t a_elem = tr_a ? pa(ik, im) : pa(im, ik);
            const data_t b_elem = tr_b ? pb(in, ik) : pb(ik, in);
            c_elem += alpha * a_elem * b_elem;
        }
        c[in*ldc + im] = c_elem;
    });
}

template <typename b_dt>
void ref_gemm_s8x8s32(const char *transa, const char *transb,
        const char *offsetc, int64_t M, int64_t N, int64_t K, const float alpha,
        int8_t *A, int64_t lda, const int8_t *oa, b_dt *B, int64_t ldb,
        const int8_t *ob, const float beta, int32_t *C, int64_t ldc,
        const int32_t *oc) {
    const bool tr_a = transa && (*transa == 'T' || *transa == 't');
    const bool tr_b = transb && (*transb == 'T' || *transb == 't');
    bool OCisR = (*offsetc == 'R' || *offsetc == 'r');
    bool OCisC = (*offsetc == 'C' || *offsetc == 'c');

    auto pa = [=] (int64_t i, int64_t j) { return (double)A[j*lda + i]; };
    auto pb = [=] (int64_t i, int64_t j) { return (double)B[j*ldb + i]; };
    auto pc = [=] (int64_t i, int64_t j) { return (double)C[j*ldc + i]; };

    mkldnn::impl::parallel_nd(M, N, [&](int64_t m, int64_t n) {
        double c_elem = 0;
        for (int64_t k = 0; k < K; k++) {
            const double a_elem = (tr_a ? pa(k, m) : pa(m, k)) + *oa;
            const double b_elem = (tr_b ? pb(n, k) : pb(k, n)) + *ob;
            c_elem += a_elem * b_elem;
        }

        double coffset = OCisR ? oc[n] : OCisC ? oc[m] : oc[0];
        double val
            = (beta == 0.f ? 0. : beta * pc(m, n)) + alpha * c_elem + coffset;
        C[n*ldc + m]
            = static_cast<int32_t>(nearbyint(saturate<int32_t, double>(val)));
    });
}

template <typename b_dt, typename c_dt>
void compare(int64_t m, int64_t n, const c_dt *c, const c_dt *c_ref, int64_t ldc,
        float alpha = 1.0f, float beta = 0.0f, int64_t k = 1) {
    using data_type = memory::data_type;
    mkldnn::impl::parallel_nd(n, ldc, [&](int64_t i, int64_t j) {
        c_dt ref = c_ref[i*ldc + j];
        c_dt got = c[i*ldc + j];
        c_dt diff = got - ref;

        if (data_traits<b_dt>::data_type == data_type::f32) {
            c_dt e = (std::abs(ref) > 1e-4) ? diff / ref : diff;
            EXPECT_NEAR(e, 0.0, 1e-4) << "Row: " << j << " Col: " << i;
        } else {
            // igemm
            if (alpha == 1.0f) {
                EXPECT_NEAR(diff, 0, 1) << "Row: " << j << " Col: " << i;
            } else {
                if (data_traits<b_dt>::data_type == data_type::u8) {
                    c_dt eps = k / 1000 + 1;
                    EXPECT_NEAR(diff, 0, eps) << "Row: " << j << " Col: " << i;
                } else if (data_traits<b_dt>::data_type == data_type::s8) {
                    c_dt eps = k / 500 + 1;
                    EXPECT_NEAR(diff, 0, eps) << "Row: " << j << " Col: " << i;
                }
            }
        }
    });
}

inline void get_matrix_size(const test_params &p, size_t &sizeA,
        size_t &sizeB, size_t &sizeC) {
    const bool tr_a = (p.transA == 'T' || p.transA == 't');
    const bool tr_b = (p.transB == 'T' || p.transB == 't');
    sizeA = !tr_a ? p.lda * p.K : p.lda * p.M,
    sizeB = !tr_b ? p.ldb * p.N : p.ldb * p.K,
    sizeC = p.ldc * p.N;
}

template <typename T>
inline T* get_matrix_buffer(size_t n) {
    return (T*)test_malloc(n * sizeof(T));
}

template <typename a_dt, typename b_dt, typename c_dt>
void fill_matrices(const test_params &p,
        const mapper_t &mapper_m, const mapper_t &mapper_n,
        a_dt *A, b_dt *B, c_dt *C, c_dt *C_ref,
        int8_t *oa = nullptr, int8_t *ob = nullptr, c_dt *oc = nullptr) {
    prepare_matrix(A, p.tr_a() ? layout_t::ROW_MAJOR : layout_t::COL_MAJOR,
            p.M, p.K, p.lda, mapper_m);
    prepare_matrix(B, p.tr_b() ? layout_t::COL_MAJOR : layout_t::ROW_MAJOR,
            p.N, p.K, p.ldb, mapper_n);

    fill_data(p.sizeC(), C);
    extend_matrix(C, p.M, p.N, p.ldc, mapper_m, mapper_n);
    mkldnn::impl::parallel_nd(p.sizeC(), [&](int64_t i) { C_ref[i] = C[i]; });

    if (oa == nullptr && ob == nullptr && oc == nullptr)
        return;

    *oa = (int8_t)(p.igemm_params.zero_oa ? 0 : 4);
    *ob = (int8_t)(p.igemm_params.zero_ob ? 0 : 3);

    if (p.igemm_params.zero_oc) {
        for (int64_t i = 0; i < p.size_oc(); i++) oc[i] = 0;
    } else {
        fill_data<c_dt>(p.size_oc(), oc, (c_dt)1, (c_dt)0);
        if (p.oc_is_R()) {
            extend_matrix_cols(oc, 1, p.N, 1, mapper_n);
        } else if (p.oc_is_C()) {
            extend_matrix_rows(oc, p.M, 1, p.M, mapper_m);
        }
    }
}

template <typename a_dt, typename b_dt, typename c_dt>
void run_test_gemm(const test_params &p) {}

template <>
void run_test_gemm<int8_t, uint8_t, int32_t>(const test_params &p) {
    if (p.expect_to_fail) {
        int8_t dummy_s8, *A = &dummy_s8, oa = 0, ob = 0;
        uint8_t dummy_u8, *B = &dummy_u8;
        int32_t dummy_s32, *C = &dummy_s32, *oc = &dummy_s32;
        auto status = mkldnn_gemm_s8u8s32(&p.transA, &p.transB,
                &p.igemm_params.offsetc, &p.M, &p.N, &p.K,
                &p.alpha, A, &p.lda, &oa, B, &p.ldb, &ob, &p.beta, C, &p.ldc, oc);
        if (status != mkldnn_success)
            throw error(status, "mkldnn_s8u8s32 returned error");
        return;
    }

    size_t sizeA, sizeB, sizeC;
    get_matrix_size(p, sizeA, sizeB, sizeC);

    int8_t  *A = get_matrix_buffer<int8_t>(sizeA);
    uint8_t *B = get_matrix_buffer<uint8_t>(sizeB);
    int32_t *C = get_matrix_buffer<int32_t>(sizeC);
    int32_t *C_ref = get_matrix_buffer<int32_t>(sizeC);
    int8_t oa, ob;
    int32_t *oc = get_matrix_buffer<int32_t>(p.size_oc());

    mapper_t mapper_m(p.M, M_test_max), mapper_n(p.N, N_test_max);
    const int64_t M_test = mapper_m.dim_test();
    const int64_t N_test = mapper_n.dim_test();

    fill_matrices(p, mapper_m, mapper_n, A, B, C, C_ref, &oa, &ob, oc);

    auto status = mkldnn_gemm_s8u8s32(&p.transA, &p.transB,
            &p.igemm_params.offsetc, &p.M, &p.N, &p.K,
            &p.alpha, A, &p.lda, &oa, B, &p.ldb, &ob, &p.beta, C, &p.ldc, oc);

    if (status == mkldnn_success) {
        ref_gemm_s8x8s32<uint8_t>(&p.transA, &p.transB, &p.igemm_params.offsetc,
                M_test, N_test, p.K, p.alpha, A, p.lda, &oa, B, p.ldb, &ob,
                p.beta, C_ref, p.ldc, oc);
        extend_matrix(C_ref, p.M, p.N, p.ldc, mapper_m, mapper_n);
        compare<uint8_t, int32_t>(p.M, p.N, C, C_ref, p.ldc, p.alpha, p.beta, p.K);
    }

    test_free((char *)A);
    test_free((char *)B);
    test_free((char *)C);
    test_free((char *)C_ref);
    test_free((char *)oc);

    if (status != mkldnn_success)
        throw error(status, "mkldnn_gemm_s8u8s32 returned error");
}

template <>
void run_test_gemm<int8_t, int8_t, int32_t>(const test_params &p) {
    if (p.expect_to_fail) {
        int8_t dummy_s8, *A = &dummy_s8, *B = &dummy_s8, oa = 0, ob = 0;
        int32_t dummy_s32, *C = &dummy_s32, *oc = &dummy_s32;
        auto status = mkldnn_gemm_s8s8s32(&p.transA, &p.transB,
                &p.igemm_params.offsetc, &p.M, &p.N, &p.K,
                &p.alpha, A, &p.lda, &oa, B, &p.ldb, &ob, &p.beta, C, &p.ldc, oc);
        if (status != mkldnn_success)
            throw error(status, "mkldnn_s8s8s32 returned error");
        return;
    }

    size_t sizeA, sizeB, sizeC;
    get_matrix_size(p, sizeA, sizeB, sizeC);

    int8_t  *A = get_matrix_buffer<int8_t>(sizeA);
    int8_t  *B = get_matrix_buffer<int8_t>(sizeB);
    int32_t *C = get_matrix_buffer<int32_t>(sizeC);
    int32_t *C_ref = get_matrix_buffer<int32_t>(sizeC);
    int8_t oa, ob;
    int32_t* oc = get_matrix_buffer<int32_t>(p.size_oc());

    mapper_t mapper_m(p.M, M_test_max), mapper_n(p.N, N_test_max);
    const int64_t M_test = mapper_m.dim_test();
    const int64_t N_test = mapper_n.dim_test();

    fill_matrices(p, mapper_m, mapper_n, A, B, C, C_ref, &oa, &ob, oc);

    auto status = mkldnn_gemm_s8s8s32(&p.transA, &p.transB,
            &p.igemm_params.offsetc, &p.M, &p.N, &p.K,
            &p.alpha, A, &p.lda, &oa, B, &p.ldb, &ob, &p.beta, C, &p.ldc, oc);

    if (status == mkldnn_success) {
        ref_gemm_s8x8s32<int8_t>(&p.transA, &p.transB, &p.igemm_params.offsetc,
                M_test, N_test, p.K, p.alpha, A, p.lda, &oa, B, p.ldb, &ob,
                p.beta, C_ref, p.ldc, oc);
        extend_matrix(C_ref, p.M, p.N, p.ldc, mapper_m, mapper_n);
        compare<int8_t, int32_t>(p.M, p.N, C, C_ref, p.ldc, p.alpha, p.beta, p.K);
    }

    test_free((char *)A);
    test_free((char *)B);
    test_free((char *)C);
    test_free((char *)C_ref);
    test_free((char *)oc);

    if (status != mkldnn_success)
        throw error(status, "mkldnn_gemm_s8s8s32 returned error");
}

template <>
void run_test_gemm<float, float, float>(const test_params &p) {
    if (p.expect_to_fail) {
        float dummy_f32, *A = &dummy_f32, *B = &dummy_f32, *C = &dummy_f32;
        auto status = mkldnn_sgemm(&p.transA, &p.transB, &p.M, &p.N, &p.K,
                &p.alpha, A, &p.lda, B, &p.ldb, &p.beta, C, &p.ldc);
        if (status != mkldnn_success)
            throw error(status, "mkldnn_sgemm returned error");
        return;
    }

    size_t sizeA, sizeB, sizeC;
    get_matrix_size(p, sizeA, sizeB, sizeC);

    float *A = get_matrix_buffer<float>(sizeA);
    float *B = get_matrix_buffer<float>(sizeB);
    float *C = get_matrix_buffer<float>(sizeC);
    float *C_ref = get_matrix_buffer<float>(sizeC);

    mapper_t mapper_m(p.M, M_test_max), mapper_n(p.N, N_test_max);
    const int64_t M_test = mapper_m.dim_test();
    const int64_t N_test = mapper_n.dim_test();

    fill_matrices(p, mapper_m, mapper_n, A, B, C, C_ref);

    auto status = mkldnn_sgemm(&p.transA, &p.transB, &p.M, &p.N, &p.K, &p.alpha,
        A, &p.lda, B, &p.ldb, &p.beta, C, &p.ldc);

    if (status == mkldnn_success) {
        ref_gemm(&p.transA, &p.transB, M_test, N_test, p.K,
                p.alpha, A, p.lda, B, p.ldb, p.beta, C_ref, p.ldc);
        extend_matrix(C_ref, p.M, p.N, p.ldc, mapper_m, mapper_n);
        compare<float, float>(p.M, p.N, C, C_ref, p.ldc);
    }

    test_free((char *)A);
    test_free((char *)B);
    test_free((char *)C);
    test_free((char *)C_ref);

    if (status != mkldnn_success)
        throw error(status, "mkldnn_sgemm returned error");
}

template <typename a_dt, typename b_dt, typename c_dt>
class gemm_test_common: public ::testing::TestWithParam<test_params> {
protected:
    virtual void SetUp() {
        const auto &p = ::testing::TestWithParam<test_params>::GetParam();
        catch_expected_failures([=](){Test();}, p.expect_to_fail,
                    p.expected_status);
    }
    void Test() {
        const auto &p = ::testing::TestWithParam<test_params>::GetParam();
        run_test_gemm<a_dt, b_dt, c_dt>(p);
    }
};
}
#endif
