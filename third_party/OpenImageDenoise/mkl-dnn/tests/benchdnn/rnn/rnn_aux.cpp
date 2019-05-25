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

#include "rnn/rnn_aux.hpp"
#include "norm.hpp"

#define DPRINT(...)                                     \
    do {                                                \
        int l = snprintf(buffer, rem_len, __VA_ARGS__); \
        buffer += l;                                    \
        rem_len -= l;                                   \
    } while (0)

namespace rnn {

alg_t str2alg(const char *str) {
#define CASE(_alg)                         \
    if (!strcasecmp(STRINGIFY(_alg), str)) \
    return _alg
    CASE(VANILLA_RNN);
    CASE(VANILLA_LSTM);
    CASE(VANILLA_GRU);
    CASE(LBR_GRU);
#undef CASE
    assert(!"unknown algorithm");
    return VANILLA_RNN;
}

policy_t str2policy(const char *str) {
#define CASE(_plc) if (!strcasecmp(STRINGIFY(_plc), str)) return _plc
    CASE(NONE);
    CASE(COMMON);
    CASE(PER_OC);
#undef CASE
    assert(!"unknown policy");
    return NONE;
}

const char * policy2str(policy_t policy) {
    if (policy == NONE) return "none";
    if (policy == COMMON) return "common";
    if (policy == PER_OC) return "per_oc";
    assert(!"unknown policy");
    return "unknown policy";
}

const char *alg2str(alg_t alg) {
    if (alg == VANILLA_RNN)
        return "VANILLA_RNN";
    if (alg == VANILLA_LSTM)
        return "VANILLA_LSTM";
    if (alg == VANILLA_GRU)
        return "VANILLA_GRU";
    if (alg == LBR_GRU)
        return "LBR_GRU";
    assert(!"unknown algorithm");
    return "unknown algorithm";
}

mkldnn_alg_kind_t alg2kind(alg_t alg) {
    if (alg == VANILLA_RNN)
        return mkldnn_vanilla_rnn;
    if (alg == VANILLA_LSTM)
        return mkldnn_vanilla_lstm;
    if (alg == VANILLA_GRU)
        return mkldnn_vanilla_gru;
    if (alg == LBR_GRU)
        return mkldnn_gru_linear_before_reset;
    assert(!"unknown algorithm");
    return mkldnn_alg_kind_undef;
}

activation_t str2activation(const char *str) {
#define CASE(_act)                         \
    if (!strcasecmp(STRINGIFY(_act), str)) \
    return _act
    CASE(RELU);
    CASE(LOGISTIC);
    CASE(TANH);
#undef CASE
    assert(!"unknown activation");
    return TANH;
}

const char *activation2str(activation_t act) {
    const char *str = "unknown activation";
    switch (act) {
    case RELU: str = "RELU"; break;
    case LOGISTIC: str = "LOGISTIC"; break;
    case TANH: str = "TANH"; break;
    default: assert(!"unknown activation");
    }
    return str;
}

mkldnn_alg_kind_t activation2kind(activation_t act) {
    mkldnn_alg_kind_t alg_kind = mkldnn_alg_kind_undef;
    switch (act) {
    case RELU: alg_kind = mkldnn_eltwise_relu; break;
    case LOGISTIC: alg_kind = mkldnn_eltwise_logistic; break;
    case TANH: alg_kind = mkldnn_eltwise_tanh; break;
    default: assert(!"unknown activation");
    }
    return alg_kind;
}

mkldnn_prop_kind_t str2prop(const char *str) {
    if (!strcasecmp("FWD_D", str))
        return mkldnn_forward;
    if (!strcasecmp("BWD_D", str))
        return mkldnn_backward;
    assert(!"unknown propagation");
    return mkldnn_forward;
}

const char *prop2str(mkldnn_prop_kind_t prop) {
    if (prop == mkldnn_forward)
        return "FWD_D";
    if (prop == mkldnn_backward)
        return "BWD_DW";
    assert(!"unknown propagation");
    return "unknown propagation";

}

mkldnn_rnn_direction_t str2direction(const char *str) {
    if (!strcasecmp("left2right", str))
        return mkldnn_unidirectional_left2right;
    if (!strcasecmp("right2left", str))
        return mkldnn_unidirectional_right2left;
    if (!strcasecmp("concat", str))
        return mkldnn_bidirectional_concat;
    if (!strcasecmp("sum", str))
        return mkldnn_bidirectional_sum;
    assert(!"unknown direction");
    return mkldnn_unidirectional_left2right;
}

const char *direction2str(mkldnn_rnn_direction_t direction) {
    if (direction == mkldnn_unidirectional_left2right)
        return "left2right";
    if (direction == mkldnn_unidirectional_right2left)
        return "right2left";
    if (direction == mkldnn_bidirectional_concat)
        return "concat";
    if (direction == mkldnn_bidirectional_sum)
        return "sum";
    assert(!"unknown direction");
    return "unknown direction";
}

int str2desc(rnn_desc_t *desc, const char *str) {
    rnn_desc_t d{0};

    /* canonical form:
     * lXtXmXsicXslcXdicXdlc
     *
     * where: X is number, S - string
     * note: symbol `_` is ignored
     *
     * implicit rules:
     *  - default values:
     *      l = 1, t = 1, mb = 2, S="wip"
     *  - if slc/dlc/dic is undefined => slc/dlc/dic = sic
     */

    d.n_layer = 1;
    d.n_iter = 1;
    d.mb = 2;
    d.name = "\"wip\"";

    const char *s = str;
    assert(s);

#   define CASE_NN(p, c) do { \
        if (!strncmp(p, s, strlen(p))) { \
            ok = 1; s += strlen(p); \
            char *end_s; d. c = strtol(s, &end_s, 10); s += (end_s - s); \
        } \
    } while (0)
#   define CASE_N(c) CASE_NN(#c, c)
    while (*s) {
        int ok = 0;
        CASE_NN("l", n_layer);
        CASE_NN("t", n_iter);
        CASE_N(mb);
        CASE_N(sic);
        CASE_N(slc);
        CASE_N(dic);
        CASE_N(dlc);
        if (*s == 'n') { d.name = s + 1; break; }
        if (*s == '_') ++s;
        if (!ok) return FAIL;
    }
#   undef CASE_NN
#   undef CASE_N

