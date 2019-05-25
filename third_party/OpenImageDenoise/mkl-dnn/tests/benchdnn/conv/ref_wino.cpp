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

#include "common.hpp"
#include "conv/conv_common.hpp"

namespace conv {

template <typename Telem, size_t Tdims>
struct array_offset_calculator {
    template <typename... Targs>
    array_offset_calculator(Telem *base, Targs... Fargs) : _dims{ Fargs... }
    {
        _base_ptr = base;
    }
    template <typename... Targs>
    inline Telem &operator()(Targs... Fargs)
    {
        return *(_base_ptr + _offset(1, Fargs...));
    }

private:
    template <typename... Targs>
    inline size_t _offset(size_t const dimension, size_t element)
    {
        return element;
    }

    template <typename... Targs>
    inline size_t _offset(size_t const dimension, size_t theta, size_t element)
    {
        return element + (_dims[dimension] * theta);
    }

    template <typename... Targs>
    inline size_t _offset(size_t const dimension, size_t theta, size_t element,
            Targs... Fargs)
    {
        size_t t_prime = element + (_dims[dimension] * theta);
        return _offset(dimension + 1, t_prime, Fargs...);
    }

    Telem *_base_ptr;
    const int64_t _dims[Tdims];
};

void trans_I_4x4_3x3(float Iw[6][6], float I[6][6]) {
    float T[6][6];
    float t0;
    float t1;
    float t2;
    float t3;
    float t4;
    float t5;

    for (int i = 0; i < 6; i++) {
        t0 = I[2][i] * -2.25f + I[4][i];
        t1 = I[1][i] * -2.25f + I[3][i];
        t2 = I[2][i] * -0.390625f + I[4][i];
        t3 = I[1][i] * -0.390625f + I[3][i];
        t4 = I[0][i] * 0.87890625f + I[4][i];
        t5 = I[1][i] * 0.87890625f + I[5][i];

        T[0][i] = I[2][i] * -2.640625f + t4;
        T[1][i] = t1 * 0.625f + t0;
        T[2][i] = t1 * -0.625f + t0;
        T[3][i] = t3 * 1.5f + t2;
        T[4][i] = t3 * -1.5f + t2;
        T[5][i] = I[3][i] * -2.640625f + t5;
    }

    for (int i = 0; i < 6; i++) {
        t0 = T[i][2] * -2.25f + T[i][4];
        t1 = T[i][1] * -2.25f + T[i][3];
        t2 = T[i][2] * -0.390625f + T[i][4];
        t3 = T[i][1] * -0.390625f + T[i][3];
        t4 = T[i][0] * 0.87890625f + T[i][4];
        t5 = T[i][1] * 0.87890625f + T[i][5];

        Iw[i][0] = T[i][2] * -2.640625f + t4;
        Iw[i][1] = t1 * 0.625f + t0;
        Iw[i][2] = t1 * -0.625f + t0;
        Iw[i][3] = t3 * 1.5f + t2;
        Iw[i][4] = t3 * -1.5f + t2;
        Iw[i][5] = T[i][3] * -2.640625f + t5;
    }
}

void trans_W_4x4_3x3(float Fw_[6][6], float F[3][3]) {
    float Fw[6];
    float T[6][3];
    float t0;
    float t1;
    float t2;

    for (int i = 0; i < 3; i++) {
        t0 = 0.26890756302521f * F[2][i];
        t1 = -t0 - 0.688403361344538f * F[0][i];
        t2 = t0 + 0.119514472455649f * F[0][i];

        T[0][i] = 1.13777777777778f * F[0][i];
        T[1][i] = t1 - 0.430252100840336f * F[1][i];
        T[2][i] = t1 + 0.430252100840336f * F[1][i];
        T[3][i] = t2 + 0.179271708683473f * F[1][i];
        T[4][i] = t2 - 0.179271708683473f * F[1][i];
        T[5][i] = F[2][i];
    }

    for (int i = 0; i < 6; i++) {
        t0 = 0.26890756302521f * T[i][2];
        t1 = -t0 - 0.688403361344538f * T[i][0];
        t2 = t0 + 0.119514472455649f * T[i][0];

        Fw[0] = 1.13777777777778f * T[i][0];
        Fw[1] = t1 - 0.430252100840336f * T[i][1];
        Fw[2] = t1 + 0.430252100840336f * T[i][1];
        Fw[3] = t2 + 0.179271708683473f * T[i][1];
        Fw[4] = t2 - 0.179271708683473f * T[i][1];
        Fw[5] = T[i][2];
        for (int l = 0; l < 6; l++) {
            Fw_[i][l] = Fw[l];
        }
    }
}

void trans_O_4x4_3x3(float Mw[6][6], float O[4][4]) {
    float T[4][6];
    float t0;
    float t1;
    float t2;
    float t3;

    for (int i = 0; i < 6; i++) {
        t0 = Mw[1][i] + Mw[2][i];
        t1 = Mw[3][i] + Mw[4][i];
        t2 = Mw[1][i] - Mw[2][i];
        t3 = Mw[3][i] - Mw[4][i];

        T[0][i] = t0 + t1 + Mw[0][i];
        T[1][i] = t2 * 0.625f + t3 * 1.5f;
        T[2][i] = t0 * 0.390625f + t1 * 2.25f;
        T[3][i] = t2 * 0.244140625f + t3 * 3.375f + Mw[5][i];
    }

    for (int i = 0; i < 4; i++) {
        t0 = T[i][1] + T[i][2];
        t1 = T[i][3] + T[i][4];
        t2 = T[i][1] - T[i][2];
        t3 = T[i][3] - T[i][4];

        O[i][0] = t0 + t1 + T[i][0];
        O[i][1] = t2 * 0.625f + t3 * 1.5f;
        O[i][2] = t0 * 0.390625f + t1 * 2.25f;
        O[i][3] = t2 * 0.244140625f + t3 * 3.375f + T[i][5];
    }
}

void trans_W_3x3_4x4_wu(float Fw[6][6], float F[4][6]) {
    float T[6][4];
    float t0;
    float t1;
    float t2;
    float t3;
    float t4;

    for (int i = 0; i < 4; i++) {
        t0 = F[2][i] * 0.26890756302521f;
        t1 = F[0][i] * -0.688403361344538f - t0;
        t2 = F[0][i] * 0.119514472455649f + t0;
        t3 = F[1][i] * 0.430252100840336f + F[3][i] * 0.168067226890756f;
        t4 = F[1][i] * 0.179271708683473f + F[3][i] * 0.403361344537815f;

        T[0][i] = F[0][i] * 1.13777777777778f;
        T[1][i] = t1 - t3;
        T[2][i] = t1 + t3;
        T[3][i] = t2 + t4;
        T[4][i] = t2 - t4;
        T[5][i] = F[3][i];
    }

    for (int i = 0; i < 6; i++) {
        t0 = T[i][2] * 0.26890756302521f;
        t1 = T[i][0] * -0.688403361344538f - t0;
        t2 = T[i][0] * 0.119514472455649f + t0;
        t3 = T[i][1] * 0.430252100840336f + T[i][3] * 0.168067226890756f;
        t4 = T[i][1] * 0.179271708683473f + T[i][3] * 0.403361344537815f;

        Fw[i][0] = T[i][0] * 1.13777777777778f;
        Fw[i][1] = t1 - t3;
        Fw[i][2] = t1 + t3;
        Fw[i][3] = t2 + t4;
        Fw[i][4] = t2 - t4;
        Fw[i][5] = T[i][3];
    }
}

void trans_O_3x3_4x4_wu(float Mw[6][6], float M[3][3]) {
    float T[3][6];
    float t0;
    float t1;
    float t2;
    float M_[3];

    for (int i = 0; i < 6; i++) {
        t0 = Mw[1][i] + Mw[2][i];
        t1 = Mw[3][i] + Mw[4][i];
        t2 = t1 * 2.25f + Mw[5][i];

        T[0][i] = Mw[0][i] + t0 + t1;
        T[1][i] = 0.625f * (Mw[1][i] - Mw[2][i]) +
            1.5f * (Mw[3][i] - Mw[4][i]);
        T[2][i] = t0 * 0.390625f + t2;
    }
    for (int i = 0; i < 3; i++) {
        t0 = T[i][1] + T[i][2];
        t1 = T[i][3] + T[i][4];
        t2 = t1 * 2.25f + T[i][5];

        M_[0] = T[i][0] + t0 + t1;
        M_[1] = 0.625f * (T[i][1] - T[i][2]) +
            1.5f * (T[i][3] - T[i][4]);
        M_[2] = t0 * 0.390625f + t2;

        for (int k = 0; k < 3; k++) {
            M[i][k] = M_[k];
        }
    }
}

struct scratchpad_t {
    float *_u_ptr;
    float *_m_ptr;
    float *_v_ptr;

