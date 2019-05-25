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

#include <stdlib.h>
#include <assert.h>
#include "bnorm/bnorm.hpp"

namespace bnorm {

check_alg_t str2check_alg(const char *str) {
    if (!strcasecmp("alg_0", str)) return ALG_0;
    if (!strcasecmp("alg_1", str)) return ALG_1;
    return ALG_AUTO;
}

const char* check_alg2str(check_alg_t alg) {
    switch (alg) {
    case ALG_0: return "alg_0";
    case ALG_1: return "alg_1";
    case ALG_AUTO: return "alg_auto";
    }
    return "alg_auto";
}

flags_t str2flags(const char *str) {
    flags_t flags = (flags_t)0;
    while (str && *str) {
        if (*str == 'G') flags |= GLOB_STATS;
        if (*str == 'S') flags |= USE_SCALESHIFT;
        if (*str == 'R') flags |= FUSE_BN_RELU;
        str++;
    }
    return flags;
}

const char *flags2str(flags_t flags) {
    if (flags & GLOB_STATS) {
        if (flags & USE_SCALESHIFT)
            return flags & FUSE_BN_RELU ? "GSR" : "GS";
        return flags & FUSE_BN_RELU ? "GR" : "G";
    }

    if (flags & USE_SCALESHIFT)
        return flags & FUSE_BN_RELU ? "SR" : "S";

    return flags & FUSE_BN_RELU ? "R" : "";
}

int str2desc(desc_t *desc, const char *str) {
    /* canonical form:
     * mbXicXihXiwXepsYnS
     *
     * where:
     *  X is number (integer)
     *  Y is real (float)
     *  S - string
     * note: symbol `_` is ignored
     *
     * implicit rules:
     *  eps = 1./16
     *  S = "wip"
     *  if iw is unset iw <-- ih
     *  if ih is unset ih <-- iw
     */

    desc_t d{0};
    d.mb = 2;
    d.eps = 1.f / 16;
    d.name = "\"wip\"";

    const char *s = str;
    assert(s);

    auto mstrtol = [](const char *nptr, char **endptr)
    { return strtol(nptr, endptr, 10); };

#   define CASE_NN(p, c, cvfunc) do { \
        if (!strncmp(p, s, strlen(p))) { \
            ok = 1; s += strlen(p); \
            char *end_s; d. c = cvfunc(s, &end_s); s += (end_s - s); \
            /* printf("@@@debug: %s: " IFMT "\n", p, d. c); */ \
        } \
    } while (0)
#   define CASE_N(c, cvfunc) CASE_NN(#c, c, cvfunc)
    while (*s) {
        int ok = 0;
        CASE_N(mb, mstrtol);
        CASE_N(ic, mstrtol);
        CASE_N(id, mstrtol);
        CASE_N(ih, mstrtol);
        CASE_N(iw, mstrtol);
        CASE_N(eps, strtof);
        if (*s == 'n') { d.name = s + 1; break; }
        if (*s == '_') ++s;
        if (!ok) return FAIL;
    }
#   undef CASE_NN
#   undef CASE_N

    if (d.id == 0) d.id = 1;
    if (d.ih == 0) d.ih = d.iw;
    if (d.iw == 0) d.iw = d.ih;
    if (d.ic == 0 || d.ih == 0 || d.iw == 0) return FAIL;

    *desc = d;

    return OK;
}

void desc2str(const desc_t *d, char *buffer, bool canonical) {
    int rem_len = max_desc_len;
#   define DPRINT(...) do { \
        int l = snprintf(buffer, rem_len, __VA_ARGS__); \
        buffer += l; rem_len -= l; \
    } while(0)

    if (canonical || d->mb != 2) DPRINT("mb" IFMT "", d->mb);
    DPRINT("ic" IFMT "", d->ic);
    if (d->id > 1) DPRINT("id" IFMT "", d->id);
    DPRINT("ih" IFMT "", d->ih);
    if (canonical || d->iw != d->ih || d->id > 1) DPRINT("iw" IFMT "", d->iw);
    if (canonical || d->eps != 1.f/16) DPRINT("eps%g", d->eps);
    DPRINT("n%s", d->name);

#   undef DPRINT
}

void prb2str(const prb_t *p, char *buffer, bool canonical) {
    char desc_buf[max_desc_len];
    char dir_str[32] = {0};
    char dt_str[16] = {0};
    char tag_str[32] = {0};
    char flags_str[16] = {0};
    char attr_buf[max_attr_len];
    char check_str[24] = {0};
    desc2str(p, desc_buf, canonical);
    snprintf(dir_str, sizeof(dir_str), "--dir=%s ", dir2str(p->dir));
    snprintf(dt_str, sizeof(dt_str), "--dt=%s ", dt2str(p->dt));
    snprintf(tag_str, sizeof(tag_str), "--tag=%s ", tag2str(p->tag));
    snprintf(flags_str, sizeof(flags_str), "--flags=%s ", flags2str(p->flags));
    bool is_attr_def = p->attr.is_def();
    if (!is_attr_def) {
        int len = snprintf(attr_buf, max_attr_len, "--attr=\"");
        SAFE_V(len >= 0 ? OK : FAIL);
        attr2str(&p->attr, attr_buf + len);
        len = (int)strnlen(attr_buf, max_attr_len);
        snprintf(attr_buf + len, max_attr_len - len, "\" ");
    }
    snprintf(check_str, sizeof(check_str), "--check-alg=%s ",
            check_alg2str(p->check_alg));
    snprintf(buffer, max_prb_len, "%s%s%s%s%s%s%s",
            p->check_alg == ALG_AUTO ? "" : check_str,
            p->dir == FWD_B ? "" : dir_str,
            p->dt == mkldnn_f32 ? "" : dt_str,
            is_bnorm_3d(p)
                ? p->tag == mkldnn_ncdhw ? "" : tag_str
                : p->tag == mkldnn_nchw ? "" : tag_str,
            p->flags == (flags_t)0 ? "" : flags_str,
            is_attr_def ? "" : attr_buf,
            desc_buf);
}

}
