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

#include "self/self.hpp"
#include "bnorm/bnorm.hpp"

using namespace bnorm;

namespace self {

static int check_flags() {
    CHECK_CASE_STR_EQ(flags2str((flags_t)0), "");
    CHECK_CASE_STR_EQ(flags2str(GLOB_STATS), "G");
    CHECK_CASE_STR_EQ(flags2str(USE_SCALESHIFT), "S");
    CHECK_CASE_STR_EQ(flags2str(FUSE_BN_RELU), "R");
    CHECK_CASE_STR_EQ(flags2str(GLOB_STATS | USE_SCALESHIFT), "GS");
    CHECK_CASE_STR_EQ(flags2str(GLOB_STATS | FUSE_BN_RELU), "GR");
    CHECK_CASE_STR_EQ(flags2str(USE_SCALESHIFT | FUSE_BN_RELU), "SR");
    CHECK_CASE_STR_EQ(flags2str(GLOB_STATS | USE_SCALESHIFT | FUSE_BN_RELU),
            "GSR");

    CHECK_EQ(str2flags(""), 0);
    CHECK_EQ(str2flags("G"), GLOB_STATS);
    CHECK_EQ(str2flags("S"), USE_SCALESHIFT);
    CHECK_EQ(str2flags("R"), FUSE_BN_RELU);
    CHECK_EQ(str2flags("GS"), GLOB_STATS | USE_SCALESHIFT);
    CHECK_EQ(str2flags("GR"), GLOB_STATS | FUSE_BN_RELU);
    CHECK_EQ(str2flags("RSG"), GLOB_STATS | USE_SCALESHIFT | FUSE_BN_RELU);
    return OK;
}

static int check_desc() {
    char str[max_desc_len];
    desc_t d{0};
    d.mb = 3; d.ic = 4; d.ih = 5; d.iw = 6; d.eps = 7.; d.name = "test";

    desc2str(&d, str);
    CHECK_CASE_STR_EQ(str, "mb3ic4ih5iw6eps7ntest");

    d.mb = 2;
    d.iw = d.ih;
    d.eps = 1.f / 16;
    desc2str(&d, str);
    CHECK_CASE_STR_EQ(str, "ic4ih5ntest");

#   define CHECK_D(_mb, _ic, _ih, _iw, _eps, _name) \
    CHECK_EQ(d.mb, _mb); CHECK_EQ(d.ic, _ic); CHECK_EQ(d.ih, _ih); \
    CHECK_EQ(d.iw, _iw); CHECK_EQ(d.eps, _eps); \
    CHECK_CASE_STR_EQ(d.name, _name)
    CHECK_EQ(str2desc(&d, "mb1ic2ih3iw4eps5ntest2"), OK);
    CHECK_D(1, 2, 3, 4, 5.f, "test2");
    CHECK_EQ(str2desc(&d, "ic8ih9ntest3"), OK);
    CHECK_D(2, 8, 9, 9, 1.f / 16, "test3");
    CHECK_EQ(str2desc(&d, "ic8iw9ntest3"), OK);
    CHECK_D(2, 8, 9, 9, 1.f / 16, "test3");
#   undef CHECK_D
    return OK;
}

void bnorm() {
    RUN(check_flags());
    RUN(check_desc());
}

}