    if (d.sic == 0) return FAIL;
    if (d.slc == 0) d.slc = d.sic;
    if (d.dlc == 0) d.dlc = d.sic;
    if (d.dic == 0) d.dic = d.sic;

    *desc = d;

    return OK;
}


void prb2str(const rnn_prb_t *p, const res_t *res, char *buffer) {
    int rem_len = max_prb_len;

    DPRINT("--prop=%s --alg=%s --activation=%s --direction=%s --cfg=%s "
           "--scaling=%s ",
            prop2str(p->prop), alg2str(p->alg), activation2str(p->activation),
            direction2str(p->direction), cfg2str(p->cfg),
            policy2str(p->scale_policy));
    DPRINT("l" IFMT "", p->n_layer);
    DPRINT("t" IFMT "", p->n_iter);
    DPRINT("mb" IFMT "", p->mb);
    DPRINT("sic" IFMT "", p->sic);
    DPRINT("slc" IFMT "", p->slc);
    DPRINT("dic" IFMT "", p->dic);
    DPRINT("dlc" IFMT "", p->dlc);
    DPRINT("n\"%s\"", p->name);
}

void init_buffer(float *buf, int64_t size, float value) {
    for (int64_t i = 0; i < size; i++)
        buf[i] = value;
}

float logistic(float x) {
    if (x < 0)
        return (expf(x) / (1 + expf(x)));
    else
        return 1.0f - (expf(-x) / (1 + expf(-x)));
}
float dlogistic(float x) {
    float tmp = logistic(x);
    return tmp * (1 - tmp);
}
float dtanhf(float x) {
    return (1 - tanhf(x)) * (1 + tanhf(x));
}
float x_m_square(float x) {
    return x - x * x;
}
float relu(float x) {
    return x > 0 ? x : 0;
}
float drelu(float x) {
    return float(x > 0);
}
float one_m_square(float x) {
    return 1 - x * x;
}

