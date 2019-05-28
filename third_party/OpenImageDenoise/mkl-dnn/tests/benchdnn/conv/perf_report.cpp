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
#include <string.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

#include "mkldnn.h"
#include "mkldnn_memory.hpp"

#include "conv/conv_common.hpp"

namespace conv {

#if 0
**benchdnn** supports custom performance report. Template is passed via
command line and consists of terminal and nonterminal symbols. Nonterminal
symbols are printed as is. Description of terminal symbols is given below.
There is also a notion of modifiers (marked as @) that change meaning of
terminal symbols, e.g. sign '-' means minimum of (in terms of time). See
table of modifiers below.

> **caution:** threads have to be pinned in order to get consistent frequency

| abbreviation  | description
|:------------  |:-----------
| %d            | problem descriptor
| %D            | expanded problem descriptor (conv parameters in csv format)
| %n            | problem name
| %z            | direction
| %@F           | effective cpu frequency computed as clocks[@] / time[@]
| %O            | number of ops required (padding is not taken into account)
| %@t           | time in ms
| %@c           | time in clocks
| %@p           | ops per second

| modifier  | description
|:--------  |:-----------
|           | default
| -         | min (time) -- default
| 0         | avg (time)
| +         | max (time)
|           |
| K         | Kilo (1e3)
| M         | Mega (1e6)
| G         | Giga (1e9)

The definition of expanded problem descriptor is:
`g,mb,ic,ih,iw,oc,oh,ow,kh,kw,sh,sw,ph,pw`.
#endif

void perf_report(const prb_t *p, const res_t *r, const char *pstr) {
    const auto &t = r->timer;
    const int max_len = 400;
    int rem_len = max_len - 1;
    char buffer[max_len], *buf = buffer;

#   define DPRINT(...) do { \
        int l = snprintf(buf, rem_len, __VA_ARGS__); \
        buf += l; rem_len -= l; \
    } while(0)

    auto modifier2mode = [](char c) {
        if (c == '-') return benchdnn_timer_t::min;
        if (c == '0') return benchdnn_timer_t::avg;
        if (c == '+') return benchdnn_timer_t::max;
        return benchdnn_timer_t::min;
    };

    auto modifier2unit = [](char c) {
        if (c == 'K') return 1e3;
        if (c == 'M') return 1e6;
        if (c == 'G') return 1e9;
        return 1e0;
    };

    const char *pt = perf_template;
    char c;

    while ((c = *pt++) != '\0') {
        if (c != '%') { *buf++ = c; rem_len--; continue; }

        c = *pt++;

        benchdnn_timer_t::mode_t mode = benchdnn_timer_t::min;
        double unit = 1e0;

        if (c == '-' || c == '0' || c == '+') {
            mode = modifier2mode(c);
            c = *pt++;
        }

        if (c == 'K' || c == 'M' || c == 'G') {
            unit = modifier2unit(c);
            c = *pt++;
        }

        if (c == 'd') DPRINT("%s", pstr);
        else if (c == 'D')
            DPRINT("" IFMT "," IFMT ","
                    IFMT "," IFMT "," IFMT ","
                    IFMT "," IFMT "," IFMT ","
                    IFMT "," IFMT "," IFMT "," IFMT "," IFMT "," IFMT "",
                    p->g, p->mb,
                    p->ic, p->ih, p->iw,
                    p->oc, p->oh, p->ow,
                    p->kh, p->kw, p->sh, p->sw, p->ph, p->pw);
        else if (c == 'n')
            DPRINT("%s", p->name);
        else if (c == 'z')
            DPRINT("%s", dir2str(p->dir));
        else if (c == 'O')
            DPRINT("%g", p->ops / unit);
        else if (c == 'F')
            DPRINT("%g", t.ticks(mode) / t.ms(mode) / unit * 1e3);
        else if (c == 't')
            DPRINT("%g", t.ms(mode) / unit);
        else if (c == 'c')
            DPRINT("%g", t.ticks(mode) / unit);
        else if (c == 'p')
            DPRINT("%g", p->ops / t.ms(mode) / unit * 1e3);
        else
            []() { SAFE(FAIL, CRIT); return 0; }();
    }

    *buf = '\0';
    assert(rem_len >= 0);

#   undef DPRINT
    print(0, "%s\n", buffer);
}

}
