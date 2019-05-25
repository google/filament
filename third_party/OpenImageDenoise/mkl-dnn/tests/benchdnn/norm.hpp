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

#ifndef _NORM_HPP
#define _NORM_HPP

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

#include "common.hpp"

struct norm_t {
    typedef float real_t;
    typedef double acc_t;

    /* strictly speaking L0 is not a norm... it stands for the biggest
     * absolute element-wise difference and is used in diff_norm_t only */
    enum { L0, L1, L2, LINF, L8 = LINF, L_LAST };

    norm_t(): num_(0) {
        for (int i = 0; i < L_LAST; ++i)
            norm_[i] = 0;
    }

    void update(real_t v) {
        norm_[L1] += ABS(v);
        norm_[L2] += v*v;
        norm_[L8] = MAX2(norm_[L8], ABS(v));
        num_++;
    }

    void done() { norm_[L2] = sqrt(norm_[L2]); }

    real_t operator[](int type) const { return norm_[type]; }

    acc_t norm_[L_LAST];
    int num_;
};

struct diff_norm_t {
    using real_t = norm_t::real_t;

    void update(real_t a, real_t b) {
        real_t diff = a - b;
        a_.update(a);
        b_.update(b);
        diff_.update(diff);
        diff_.norm_[norm_t::L0] = MAX2(diff_.norm_[norm_t::L0],
                ABS(diff) / (ABS(a) > FLT_MIN ? ABS(a) : 1.));
    }
    void done() { a_.done(); b_.done(); diff_.done(); }

    real_t rel_diff(int type) const {
        if (type == norm_t::L0) return diff_.norm_[type];
        return diff_.norm_[type] / a_.norm_[type];
    }

    norm_t a_, b_, diff_;
};

#endif
