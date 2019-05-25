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

#include "mkldnn.h"

#include "mkldnn_common.hpp"
#include "mkldnn_debug.hpp"

#include "ip/ip.hpp"
namespace ip {

void prb_t::generate_oscales() {
    if (attr.oscale.policy != attr_t::scale_t::policy_t::PER_OC) return;

    scales = (float *)zmalloc(sizeof(float) * oc, 64);
    SAFE_V(scales != NULL ? OK : FAIL);

    const float K = 32;
    /* scale in [1/K .. K], with starting point at oscale.scale */
    float s[2] = {attr.oscale.scale, attr.oscale.scale/2};
    for (int64_t i = 0; i < oc; ++i) {
        int64_t si = i % 2; // 0 -> left, 1 -> right
        scales[i] = s[si];
        if (si == 0) {
            s[si] /= 2.;
            if (s[si] < 1./K) s[si] *= K*K; // turn around to become ~K
        } else {
            s[si] *= 2.;
            if (s[si] > K) s[si] /= K*K; // turn around to become ~K
        }
    }
}

int str2desc(desc_t *desc, const char *str) {
    desc_t d{0};

    /* canonical form:
     * mbXicXidXihXiwXSocXnS
     *
     * where: X is number, S - string
     * note: symbol `_` is ignored
     *
     * implicit rules:
     *  - default values:
     *      mb = 2, id = 1, S="wip", ih = 1
     *  - if W is undefined => W = H
     */

    d.mb = 2;
    d.name = "\"wip\"";

    const char *s = str;
    assert(s);

#   define CASE_NN(p, c) do { \
        if (!strncmp(p, s, strlen(p))) { \
            ok = 1; s += strlen(p); \
            char *end_s; d. c = strtol(s, &end_s, 10); s += (end_s - s); \
            /* printf("@@@debug: %s: %d\n", p, d. c); */ \
        } \
    } while (0)
#   define CASE_N(c) CASE_NN(#c, c)
    while (*s) {
        int ok = 0;
        CASE_N(mb);
        CASE_N(ic);
        CASE_N(ih);
        CASE_N(iw);
        CASE_N(id);
        CASE_N(oc);
        if (*s == 'n') { d.name = s + 1; break; }
        if (*s == '_') ++s;
        if (!ok) return FAIL;
    }
#   undef CASE_NN
#   undef CASE_N

    if (d.ic == 0 || d.oc == 0) return FAIL;

    if (d.id == 0) d.id = 1;
    if (d.ih == 0) d.ih = 1;
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
#   define is_1d(d) (d->ih == 1 && d->id == 1)

    if (canonical || d->mb != 2) DPRINT("mb" IFMT "", d->mb);
    DPRINT("oc" IFMT "", d->oc);
    DPRINT("ic" IFMT "", d->ic);
    if (d->id > 1) DPRINT("id" IFMT "", d->id);
    if (canonical || !is_1d(d)) DPRINT("ih" IFMT "", d->ih);
    if (canonical || d->iw != d->ih || d->id > 1 || is_1d(d))
        DPRINT("iw" IFMT "", d->iw);
    DPRINT("n%s", d->name);

#   undef is_1d
#   undef DPRINT
}


void prb2str(const prb_t *p, char *buffer, bool canonical) {
    char desc_buf[max_desc_len], attr_buf[max_attr_len];
    char dir_str[32] = {0}, cfg_str[32] = {0};
    desc2str(p, desc_buf, canonical);
    snprintf(dir_str, sizeof(dir_str), "--dir=%s ", dir2str(p->dir));
    snprintf(cfg_str, sizeof(cfg_str), "--cfg=%s ", cfg2str(p->cfg));
    bool is_attr_def = p->attr.is_def();
    if (!is_attr_def) {
        int len = snprintf(attr_buf, max_attr_len, "--attr=\"");
        SAFE_V(len >= 0 ? OK : FAIL);
        attr2str(&p->attr, attr_buf + len);
        len = (int)strnlen(attr_buf, max_attr_len);
        snprintf(attr_buf + len, max_attr_len - len, "\" ");
    }
    snprintf(buffer, max_prb_len, "%s%s%s%s",
            p->dir == FWD_B ? "" : dir_str,
            p->cfg == conf_f32 ? "" : cfg_str,
            is_attr_def ? "" : attr_buf,
            desc_buf);
}

}