    int64_t h_tiles;
    int64_t w_tiles;

    const int64_t alpha = 6;
    const int64_t out_dim = 4;
};

int init_scratchpad(const  prb_t *p, scratchpad_t &sp) {
    if (sp.out_dim != 4 || sp.alpha != 6)
        return FAIL;

    sp.h_tiles = p->dir == FLAG_FWD ? div_up(p->oh, sp.out_dim) :
                                             div_up(p->ih, sp.out_dim);
    sp.w_tiles = p->dir == FLAG_FWD ? div_up(p->ow, sp.out_dim) :
                                             div_up(p->iw, sp.out_dim);

    sp._u_ptr = (float *)zmalloc(
            sizeof(float) * sp.alpha * sp.alpha * p->oc * p->ic, 64);
    sp._v_ptr = (float *)zmalloc(sizeof(float) * sp.alpha * sp.alpha * p->ic
                    * p->mb * sp.h_tiles * sp.w_tiles, 64);
    sp._m_ptr = (float *)zmalloc(sizeof(float) * sp.alpha * sp.alpha * p->oc
                    * p->mb * sp.h_tiles * sp.w_tiles, 64);

    if (sp._u_ptr == NULL || sp._v_ptr == NULL || sp._m_ptr == NULL)
        return mkldnn_out_of_memory;

    array_set((char *)sp._u_ptr,
            sizeof(float) * sp.alpha * sp.alpha * p->oc * p->ic);
    array_set((char *)sp._v_ptr, sizeof(float) * sp.alpha * sp.alpha * p->ic
                    * p->mb * sp.h_tiles * sp.w_tiles);
    array_set((char *)sp._m_ptr, sizeof(float) * sp.alpha * sp.alpha * p->oc
                    * p->mb * sp.h_tiles * sp.w_tiles);

    return OK;
}

void free_scratchpad(scratchpad_t *sp) {
    if(sp->_u_ptr != NULL) zfree(sp->_u_ptr);
    if(sp->_v_ptr != NULL) zfree(sp->_v_ptr);
    if(sp->_m_ptr != NULL) zfree(sp->_m_ptr);
}

void compute_wino_ref_fwd(const prb_t *p, dnn_mem_t &src_m, dnn_mem_t &wei_m,
        dnn_mem_t &bia_m, dnn_mem_t &dst_m) {
    scratchpad_t sp{};
    SAFE_V(init_scratchpad(p, sp));

    array_offset_calculator<float, 4> U(
            sp._u_ptr, sp.alpha, sp.alpha, p->oc, p->ic);
    array_offset_calculator<float, 6> V(sp._v_ptr, sp.alpha, sp.alpha, p->ic,
            p->mb, sp.h_tiles, sp.w_tiles);
    array_offset_calculator<float, 6> M(sp._m_ptr, sp.alpha, sp.alpha, p->oc,
            p->mb, sp.h_tiles, sp.w_tiles);

    SAFE_V(p->kh == 3 ? OK : FAIL);
    SAFE_V(p->kw == 3 ? OK : FAIL);

    bool with_bias = p->dir & FLAG_BIA;
    const int64_t t_pad = p->ph;
    const int64_t l_pad = p->pw;
    const int64_t wp_max = p->iw + l_pad;
    const int64_t hp_max = p->ih + t_pad;
    const int64_t p_dim = p->mb * sp.h_tiles * sp.w_tiles;

#pragma omp parallel
    {
    float I[6][6];
    float F[3][3];
    float O[4][4];

    float _v[6][6];
    float _u[6][6];
    float _m[6][6];

#pragma omp for collapse(4)
    /* src_transform v <- B_t * d * B */
    for (int64_t img = 0; img < p->mb; img++) {
        for (int64_t c = 0; c < p->ic; c++) {
            for (int64_t hfm = 0; hfm < sp.h_tiles; hfm++) {
                for (int64_t wfm = 0; wfm < sp.w_tiles; wfm++) {
                    for (int64_t j = 0; j < sp.alpha; j++) {
                        int64_t ydim = hfm * sp.out_dim + j;
                        if ((t_pad <= ydim) && (ydim < hp_max)) {
                            for (int64_t k = 0; k < sp.alpha; k++) {
                                int64_t xdim = wfm * sp.out_dim + k;
                                if ((l_pad <= xdim) && (xdim < wp_max)) {
                                    size_t src_off = src_off_f(p, img, 0, c, 0,
                                            ydim - t_pad, xdim - l_pad);
                                    I[j][k] = ((float *)src_m)[src_off];
                                } else {
                                    I[j][k] = 0.f;
                                }
                            }
                        } else {
                            for (int64_t k = 0; k < sp.alpha; k++) {
                                I[j][k] = 0.f;
                            }
                        }
                    }

                    trans_I_4x4_3x3(_v, I);

                    /* scatter v:V */
                    for (int64_t j = 0; j < sp.alpha; j++) {
                        for (int64_t k = 0; k < sp.alpha; k++) {
                            V(j, k, c, img, hfm, wfm) = _v[j][k];
                        }
                    }
                }
            }
        }
    }

#pragma omp for collapse(2)
    /* wei_transform u <- G * g * G_t */
    for (int64_t oc = 0; oc < p->oc; ++oc) {
        for (int64_t ic = 0; ic < p->ic; ++ic) {
            for (int64_t j = 0; j < p->kh; j++) {
                for (int64_t i = 0; i < p->kw; i++) {
                    size_t wei_off = wei_off_f(p, 0, oc, ic, 0, j, i);
                    F[j][i] = ((float *)wei_m)[wei_off];
                }
            }

            trans_W_4x4_3x3(_u, F);

            /* scatter u:U */
            for (int64_t j = 0; j < sp.alpha; j++) {
                for (int64_t k = 0; k < sp.alpha; k++) {
                    U(j, k, oc, ic) = _u[j][k];
                }
            }
        }
    }

#pragma omp for collapse(2)
    /* M = U * V */
    for (int64_t j = 0; j < sp.alpha; ++j) {
        for (int64_t k = 0; k < sp.alpha; ++k) {
            gemm("C", "N", "N", p->oc, p_dim, p->ic, 1.0,
                    (float *)&(U(j, k, 0, 0)), p->ic,
                    (float *)&(V(j, k, 0, 0, 0, 0)), p_dim, 1.0,
                    (float *)&(M(j, k, 0, 0, 0, 0)), p_dim);
        }
    }

#pragma omp for collapse(4)
    /* Y = A_t *m * A */
    for (int64_t oc = 0; oc < p->oc; ++oc) {
        for (int64_t img = 0; img < p->mb; ++img) {
            for (int64_t hfm = 0; hfm < sp.h_tiles; ++hfm) {
                for (int64_t wfm = 0; wfm < sp.w_tiles; ++wfm) {
                    for (int64_t j = 0; j < sp.alpha; j++) {
                        for (int64_t k = 0; k < sp.alpha; k++) {
                            _m[j][k] = M(j, k, oc, img, hfm, wfm);
                        }
                    }
                    trans_O_4x4_3x3(_m, O);

                    for (int64_t j = 0; j < sp.out_dim; j++) {
                        int64_t ydim = hfm * sp.out_dim + j;
                        if (ydim < p->oh) {
                            for (int64_t k = 0; k < sp.out_dim; k++) {

                                float conv_res = O[j][k];

                                int64_t xdim = wfm * sp.out_dim + k;
                                if (xdim < p->ow) {
                                    const size_t dst_off = dst_off_f(
                                            p, img, 0, oc, 0, ydim, xdim);
                                    float &dst = ((float *)dst_m)[dst_off];

                                    const size_t bia_off = bia_off_f(p, 0, oc);
                                    conv_res += with_bias ?
                                            ((float *)bia_m)[bia_off] :
                                            0.f;

                                    const auto &ops = p->attr.post_ops;
                                    for (int idx = 0; idx < ops.len; ++idx) {
                                        using pk = attr_t::post_ops_t::kind_t;
                                        const auto &e = ops.entry[idx];
                                        switch (e.kind) {
                                        case pk::SUM:
                                            conv_res += e.sum.scale * dst;
                                            break;
                                        case pk::RELU:
                                            conv_res = e.eltwise.scale
                                                    * (conv_res < 0 ? 0 :
                                                                      conv_res);
                                            break;
                                        default:
                                            assert(!"unknown "
                                                    "attr::post_ops::kind");
                                        }
                                    }

                                    dst = conv_res;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    }

    free_scratchpad(&sp);
}

void compute_wino_ref_bwd_d(const prb_t *p, dnn_mem_t &diff_src_m,
        dnn_mem_t &wei_m, dnn_mem_t &bia_m, dnn_mem_t &diff_dst_m) {
    scratchpad_t sp{};
    SAFE_V(init_scratchpad(p, sp));

    array_offset_calculator<float, 4> U(
            sp._u_ptr, sp.alpha, sp.alpha, p->ic, p->oc);
    array_offset_calculator<float, 6> V(sp._m_ptr, sp.alpha, sp.alpha, p->oc,
            p->mb, sp.h_tiles, sp.w_tiles);
    array_offset_calculator<float, 6> M(sp._v_ptr, sp.alpha, sp.alpha, p->ic,
            p->mb, sp.h_tiles, sp.w_tiles);

    SAFE_V(p->kh == 3 ? OK : FAIL);
    SAFE_V(p->kw == 3 ? OK : FAIL);

    const int64_t r_pad = MAX2(0, p->ow - 1 + p->kw - p->iw - p->pw);
    const int64_t l_pad = p->iw + r_pad - p->ow;
    const int64_t t_pad = p->ih + p->ph - p->oh;
    const int64_t wp_max = p->ow + l_pad;
    const int64_t hp_max = p->oh + t_pad;
    const int64_t p_dim = p->mb * sp.h_tiles * sp.w_tiles;

    bool with_bias = p->dir & FLAG_BIA;

#pragma omp parallel
    {
    float I[6][6];
    float F[3][3];
    float O[4][4];

    float _v[6][6];
    float _u[6][6];
    float _m[6][6];

#pragma omp for collapse(4)
    /* diff_src transform v <- B_t * d * B */
    for (int64_t img = 0; img < p->mb; img++) {
        for (int64_t c = 0; c < p->oc; c++) {
            for (int64_t hfm = 0; hfm < sp.h_tiles; hfm++) {
                for (int64_t wfm = 0; wfm < sp.w_tiles; wfm++) {

                    for (int64_t j = 0; j < sp.alpha; j++) {
                        int64_t ydim = hfm * sp.out_dim + j;
                        if ((t_pad <= ydim) && (ydim < hp_max)) {
                            for (int64_t k = 0; k < sp.alpha; k++) {
                                int64_t xdim = wfm * sp.out_dim + k;
                                if ((l_pad <= xdim) && (xdim < wp_max)) {
                                    size_t dst_off = dst_off_f(p, img, 0, c, 0,
                                            ydim - t_pad, xdim - l_pad);
                                    I[j][k] = ((float *)diff_dst_m)[dst_off];
                                } else {
                                    I[j][k] = 0.f;
                                }
                            }
                        } else {
                            for (int64_t k = 0; k < sp.alpha; k++) {
                                I[j][k] = 0.f;
                            }
                        }
                    }

                    trans_I_4x4_3x3(_v, I);

                    /* scatter v:V */
                    for (int64_t j = 0; j < sp.alpha; j++) {
                        for (int64_t k = 0; k < sp.alpha; k++) {
                            V(j, k, c, img, hfm, wfm) = _v[j][k];
                        }
                    }
                }
            }
        }
    }

#pragma omp for collapse(2)
    /* wei_transform u <- G * g * G_t */
    for (int64_t ic = 0; ic < p->ic; ++ic) {
        for (int64_t oc = 0; oc < p->oc; ++oc) {
            for (int64_t j = 0; j < p->kh; j++) {
                for (int64_t i = 0; i < p->kw; i++) {
                    size_t wei_off = wei_off_f(
                            p, 0, oc, ic, 0, p->kh - j - 1, p->kw - i - 1);
                    F[j][i] = ((float *)wei_m)[wei_off];
                }
            }
            trans_W_4x4_3x3(_u, F);

            /* scatter u:U */
            for (int64_t j = 0; j < sp.alpha; j++) {
                for (int64_t k = 0; k < sp.alpha; k++) {
                    U(j, k, ic, oc) = _u[j][k];
                }
            }
        }
    }

#pragma omp for collapse(2)
    /* M = U * V */
    for (int64_t j = 0; j < sp.alpha; ++j) {
        for (int64_t k = 0; k < sp.alpha; ++k) {
            gemm("C", "N", "N", p->ic, p_dim, p->oc, 1.0,
                    (float *)&(U(j, k, 0, 0)), p->oc,
                    (float *)&(V(j, k, 0, 0, 0, 0)), p_dim, 1.0,
                    (float *)&(M(j, k, 0, 0, 0, 0)), p_dim);
        }
    }

#pragma omp for collapse(4)
    /* diff_dst: Y = A_t *m * A */
    for (int64_t c = 0; c < p->ic; ++c) {
        for (int64_t img = 0; img < p->mb; ++img) {
            for (int64_t hfm = 0; hfm < sp.h_tiles; ++hfm) {
                for (int64_t wfm = 0; wfm < sp.w_tiles; ++wfm) {
                    for (int64_t j = 0; j < sp.alpha; j++) {
                        for (int64_t k = 0; k < sp.alpha; k++) {
                            _m[j][k] = M(j, k, c, img, hfm, wfm);
                        }
                    }
                    trans_O_4x4_3x3(_m, O);

                    float bia = with_bias ? ((float *)bia_m)[c] : 0.f;

                    for (int64_t j = 0; j < sp.out_dim; j++) {
                        int64_t ydim = hfm * sp.out_dim + j;
                        if (ydim < p->ih) {
                            for (int64_t k = 0; k < sp.out_dim; k++) {
                                int64_t xdim = wfm * sp.out_dim + k;
                                if (xdim < p->iw) {
                                    size_t src_off = src_off_f(
                                            p, img, 0, c, 0, ydim, xdim);
                                    ((float *)diff_src_m)[src_off] = O[j][k]
                                            + bia;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    }

    free_scratchpad(&sp);
}

void compute_wino_ref_bwd_w(const prb_t *p, dnn_mem_t &src_m,
        dnn_mem_t &diff_wei_m, dnn_mem_t &diff_bia_m, dnn_mem_t &diff_dst_m) {
    scratchpad_t sp{};
    SAFE_V(init_scratchpad(p, sp));

    array_offset_calculator<float, 4> U(
            sp._u_ptr, sp.alpha, sp.alpha, p->oc, p->ic);
    array_offset_calculator<float, 6> V(sp._v_ptr, sp.alpha, sp.alpha, p->mb,
            sp.h_tiles, sp.w_tiles, p->ic);
    array_offset_calculator<float, 6> M(sp._m_ptr, sp.alpha, sp.alpha, p->oc,
            p->mb, sp.h_tiles, sp.w_tiles);

    SAFE_V(p->kh == 3 ? OK : FAIL);
    SAFE_V(p->kw == 3 ? OK : FAIL);

    const int64_t t_pad = p->ph;
    const int64_t l_pad = p->pw;
    const int64_t wp_max = p->iw + l_pad;
    const int64_t hp_max = p->ih + t_pad;
    const int64_t p_dim = p->mb * sp.h_tiles * sp.w_tiles;

#pragma omp parallel
    {
    float I[6][6];
    float F[6][6];
    float O[6][6];

    float _v[6][6];
    float _u[3][3];
    float _m[6][6];

#pragma omp for collapse(4)
    /* src transform v <- B_t * d * B */
    for (int64_t img = 0; img < p->mb; img++) {
        for (int64_t hfm = 0; hfm < sp.h_tiles; hfm++) {
            for (int64_t wfm = 0; wfm < sp.w_tiles; wfm++) {
                for (int64_t ic = 0; ic < p->ic; ic++) {
                    for (int64_t j = 0; j < sp.alpha; j++) {
                        int64_t ydim = hfm * sp.out_dim + j;
                        if ((t_pad <= ydim) && (ydim < hp_max)) {
                            for (int64_t k = 0; k < sp.alpha; k++) {
                                int64_t xdim = wfm * sp.out_dim + k;
                                if ((l_pad <= xdim) && (xdim < wp_max)) {
                                    size_t src_off = src_off_f(p, img, 0, ic, 0,
                                            ydim - t_pad, xdim - l_pad);
                                    I[j][k] = ((float *)src_m)[src_off];
                                } else {
                                    I[j][k] = 0.f;
                                }
                            }
                        } else {
                            for (int64_t k = 0; k < sp.alpha; k++) {
                                I[j][k] = 0.f;
                            }
                        }
                    }

                    trans_I_4x4_3x3(_v, I);

                    /* scatter v:V */
                    for (int64_t j = 0; j < sp.alpha; j++) {
                        for (int64_t k = 0; k < sp.alpha; k++) {
                            V(j, k, img, hfm, wfm, ic) = _v[j][k];
                        }
                    }
                }
            }
        }
    }

#pragma omp for collapse(4)
    /* diff_dst transform */
    for (int64_t oc = 0; oc < p->oc; oc++) {
        for (int64_t img = 0; img < p->mb; img++) {
            for (int64_t hfm = 0; hfm < sp.h_tiles; hfm++) {
                for (int64_t wfm = 0; wfm < sp.w_tiles; wfm++) {
                    for (int64_t j = 0; j < sp.alpha; j++) {
                        int64_t ydim = hfm * sp.out_dim + j;
                        if (ydim < p->oh) {
                            for (int64_t k = 0; k < sp.alpha; k++) {
                                int64_t xdim = wfm * sp.out_dim + k;
                                if (xdim < p->ow) {
                                    size_t dst_off = dst_off_f(
                                            p, img, 0, oc, 0, ydim, xdim);
                                    O[j][k] = ((float *)diff_dst_m)[dst_off];
                                } else {
                                    O[j][k] = 0.f;
                                }
                            }
                        } else {
                            for (int64_t k = 0; k < sp.alpha; k++) {
                                O[j][k] = 0.f;
                            }
                        }
                    }
                    trans_W_3x3_4x4_wu(_m, O);

                    /* scatter v:V */
                    for (int64_t j = 0; j < sp.alpha; j++) {
                        for (int64_t k = 0; k < sp.alpha; k++) {
                            M(j, k, oc, img, hfm, wfm) = _m[j][k];
                        }
                    }
                }
            }
        }
    }

#pragma omp for collapse(2)
    /* GeMM U = M * V */
    for (int64_t j = 0; j < sp.alpha; ++j) {
        for (int64_t k = 0; k < sp.alpha; ++k) {
            gemm("C", "N", "N", p->oc, p->ic, p_dim, 1.0,
                    (float *)&(M(j, k, 0, 0, 0, 0)), p_dim,
                    (float *)&(V(j, k, 0, 0, 0, 0)), p->ic, 1.0,
                    (float *)&(U(j, k, 0, 0)), p->ic);
        }
    }

#pragma omp for collapse(2)
    for (int64_t oc = 0; oc < p->oc; ++oc) {
        for (int64_t ic = 0; ic < p->ic; ++ic) {
            for (int64_t j = 0; j < sp.alpha; j++) {
                for (int64_t k = 0; k < sp.alpha; k++) {
                    F[j][k] = U(j, k, oc, ic);
                }
            }

            trans_O_3x3_4x4_wu(F, _u);

            /* scatter u:U */
            for (int64_t kh = 0; kh < p->kh; kh++) {
                for (int64_t kw = 0; kw < p->kw; kw++) {
                    size_t wei_off = wei_off_f(p, 0, oc, ic, 0, kh, kw);
                    ((float *)diff_wei_m)[wei_off] = _u[kh][kw];
                }
            }
        }
    }
    }

    free_scratchpad(&sp);

    if (p->dir & FLAG_BIA)
        compute_ref_bwd_bias(p, diff_bia_m, diff_dst_m);
}

}
