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

#include <string.h>
#include <stdlib.h>

#include "common.hpp"
#include "mkldnn_common.hpp"

#include "self/self.hpp"

namespace self {

static int check_simple_enums() {
    /* attr::post_ops::kind */
    using p = attr_t::post_ops_t;
    CHECK_CASE_STR_EQ(p::kind2str(p::kind_t::SUM), "sum");
    CHECK_CASE_STR_EQ(p::kind2str(p::kind_t::RELU), "relu");

    CHECK_EQ(p::str2kind("sum"), p::kind_t::SUM);
    CHECK_EQ(p::str2kind("SuM"), p::kind_t::SUM);

    CHECK_EQ(p::str2kind("relu"), p::kind_t::RELU);
    CHECK_EQ(p::str2kind("ReLU"), p::kind_t::RELU);

    return OK;
}

static int check_attr2str() {
    char str[max_attr_len] = {'\0'};

    attr_t attr;
    CHECK_EQ(attr.is_def(), true);

#   define CHECK_ATTR_STR_EQ(attr, s) \
    do { \
        attr2str(&attr, str); \
        CHECK_CASE_STR_EQ(str, s); \
    } while(0)

    CHECK_ATTR_STR_EQ(attr, "oscale=none:1;post_ops=''");

    attr.oscale.policy = attr_t::scale_t::policy_t::COMMON;
    attr.oscale.scale = 2.4;
    CHECK_ATTR_STR_EQ(attr, "oscale=common:2.4;post_ops=''");

    attr.oscale.policy = attr_t::scale_t::policy_t::PER_OC;
    attr.oscale.scale = 3.2;
    CHECK_ATTR_STR_EQ(attr, "oscale=per_oc:3.2;post_ops=''");

    attr.oscale.policy = attr_t::scale_t::policy_t::PER_DIM_01;
    attr.oscale.scale = 3.2;
    CHECK_ATTR_STR_EQ(attr, "oscale=per_dim_01:3.2;post_ops=''");

#   undef CHECK_ATTR_STR_EQ

    return OK;
}

static int check_str2attr() {
    attr_t attr;

#   define CHECK_ATTR(str, os_policy, os_scale) \
    do { \
        CHECK_EQ(str2attr(&attr, str), OK); \
        CHECK_EQ(attr.oscale.policy, attr_t::scale_t::policy_t:: os_policy); \
        CHECK_EQ(attr.oscale.scale, os_scale); \
    } while(0)

    CHECK_ATTR("", NONE, 1.);
    CHECK_EQ(attr.is_def(), true);

    CHECK_ATTR("oscale=none:1.0", NONE, 1.);
    CHECK_EQ(attr.is_def(), true);

    CHECK_ATTR("oscale=none:2.0", NONE, 2.);
    CHECK_ATTR("oscale=common:2.0", COMMON, 2.);
    CHECK_ATTR("oscale=per_oc:.5;", PER_OC, .5);
    CHECK_ATTR("oscale=none:.5;oscale=common:1.5", COMMON, 1.5);

#   undef CHECK_ATTR

    return OK;
}

static int check_post_ops2str() {
    char str[max_attr_len] = {'\0'};

    attr_t::post_ops_t ops;
    CHECK_EQ(ops.is_def(), true);

#   define CHECK_OPS_STR_EQ(ops, s) \
    do { \
        ops.to_str(str, NULL); \
        CHECK_CASE_STR_EQ(str, s); \
    } while(0)

    CHECK_OPS_STR_EQ(ops, "''");

    ops.len = 4;
    for (int i = 0; i < 2; ++i) {
        ops.entry[2*i + 0].kind = attr_t::post_ops_t::SUM;
        ops.entry[2*i + 0].sum.scale = 2. + i;
        ops.entry[2*i + 1].kind = attr_t::post_ops_t::RELU;
        ops.entry[2*i + 1].eltwise.scale = 1.;
        ops.entry[2*i + 1].eltwise.alpha = 0.;
        ops.entry[2*i + 1].eltwise.beta = 0.;
    }
    CHECK_OPS_STR_EQ(ops, "'sum:2;relu;sum:3;relu'");

    ops.len = 3;
    CHECK_OPS_STR_EQ(ops, "'sum:2;relu;sum:3'");

    ops.len = 2;
    CHECK_OPS_STR_EQ(ops, "'sum:2;relu'");

    ops.len = 1;
    CHECK_OPS_STR_EQ(ops, "'sum:2'");

#   undef CHECK_OPS_STR_EQ

    return OK;
}

static int check_str2post_ops() {
    attr_t::post_ops_t ops;

    CHECK_EQ(ops.is_def(), true);

    auto quick = [&](int len) -> int {
        for (int i = 0; i < 2; ++i) {
            if (2*i + 0 >= len) return OK;
            CHECK_EQ(ops.entry[2*i + 0].kind, attr_t::post_ops_t::SUM);
            CHECK_EQ(ops.entry[2*i + 0].sum.scale, 2. + i);
            if (2*i + 1 >= len) return OK;
            CHECK_EQ(ops.entry[2*i + 1].kind, attr_t::post_ops_t::RELU);
            CHECK_EQ(ops.entry[2*i + 1].eltwise.scale, 1.);
            CHECK_EQ(ops.entry[2*i + 1].eltwise.alpha, 0.);
            CHECK_EQ(ops.entry[2*i + 1].eltwise.beta, 0.);
        }
        return OK;
    };

    ops.from_str("''", NULL);
    CHECK_EQ(ops.is_def(), true);

    ops.from_str("'sum:2;'", NULL);
    CHECK_EQ(quick(1), OK);

    ops.from_str("'sum:2;relu'", NULL);
    CHECK_EQ(quick(2), OK);

    ops.from_str("'sum:2;relu;sum:3'", NULL);
    CHECK_EQ(quick(3), OK);

    ops.from_str("'sum:2;relu;sum:3;relu;'", NULL);
    CHECK_EQ(quick(4), OK);

    return OK;
}

void common() {
    RUN(check_simple_enums());
    RUN(check_attr2str());
    RUN(check_str2attr());
    RUN(check_post_ops2str());
    RUN(check_str2post_ops());
}

}
