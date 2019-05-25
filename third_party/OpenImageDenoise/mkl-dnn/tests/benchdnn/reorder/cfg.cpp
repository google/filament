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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

#include "mkldnn.h"
#include "mkldnn_common.hpp"

#include "reorder.hpp"

namespace reorder {

const int int_max_exact = 1<<24;

#define REG(dt, min, range) \
const dt_conf_s CONCAT2(_conf_,dt) = {CONCAT2(mkldnn_,dt), min, range}; \
const dt_conf_t CONCAT2(conf_,dt) = &CONCAT2(_conf_,dt);

REG(f32, -int_max_exact, 2 * int_max_exact);
REG(s32, -int_max_exact, 2 * int_max_exact);
REG( s8,       INT8_MIN,     -2 * INT8_MIN);
REG( u8,              0,         UINT8_MAX);

#undef REG

dt_conf_t dt2cfg(mkldnn_data_type_t dt) {
#define CASE(cfg) if (CONCAT2(mkldnn_,cfg) == dt) return CONCAT2(conf_,cfg)
    CASE(f32);
    CASE(s32);
    CASE(s8);
    CASE(u8);
#undef CASE
    SAFE_V(FAIL);
    return conf_f32;
}

mkldnn_data_type_t cfg2dt(dt_conf_t cfg) {
#define CASE(_cfg) if (cfg == CONCAT2(conf_,_cfg)) \
    return CONCAT2(mkldnn_,_cfg)
    CASE(f32);
    CASE(s32);
    CASE(s8);
    CASE(u8);
#undef CASE
    SAFE_V(FAIL);
    return mkldnn_f32;
}

}
