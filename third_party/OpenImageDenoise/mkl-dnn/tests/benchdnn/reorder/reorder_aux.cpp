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

#include "mkldnn_debug.hpp"
#include "reorder/reorder.hpp"

#define DPRINT(...) do { \
    int l = snprintf(buffer, rem_len, __VA_ARGS__); \
    buffer += l; rem_len -= l; \
} while(0)

namespace reorder {

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

void prb2str(const prb_t *p, const res_t *res, char *buffer) {
    char dims_buf[max_dims_len] = {0};
    dims2str(p->reorder.dims, dims_buf);

    char attr_buf[max_attr_len] = {0};
    bool is_attr_def = p->attr.is_def();
    if (!is_attr_def) {
        int len = snprintf(attr_buf, max_attr_len, "--attr=\"");
        SAFE_V(len >= 0 ? OK : FAIL);
        attr2str(&p->attr, attr_buf + len);
        len = (int)strnlen(attr_buf, max_attr_len);
        snprintf(attr_buf + len, max_attr_len - len, "\" ");
    }

    int rem_len = max_prb_len;
    DPRINT("--idt=%s --odt=%s --itag=%s --otag=%s %s%s",
            dt2str(cfg2dt(p->conf_in)), dt2str(cfg2dt(p->conf_out)),
            tag2str(p->reorder.tag_in), tag2str(p->reorder.tag_out),
            attr_buf, dims_buf);
}

}