int compare_dat(const rnn_prb_t *p, rnn_data_kind_t kind, dnn_mem_t &mem_dt,
        dnn_mem_t &mem_fp, res_t *r, bool final_compare = false) {
    size_t nelems = mem_dt.nelems();

    const char *skind = rnn_data_kind2str(kind);

    diff_norm_t diff_norm;

    r->errors = 0;
    r->total = nelems;

    for (size_t i = 0; i < nelems; ++i) {
        const float dt = ((float *)mem_dt)[i];
        const float fp = ((float *)mem_fp)[i];
        diff_norm.update(fp, dt);

        const float diff = fabsf(fp - dt);
        const float rel_diff = diff / (fabsf(fp) > FLT_MIN ? fabsf(fp) : 1);

        const bool ok = (fabs(fp) > 1e-5 ? rel_diff : diff) <= p->cfg[kind].eps;

        if (!ok) {
            r->errors++;
            if (r->errors < 10 || verbose >= 10) {
                int64_t n = 0, t = 0, c = 0, s = 0, l = 0, d = 0, w = 0, ic = 0,
                    oc = 0, b = 0;
                switch (kind) {
                case input:
                    inv_ntc_off_f(p, i, n, t, c);
                    print(0, "%lu, %s, [%s][" IFMT "," IFMT "," IFMT "] "
                             "fp:%8g dt:%8g diff:%8g rdiff:%8g\n",
                            (unsigned long)i,
                            final_compare == false ? "REORDER " : "", skind, n,
                            t, c, fp, dt, diff, rel_diff);
                    break;
                case states:
                    inv_ldsnc_off_f(p, i, l, d, s, n, c);
                    print(0, "%lu, %s, [%s][" IFMT "," IFMT "," IFMT "," IFMT "," IFMT "] "
                             "fp:%8g dt:%8g diff:%8g rdiff:%8g\n",
                            (unsigned long)i,
                            final_compare == false ? "REORDER " : "", skind, l,
                            d, s, n, c, fp, dt, diff, rel_diff);
                    break;
                case weights_input:
                    inv_ldigo_off_f(p, i, l, d, w, ic, oc);
                    print(0, "%lu, %s, [%s][" IFMT "," IFMT "," IFMT "," IFMT "," IFMT "] "
                             "fp:%8g dt:%8g diff:%8g rdiff:%8g\n",
                            (unsigned long)i,
                            final_compare == false ? "REORDER " : "", skind, l,
                            d, w, ic, oc, fp, dt, diff, rel_diff);
                    break;
                case weights_states:
                    inv_ldigo_off_f(p, i, l, d, w, ic, oc);
                    print(0, "%lu, %s, [%s][" IFMT "," IFMT "," IFMT "," IFMT "," IFMT "] "
                             "fp:%8g dt:%8g diff:%8g rdiff:%8g\n",
                            (unsigned long)i,
                            final_compare == false ? "REORDER " : "", skind, l,
                            d, w, ic, oc, fp, dt, diff, rel_diff);
                    break;
                case bias:
                    inv_ldgo_off_f(p, i, l, d, b, c);
                    print(0, "%lu, %s, [%s][" IFMT "," IFMT "," IFMT "," IFMT "] "
                             "fp:%8g dt:%8g diff:%8g rdiff:%8g\n",
                            (unsigned long)i,
                            final_compare == false ? "REORDER " : "", skind, l,
                            d, b, c, fp,  dt, diff, rel_diff);
                    break;
                case dst_last_layer:
                    inv_tnc_off_f(p, i, s, t, n, c);
                    print(0, "%lu, %s, [%s][" IFMT "," IFMT "," IFMT "," IFMT "] "
                             "fp:%8g dt:%8g diff:%8g rdiff:%8g\n",
                            (unsigned long)i,
                            final_compare == false ? "REORDER " : "", skind, s,
                            t, n, c, fp, dt, diff, rel_diff);
                    break;
                case dst_last_iteration:
                    inv_ldsnc_off_f(p, i, l, d, s, n, c);
                    print(0, "%lu, %s, [%s][" IFMT "," IFMT "," IFMT "," IFMT "," IFMT " "
                             "fp:%8g dt:%8g diff:%8g rdiff:%8g\n",
                            (unsigned long)i,
                            final_compare == false ? "REORDER " : "", skind, l,
                            d, s, n, c, fp, dt, diff, rel_diff);
                    break;
                default: assert("unknown data kind"); return FAIL;
                }
            }
        }

#if 1
        /* for debug purposes only: dump the output */
        if (final_compare && verbose >= 50) {
            int64_t n = 0, t = 0, c = 0, s = 0, l = 0, d = 0, w = 0, ic = 0, oc = 0,
                b = 0;

            switch (kind) {
            case input:
                inv_ntc_off_f(p, i, n, t, c);
                print(0, "[%4lu][%s][" IFMT "," IFMT "," IFMT "] fp:%8g dt:%8g\n",
                        (unsigned long)i, skind, n, t, c, fp, dt);
                break;
            case states:
                inv_ldsnc_off_f(p, i, l, d, s, n, c);
                print(0, "[%4lu][%s][" IFMT "," IFMT "," IFMT "," IFMT "," IFMT "] fp:%8g dt:%8g\n",
                        (unsigned long)i, skind, l, d, s, n, c, fp, dt);
                break;
            case weights_input:
                inv_ldigo_off_f(p, i, l, d, w, ic, oc);
                print(0, "[%4lu][%s][" IFMT "," IFMT "," IFMT "," IFMT "," IFMT "] fp:%8g dt:%8g\n",
                        (unsigned long)i, skind, l, d, w, ic, oc, fp, dt);
                break;
            case weights_states:
                inv_ldigo_off_f(p, i, l, d, w, ic, oc);
                break;
                print(0, "[%4lu][%s][" IFMT "," IFMT "," IFMT "," IFMT "," IFMT "] fp:%8g dt:%8g\n",
                        (unsigned long)i, skind, l, d, w, ic, oc, fp, dt);
            case bias:
                inv_ldgo_off_f(p, i, l, d, b, c);
                break;
                print(0, "[%4lu][%s][" IFMT "," IFMT "," IFMT "," IFMT "] fp:%8g dt:%8g\n",
                        (unsigned long)i, skind, l, d, b, c, fp, dt);
            case dst_last_layer:
                inv_tnc_off_f(p, i, s, t, n, c);
                print(0, "[%4lu][%s][" IFMT "," IFMT "," IFMT "] fp:%8g dt:%8g\n",
                        (unsigned long)i, skind, n, t, c, fp, dt);
                break;
            case dst_last_iteration:
                inv_ldsnc_off_f(p, i, l, d, s, n, c);
                print(0, "[%4lu][%s][" IFMT "," IFMT "," IFMT "," IFMT "," IFMT "] fp:%8g dt:%8g\n",
                        (unsigned long)i, skind, l, d, s, n, c, fp, dt);
                break;
            default:
                print(0, "[%4lu][unknown] fp:%8g dt:%8g\n",
                        (unsigned long)i, fp, dt);
                break;
            }
        }
#endif
    }

    diff_norm.done();

    if (final_compare || r->errors) {
        const int vl = r->errors ? 0 : 2;
        print(vl,
                "@@@ [%s] %sdiff: l0(``%g``) "
                "l1:(%g,%g,%g,``%g``) "
                "l2:(%g,%g,%g,``%g``) "
                "l8:(%g,%g,%g,``%g``)\n",
                skind, final_compare ? "final: " : "",
                diff_norm.rel_diff(norm_t::L0), diff_norm.a_[norm_t::L1],
                diff_norm.b_[norm_t::L1], diff_norm.diff_[norm_t::L1],
                diff_norm.rel_diff(norm_t::L1), diff_norm.a_[norm_t::L2],
                diff_norm.b_[norm_t::L2], diff_norm.diff_[norm_t::L2],
                diff_norm.rel_diff(norm_t::L2), diff_norm.a_[norm_t::L8],
                diff_norm.b_[norm_t::L8], diff_norm.diff_[norm_t::L8],
                diff_norm.rel_diff(norm_t::L8));
    }

    if (r->errors)
        r->state = FAILED;

    if (final_compare && r->state == UNTESTED)
        r->state = PASSED; /* optimism */

    return r->state == FAILED ? FAIL : OK;
}

