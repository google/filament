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
#include <assert.h>
#include "shuffle/shuffle.hpp"

namespace shuffle {

#define DPRINT(...) do { \
    int l = snprintf(buffer, rem_len, __VA_ARGS__); \
    buffer += l; rem_len -= l; \
} while(0)

dims_t str2dims(const char *str) {
    dims_t dims;
    do {
        int len;
        int64_t dim;
        int scan = sscanf(str, IFMT "%n", &dim, &len);
        SAFE_V(scan == 1 ? OK : FAIL);
        dims.push_back(dim);
        str += len;
        SAFE_V(*str == 'x' || *str == '\0' ? OK : FAIL);
    } while (*str++ != '\0');
    return dims;
}

void dims2str(const dims_t &dims, char *buffer) {
    int rem_len = max_dims_len;
    for (size_t d = 0; d < dims.size() - 1; ++d)
        DPRINT(IFMT "x", dims[d]);
    DPRINT(IFMT, dims[dims.size() - 1]);
}

void prb2str(const prb_t *p, char *buffer, bool canonical) {
    char dims_buf[max_dims_len] = {0};
    dims2str(p->dims, dims_buf);

    char dir_str[32] = {0};
    char dt_str[16] = {0};
    char tag_str[32] = {0};
    char axis_str[16] = {0};
    char group_str[16] = {0};

    snprintf(dir_str, sizeof(dir_str), "--dir=%s ", dir2str(p->dir));
    snprintf(dt_str, sizeof(dt_str), "--dt=%s ", dt2str(p->dt));
    snprintf(tag_str, sizeof(tag_str), "--tag=%s ", tag2str(p->tag));
    snprintf(axis_str, sizeof(axis_str), "--axis=%d ", p->a);
    snprintf(group_str, sizeof(group_str), "--group=" IFMT " ", p->g);
    snprintf(buffer, max_prb_len, "%s%s%s%s%s%s", dir_str, dt_str, tag_str,
           axis_str, group_str, dims_buf);
}

}
