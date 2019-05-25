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

#include "mkldnn_common.hpp"
#include "rnn/rnn.hpp"

namespace rnn {

/* cfgs definition
arrays:
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
params: {data_type, min, max, f_min, f_max, f_mean, f_var, eps}
*/

const int int_max_exact = 1 << 24;
const _dt_conf_t conf_f32 = {
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.001f, 1e-5 }, //input
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.001f, 1e-5 }, //states
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.001f, 1e-5 }, //weights_input
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.001f, 1e-5 }, //weights_states
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.001f, 1e-5 }, //bias
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.001f, 1e-5 }, //dst_last_iteration
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.001f, 1e-5 }, //dst_last_layer
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.001f, 1e-5 }, //dst_diff_input
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.001f, 1e-5 }, //dst_diff_states
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.001f, 1e-5 }, //dst_diff_weights_input
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.001f, 1e-5 }, //dst_diff_weights_states
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.001f, 1e-5 }, //dst_diff_bias
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.001f, 1e-5 }, //diff_last_iteration
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.001f, 1e-5 }, //diff_last_layer
};
const _dt_conf_t conf_u8u8u8u8 = {
    { mkldnn_u8, 0, UINT8_MAX, 0, 127, 64.f, 5.f, 0. }, //input
    { mkldnn_u8, 0, UINT8_MAX, 0, 127, 64.f, 5.f, 0. }, //states
    { mkldnn_s8, INT8_MIN, INT8_MAX, -63, 63, 0.f, 10.f, 0. }, //weights_input
    { mkldnn_s8, INT8_MIN, INT8_MAX, -63, 63, 0.f, 10.f, 0. }, //weights_states
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.01f, 0. }, //bias
    { mkldnn_u8, 0, UINT8_MAX, 0, 127, 64.f, 10.f, 0. }, //dst_iter
    { mkldnn_u8, 0, UINT8_MAX, 0, 127, 64.f, 10.f, 0. }, //dst_layer
};
const _dt_conf_t conf_u8u8u8f32 = {
    { mkldnn_u8, 0, UINT8_MAX, 0, 127, 64.f, 5.f, 0. }, //input
    { mkldnn_u8, 0, UINT8_MAX, 0, 127, 64.f, 5.f, 0. }, //states
    { mkldnn_s8, INT8_MIN, INT8_MAX, -63, 63, 0.f, 10.f, 0. }, //weights_input
    { mkldnn_s8, INT8_MIN, INT8_MAX, -63, 63, 0.f, 10.f, 0. }, //weights_states
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.01f, 0. }, //bias
    { mkldnn_u8, 0, UINT8_MAX, 0, 127, 64.f, 10.f, 0. }, //dst_iter
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.001f, 1e-5 }, //dst_last_layer
};
const _dt_conf_t conf_f32u8f32u8 = {
    { mkldnn_u8, 0, UINT8_MAX, 0, 127, 64.f, 5.f, 0. }, //input
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.05f, 1e-5 }, //states
    { mkldnn_s8, INT8_MIN, INT8_MAX, -63, 63, 0.f, 10.f, 0. }, //weights_input
    { mkldnn_s8, INT8_MIN, INT8_MAX, -63, 63, 0.f, 10.f, 0. }, //weights_states
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.01f, 0. }, //bias
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.01f, 1e-5 }, //dst_iter
    { mkldnn_u8, 0, UINT8_MAX, 0, 127, 64.f, 10.f, 0. }, //dst_layer
};
const _dt_conf_t conf_f32u8f32f32 = {
    { mkldnn_u8, 0, UINT8_MAX, 0, 127, 64.f, 5.f, 0. }, //input
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.05f, 1e-5 }, //states
    { mkldnn_s8, INT8_MIN, INT8_MAX, -63, 63, 0.f, 10.f, 0. }, //weights_input
    { mkldnn_s8, INT8_MIN, INT8_MAX, -63, 63, 0.f, 10.f, 0. }, //weights_states
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.01f, 0. }, //bias
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.01f, 1e-5 }, //dst_iter
    { mkldnn_f32, -int_max_exact, int_max_exact, -1, 1, 0.f, 0.01f, 1e-5 }, //dst_last_layer
};

const dt_conf_t *str2cfg(const char *str) {
#define CASE(cfg)                         \
    if (!strcasecmp(STRINGIFY(cfg), str)) \
    return CONCAT2(conf_, cfg)
    CASE(f32);
    CASE(u8u8u8u8);
    CASE(u8u8u8f32);
    CASE(f32u8f32u8);
    CASE(f32u8f32f32);
#undef CASE
    []() {
        SAFE(FAIL, CRIT);
        return 0;
    }();
    return (const dt_conf_t *)1;
}

const char *cfg2str(const dt_conf_t *cfg) {
#define CASE(_cfg)                   \
    if (cfg == CONCAT2(conf_, _cfg)) \
    return STRINGIFY(_cfg)
    CASE(f32);
    CASE(u8u8u8u8);
    CASE(u8u8u8f32);
    CASE(f32u8f32u8);
    CASE(f32u8f32f32);
#undef CASE
    []() {
        SAFE(FAIL, CRIT);
        return 0;
    }();
    return NULL;
}
} // namespace rnn