int compare_input(const rnn_prb_t *p, dnn_mem_t &mem_dt, dnn_mem_t &mem_fp,
        res_t *r, bool final_compare = false) {
    return compare_dat(p, input, mem_dt, mem_fp, r, final_compare);
}
int compare_states(const rnn_prb_t *p, dnn_mem_t &mem_dt, dnn_mem_t &mem_fp,
        res_t *r, bool final_compare = false) {
    return compare_dat(p, states, mem_dt, mem_fp, r, final_compare);
}
int compare_weights_input(const rnn_prb_t *p, dnn_mem_t &mem_dt,
        dnn_mem_t &mem_fp, res_t *r, bool final_compare = false) {
    return compare_dat(p, weights_input, mem_dt, mem_fp, r, final_compare);
}
int compare_weights_states(const rnn_prb_t *p, dnn_mem_t &mem_dt,
        dnn_mem_t &mem_fp, res_t *r, bool final_compare = false) {
    return compare_dat(p, weights_states, mem_dt, mem_fp, r, final_compare);
}
int compare_bias(const rnn_prb_t *p, dnn_mem_t &mem_dt, dnn_mem_t &mem_fp,
        res_t *r, bool final_compare = false) {
    return compare_dat(p, bias, mem_dt, mem_fp, r, final_compare);
}
int compare_dst_last_layer(const rnn_prb_t *p, dnn_mem_t &mem_dt,
        dnn_mem_t &mem_fp, res_t *r, bool final_compare = false) {
    return compare_dat(p, dst_last_layer, mem_dt, mem_fp, r, final_compare);
}
int compare_dst_last_iteration(const rnn_prb_t *p, dnn_mem_t &mem_dt,
        dnn_mem_t &mem_fp, res_t *r, bool final_compare = false) {
    return compare_dat(p, dst_last_iteration, mem_dt, mem_fp, r, final_compare);
}

void rnn_prb_t::set_qparams(float fp_min, float fp_max) {
    if (cfg == conf_f32) {
        data_shift = 0.;
        data_scale = 1.;
        wei_scale = 1.;
        return;
    }

    /* Set parameters for quantization of src and weights from fp32 data
     * in [-1, 1] to int8 data in a range specified in cfg */
    float fp_range = fp_max - fp_min;
    float int8_src_range = cfg[input].f_max - cfg[input].f_min,
          int8_wei_range = cfg[weights_input].f_max - cfg[weights_input].f_min;

    data_shift = cfg[input].f_mean;
    data_scale = int8_src_range / fp_range;

    if (scale_policy == COMMON) {
        wei_scale = int8_wei_range / fp_range;
    } else if (scale_policy == PER_OC) {
        float K = int8_wei_range / fp_range;
        int64_t nelems = dic * n_gates();
        for (int64_t i = 0; i < nelems; i++) {
            wei_oc_scales[i] = K * (1. + (float)i / nelems);
        }
    }
}

} // namespace rnn
