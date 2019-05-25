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

#ifndef _LSTM_HPP
#define _LSTM_HPP

#include <assert.h>
#include <limits.h>
#include <stdint.h>

#include "common.hpp"
#include "dnn_types.hpp"
#include "mkldnn_common.hpp"
#include "mkldnn_debug.hpp"
#include "mkldnn_memory.hpp"

namespace rnn {

extern const char *perf_template;

enum alg_t { VANILLA_RNN, VANILLA_LSTM, VANILLA_GRU, LBR_GRU };
alg_t str2alg(const char *str);
const char *alg2str(alg_t alg);
mkldnn_alg_kind_t alg2kind(alg_t alg);

enum activation_t { RELU, LOGISTIC, TANH };
activation_t str2activation(const char *str);
const char *activation2str(activation_t alg);
mkldnn_alg_kind_t activation2kind(activation_t alg);

mkldnn_prop_kind_t str2prop(const char *str);
const char *prop2str(mkldnn_prop_kind_t prop);

mkldnn_rnn_direction_t str2direction(const char *str);
const char *direction2str(mkldnn_rnn_direction_t direction);

const int H = 0;
const int C = 1;

template <typename Telem>
struct array_offset_calculator {
    template <typename... Targs>
    array_offset_calculator(Telem *base, Targs... Fargs)
        : _size(sizeof...(Fargs)) {
        const int64_t init_list[] = { Fargs... };
        _dims = new int64_t[_size];
        for (int64_t i = 0; i < _size; ++i)
            _dims[i] = init_list[i];

        _base_ptr = base;
    }
    ~array_offset_calculator() { delete[] _dims; }
    template <typename... Targs>
    inline Telem &operator()(Targs... Fargs) {
        return *(_base_ptr + _offset(1, Fargs...));
    }

private:
    template <typename... Targs>
    inline int64_t _offset(int64_t const dimension, int64_t element) {
        return element;
    }

    template <typename... Targs>
    inline int64_t _offset(int64_t const dimension, int64_t theta, int64_t element) {
        return element + (_dims[dimension] * theta);
    }

    template <typename... Targs>
    inline int64_t _offset(
            int64_t const dimension, int64_t theta, int64_t element, Targs... Fargs) {
        int64_t t_prime = element + (_dims[dimension] * theta);
        return _offset(dimension + 1, t_prime, Fargs...);
    }

    Telem *_base_ptr;
    int64_t _size;
    int64_t *_dims;
};

struct rnn_desc_t {
    int64_t sic;
    int64_t slc;
    int64_t dic;
    int64_t dlc;
    int64_t mb;
    int64_t n_layer;
    int64_t n_iter;
    const char *name;
};
int str2desc(rnn_desc_t *desc, const char *str);

enum rnn_data_kind_t {
    input,
    states,
    weights_input,
    weights_states,
    bias,
    dst_last_iteration,
    dst_last_layer,
    dst_diff_input,
    dst_diff_states,
    dst_diff_weights_input,
    dst_diff_weights_states,
    dst_diff_bias,
    diff_last_iteration,
    diff_last_layer,
    data_kind_total // should be last to provide the total number of data kinds
};

inline const char *rnn_data_kind2str(rnn_data_kind_t kind) {
    switch (kind) {
    case input: return "INPUT";
    case states: return "STATES";
    case weights_input: return "WEIGHTS_INPUT";
    case weights_states: return "WEIGHTS_STATES";
    case bias: return "BIAS";
    case dst_last_layer: return "DST_LAST_LAYER";
    case dst_last_iteration: return "DST_LAST_ITERATION";
    default:
        assert(!"incorrect rnn data kind");
        return "incorrect rnn data kind";
    }
}

/** configuration structure, that controls initial data filling + error check
*
* dt defines precision
*
* for each lst data kind the values are filled as follows:
* if (rand() > f_sparsity) then:
*     v <-- f_base
* else:
*     v <-- f_min + rand() * f_step % (f_max - f_min)
*
* on final check the resulting values should be in [min .. max] range, the
* relative difference should not exceed eps
*/

typedef struct dt_conf_t {
    mkldnn_data_type_t dt;
    int min, max; /* representative */
    int f_min, f_max; /* fill range */
    float f_mean, f_var; /* mean and variance of normally distributed data */
    double eps; /* acceptable error */
} _dt_conf_t[data_kind_total];

extern const _dt_conf_t conf_f32;
extern const _dt_conf_t conf_u8u8u8u8;
extern const _dt_conf_t conf_u8u8u8f32;
extern const _dt_conf_t conf_f32u8f32f32;
extern const _dt_conf_t conf_f32u8f32u8;

const dt_conf_t *str2cfg(const char *str);
const char *cfg2str(const dt_conf_t *cfg);

enum policy_t { NONE = 0, COMMON, PER_OC };
policy_t str2policy(const char *str);
const char *policy2str(attr_t::scale_t::policy_t policy);

struct rnn_prb_t : public rnn_desc_t {
    rnn_prb_t(const rnn_desc_t desc, const dt_conf_t *cfg,
            mkldnn_prop_kind_t prop, alg_t alg,
            mkldnn_rnn_direction_t direction, activation_t activation,
            const attr_t &attr, policy_t scale_policy, int mb = 0)
        : rnn_desc_t(desc)
        , cfg(cfg)
        , prop(prop)
        , alg(alg)
        , direction(direction)
        , activation(activation)
        , attr(attr)
        , scale_policy(scale_policy) {
        if (mb) this->mb = mb;
        wei_oc_scales = NULL;
        if (scale_policy == PER_OC)
            wei_oc_scales
                    = (float *)zmalloc(sizeof(float) * dic * n_gates(), 64);
        set_qparams(-1., 1.);
    }
    ~rnn_prb_t() {
        if (wei_oc_scales)
            zfree(wei_oc_scales);
    }

