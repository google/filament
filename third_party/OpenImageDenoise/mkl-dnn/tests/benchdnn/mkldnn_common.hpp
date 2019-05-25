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

#ifndef _MKLDNN_COMMON_HPP
#define _MKLDNN_COMMON_HPP

#include <stddef.h>
#include <stdint.h>
#include <vector>

#include "mkldnn.h"

#include "common.hpp"
#include "dnn_types.hpp"
#include "mkldnn_debug.hpp"

#define DNN_SAFE(f, s) do { \
    mkldnn_status_t status = f; \
    if (status != mkldnn_success) { \
        if (s == CRIT || s == WARN) { \
            print(0, "error [%s:%d]: '%s' -> %s(%d)\n", \
                    __PRETTY_FUNCTION__, __LINE__, \
                    #f, status2str(status), (int)status); \
            fflush(0); \
            if (s == CRIT) exit(2); \
        } \
        return FAIL; \
    } \
} while(0)

#define DNN_SAFE_V(f) do { \
    mkldnn_status_t status = f; \
    if (status != mkldnn_success) { \
        print(0, "error [%s:%d]: '%s' -> %s(%d)\n", \
                __PRETTY_FUNCTION__, __LINE__, \
                STRINGIFY(f), status2str(status), (int)status); \
        fflush(0); \
        exit(2); \
    } \
} while(0)

/* aux */
template <mkldnn_data_type_t> struct prec_traits;
template <> struct prec_traits<mkldnn_f32> { typedef float type; };
template <> struct prec_traits<mkldnn_s32> { typedef int32_t type; };
template <> struct prec_traits<mkldnn_s8> { typedef int8_t type; };
template <> struct prec_traits<mkldnn_u8> { typedef uint8_t type; };

inline size_t sizeof_dt(mkldnn_data_type_t dt) {
    switch (dt) {
#   define CASE(dt) case dt: return sizeof(typename prec_traits<dt>::type)
    CASE(mkldnn_f32);
    CASE(mkldnn_s32);
    CASE(mkldnn_s8);
    CASE(mkldnn_u8);
#   undef CASE
    default: assert(!"bad data_type");
    }
    return 0;
}

/* simplification */
extern mkldnn_engine_t engine;
extern mkldnn_stream_t stream;

inline int init() {
    DNN_SAFE(mkldnn_engine_create(&engine, mkldnn_cpu, 0), CRIT);
    DNN_SAFE(mkldnn_stream_create(&stream, engine, mkldnn_stream_default_flags), CRIT);
    return OK;
}

inline int finalize() {
    DNN_SAFE(mkldnn_stream_destroy(stream), CRIT);
    DNN_SAFE(mkldnn_engine_destroy(engine), CRIT);
    return OK;
}

inline const char *query_impl_info(const_mkldnn_primitive_desc_t pd) {
    const char *str;
    mkldnn_primitive_desc_query(pd, mkldnn_query_impl_info_str, 0, &str);
    return str;
}

struct args_t {
    args_t &set(int arg, mkldnn_memory_t memory) {
        mkldnn_exec_arg_t a = {arg, memory};
        args_.push_back(a);
        return *this;
    }
    void clear() { args_.clear(); }

    int size() const { return (int)args_.size(); }
    const mkldnn_exec_arg_t *args() const { return args_.data(); }
    operator const mkldnn_exec_arg_t *() const { return args(); }
private:
    std::vector<mkldnn_exec_arg_t> args_;
};

#endif
