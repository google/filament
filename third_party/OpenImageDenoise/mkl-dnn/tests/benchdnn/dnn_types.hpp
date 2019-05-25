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

#ifndef _DNN_TYPES_HPP
#define _DNN_TYPES_HPP

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "common.hpp"
#include "mkldnn_types.h"

enum dir_t {
    DIR_UNDEF = 0,
    FLAG_DAT = 1, FLAG_WEI = 2, FLAG_BIA = 4,
    FLAG_FWD = 32, FLAG_BWD = 64,
    FLAG_INF = 128,
    FWD_D = FLAG_FWD + FLAG_DAT,
    FWD_I = FLAG_FWD + FLAG_DAT + FLAG_INF,
    FWD_B = FLAG_FWD + FLAG_DAT + FLAG_BIA,
    BWD_D = FLAG_BWD + FLAG_DAT,
    BWD_DW = FLAG_BWD + FLAG_DAT + FLAG_WEI,
    BWD_W = FLAG_BWD + FLAG_WEI,
    BWD_WB = FLAG_BWD + FLAG_WEI + FLAG_BIA,
};
dir_t str2dir(const char *str);
const char *dir2str(dir_t dir);

typedef int data_kind_t;
enum {
    SRC = 0, WEI, BIA, DST, ACC,
    DATA, MEAN, VAR, SS,
    GWEI,
    DAT_TOTAL };
const char *data_kind2str(data_kind_t kind);

struct attr_t {
    struct scale_t {
        enum policy_t { NONE = 0, COMMON, PER_OC,
            // reorder section
            // XXX: order is important, from longer name to a shorter one
            // TODO: generalize, use numbers instead of predefined enum
            PER_DIM_01,
            PER_DIM_0, PER_DIM_1,
            // reorder section ends
            POLICY_TOTAL };
        static policy_t str2policy(const char *str);
        static const char *policy2str(policy_t policy);

        int str2scale(const char *str, const char **end_s);
        void scale2str(char *buffer, char **end_b) const;

        bool is_def() const { return this->policy == NONE; }

        policy_t policy = NONE;
        float scale = 1.;
    };

    struct post_ops_t {
        enum kind_t { SUM, RELU, TANH, ELU, SQUARE, ABS, SQRT, LINEAR, BRELU,
            SRELU, LOGISTIC, KIND_TOTAL };
        static kind_t str2kind(const char *str);
        static const char *kind2str(kind_t kind);
        static mkldnn_alg_kind_t kind2mkldnn_kind(kind_t kind);

        struct entry_t {
            kind_t kind;
            union {
                struct { float scale; } sum;
                struct {
                    mkldnn_alg_kind_t alg;
                    float scale, alpha, beta;
                } eltwise;
            };
        };

        post_ops_t(): len(0) {}

        int from_str(const char *str, const char **end_s);
        void to_str(char *buffer, char **end_b) const;

        bool is_def() const { return len == 0; }

        enum { capacity = 4 };
        int len;
        entry_t entry[4];
    };

    scale_t oscale;
    post_ops_t post_ops;

    bool is_def() const;
};

const size_t max_attr_len = 128;
int str2attr(attr_t *attr, const char *str);
void attr2str(const attr_t *attr, char *buffer);

mkldnn_format_tag_t get_default_tag(int ndims);
mkldnn_primitive_attr_t create_mkldnn_attr(const attr_t &attr,
        int64_t scale_cnt, int scale_mask, const float *scales);
inline mkldnn_primitive_attr_t create_mkldnn_attr(const attr_t &attr,
        int64_t scale_cnt, const float *scales)
{ return create_mkldnn_attr(attr, scale_cnt, -1, scales); }

#endif