    int64_t n_directions() const {
        return (direction == mkldnn_bidirectional_concat
                       || direction == mkldnn_bidirectional_sum) ?
                2 :
                1;
    }
    int64_t n_weights() const { return 1; }
    int64_t n_states() const { return alg == VANILLA_LSTM ? 2 : 1; }
    int64_t n_gates() const {
        return alg == VANILLA_LSTM ?
                4 :
                (alg == VANILLA_GRU || alg == LBR_GRU ? 3 : 1);
    }
    int64_t n_bias() const {
        return alg == LBR_GRU ? n_gates() + 1 : n_gates();
    }

    const dt_conf_t *cfg;
    mkldnn_prop_kind_t prop;
    alg_t alg;
    mkldnn_rnn_direction_t direction;
    activation_t activation;
    attr_t attr;
    policy_t scale_policy;

    float data_scale, data_shift;
    float wei_scale;
    float *wei_oc_scales;

private:
    void set_qparams(float fp_min, float fp_max);
    rnn_prb_t(const rnn_prb_t &) = delete;
    rnn_prb_t &operator=(const rnn_prb_t &) = delete;
};

const size_t max_prb_len = 392;
void prb2str(const rnn_prb_t *p, const res_t *res, char *buffer);

void compute_ref_fwd(const rnn_prb_t *p, dnn_mem_t &input_m,
        dnn_mem_t &states_m, dnn_mem_t &weights_input_m,
        dnn_mem_t &weights_states_m, dnn_mem_t &bias_m,
        dnn_mem_t &dst_last_layer_m, dnn_mem_t &dst_last_iteration_m,
        mkldnn_rnn_direction_t direction);

void compute_ref_bwd(const rnn_prb_t *p, dnn_mem_t &input_m,
        dnn_mem_t &states_m, dnn_mem_t &diff_last_layer_m,
        dnn_mem_t &diff_last_iteration_m, dnn_mem_t &weights_input_m,
        dnn_mem_t &weights_states_m, dnn_mem_t &bias_m,
        dnn_mem_t &dst_last_layer_m, dnn_mem_t &dst_last_iteration_m,
        dnn_mem_t &dst_diff_input_m, dnn_mem_t &dst_diff_states_m,
        dnn_mem_t &dst_diff_weights_input_m,
        dnn_mem_t &dst_diff_weights_states_m, dnn_mem_t &dst_diff_bias_m,
        mkldnn_rnn_direction_t direction);

// mkldnn_ntc
inline size_t ntc_off_f(const rnn_prb_t *p, int64_t n, int64_t t, int64_t c) {
    return (n * p->n_iter + t) * p->slc + c;
}

inline void inv_ntc_off_f(const rnn_prb_t *p,
        size_t off, int64_t &n, int64_t &t, int64_t &c) {
    c = off % p->slc;
    off /= p->slc;
    t = off % p->n_iter;
    off /= p->n_iter;
    n = off % p->mb;
    off /= p->mb;
    assert(off == 0);
}

// mkldnn_ldsnc
inline size_t ldsnc_off_f(const rnn_prb_t *p,
        int64_t l, int64_t d, int64_t s, int64_t n, int64_t c) {
    return (((l * p->n_directions() + d) * p->n_states() + s) * p->mb + n)
            * p->sic + c;
}

inline void inv_ldsnc_off_f(const rnn_prb_t *p, size_t off,
        int64_t &l, int64_t &d, int64_t &s, int64_t &n, int64_t &c) {
    c = off % p->sic;
    off /= p->sic;
    n = off % p->mb;
    off /= p->mb;
    s = off % p->n_states();
    off /= p->n_states();
    d = off % p->n_directions();
    off /= p->n_directions();
    l = off % p->n_layer;
    off /= p->n_layer;
    assert(off == 0);
}

// mkldnn_ldigo
inline size_t ldigo_off_f(const rnn_prb_t *p,
        int64_t l, int64_t d, int64_t w, int64_t ic, int64_t oc) {
    return (((l * p->n_directions() + d) * p->n_weights() + w) * (4 * p->slc)
            + ic) * p->sic + oc;
}

inline void inv_ldigo_off_f(const rnn_prb_t *p, size_t off,
        int64_t &l, int64_t &d, int64_t &w, int64_t &ic, int64_t &oc) {
    oc = off % p->sic;
    off /= p->sic;
    ic = off % (4 * p->slc);
    off /= (4 * p->slc);
    w = off % p->n_weights();
    off /= p->n_weights();
    d = off % p->n_directions();
    off /= p->n_directions();
    l = off % p->n_layer;
    off /= p->n_layer;
    assert(off == 0);
}

// mkldnn_ldwOcIc
inline size_t ldwOcIc_off_f(const rnn_prb_t *p,
        int64_t l, int64_t d, int64_t w, int64_t oc, int64_t ic) {
    return (((l * p->n_directions() + d) * p->n_weights() + w) * (4 * p->sic)
            + oc) * p->slc + ic;
}

inline void inv_ldwOcIc_off_f(const rnn_prb_t *p, size_t off, int64_t &l, int64_t &d,
        int64_t &w, int64_t &oc, int64_t &ic) {
    ic = off % p->slc;
    off /= p->slc;
    oc = off % (4 * p->sic);
    off /= (4 * p->sic);
    w = off % p->n_weights();
    off /= p->n_weights();
    d = off % p->n_directions();
    off /= p->n_directions();
    l = off % p->n_layer;
    off /= p->n_layer;
    assert(off == 0);
}

// bias: mkldnn_ldgo
inline size_t ldgo_off_f(const rnn_prb_t *p,
        int64_t l, int64_t d, int64_t b, int64_t c) {
    return ((l * p->n_directions() + d) * p->n_bias() + b) * p->sic + c;
}

inline void inv_ldgo_off_f(const rnn_prb_t *p, size_t off,
        int64_t &l, int64_t &d, int64_t &b, int64_t &c) {
    c = off % p->sic;
    off /= p->sic;
    b = off % p->n_bias();
    off /= p->n_bias();
    d = off % p->n_directions();
    off /= p->n_directions();
    l = off % p->n_layer;
    off /= p->n_layer;
    assert(off == 0);
}

// dst_last_layer: mkldnn_tnc
inline size_t tnc_off_f(const rnn_prb_t *p,
        int64_t s, int64_t t, int64_t n, int64_t c) {
    return ((s * p->n_iter + t) * p->mb + n) * p->sic + c;
}

inline void inv_tnc_off_f(
        const rnn_prb_t *p, size_t off, int64_t &s, int64_t &t, int64_t &n, int64_t &c) {
    c = off % p->sic;
    off /= p->sic;
    n = off % p->mb;
    off /= p->mb;
    t = off % p->n_iter;
    off /= p->n_iter;
    s = off % p->n_states();
    off /= p->n_states();
    assert(off == 0);
}

void perf_report(const rnn_prb_t *p, const res_t *r, const char *pstr);

int doit(const rnn_prb_t *p, res_t *res);
void check(rnn_desc_t *p);
int bench(int argc, char **argv, bool main_bench = true);
} // namespace rnn

#endif
