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

#ifndef _MKLDNN_MEMORY_HPP
#define _MKLDNN_MEMORY_HPP

#include "mkldnn_common.hpp"

struct dnn_mem_t {
    dnn_mem_t(): active_(false) {}

    dnn_mem_t(const mkldnn_memory_desc_t &md, void *data = NULL)
        : active_(initialize(md, data) == OK) {}

    dnn_mem_t(int ndims, const mkldnn_dims_t dims, mkldnn_data_type_t dt,
            mkldnn_format_tag_t tag, void *data = NULL)
        : active_(initialize(ndims, dims, dt, tag, data) == OK) {}

    dnn_mem_t(int ndims, const mkldnn_dims_t dims, mkldnn_data_type_t dt,
            const mkldnn_dims_t strides, void *data = NULL)
        : active_(initialize(ndims, dims, dt, strides, data) == OK) {}

    dnn_mem_t(const mkldnn_memory_desc_t &md, mkldnn_data_type_t dt,
            mkldnn_format_tag_t tag = mkldnn_format_tag_undef,
            void *data = NULL)
        : active_(initialize(md, dt, tag, data) == OK) {}

    dnn_mem_t(const dnn_mem_t &rhs, mkldnn_data_type_t dt,
            mkldnn_format_tag_t tag = mkldnn_format_tag_undef,
            void *data = NULL): dnn_mem_t(rhs.md_, dt, tag, data)
    { if (active_) reorder(rhs); }

    /* FIXME: ugly RT assert... need better mkldnn memory handling */
    dnn_mem_t &operator=(const dnn_mem_t &rhs)
    { []() { SAFE(FAIL, CRIT); return 0; }(); return *this; }
    dnn_mem_t(const dnn_mem_t &rhs)
    { []() { SAFE(FAIL, CRIT); return 0; }(); }

    ~dnn_mem_t() { cleanup(); }

    int reorder(const dnn_mem_t &rhs) { return reorder(rhs, NULL); }
    int reorder(const dnn_mem_t &rhs, const mkldnn_primitive_attr_t &attr) {
        if (this == &rhs) return OK;

        mkldnn_primitive_desc_t rpd;
        DNN_SAFE(mkldnn_reorder_primitive_desc_create(&rpd,
                    engine, &rhs.md_, engine, &md_, attr), WARN);

        mkldnn_primitive_t r;
        DNN_SAFE(mkldnn_primitive_create(&r, rpd), WARN);
        DNN_SAFE(mkldnn_primitive_desc_destroy(rpd), CRIT);

        mkldnn_exec_arg_t args[] = {
            {MKLDNN_ARG_FROM, rhs.m_},
            {MKLDNN_ARG_TO, m_},
        };
        DNN_SAFE(mkldnn_primitive_execute(r, stream, 2, args), WARN);
        DNN_SAFE(mkldnn_primitive_destroy(r), CRIT);

        return OK;
    }

    int64_t N() { return md_.dims[0]; }
    int64_t with_G() { return md_.ndims == 5; }
    int64_t G() { return md_.ndims == 5 ? md_.dims[0] : 1; }

    int64_t C() { return md_.ndims == 1 ? md_.dims[0] : md_.dims[1]; }
    int64_t OC() { return md_.dims[with_G() + 0]; }
    int64_t IC() { return md_.dims[with_G() + 1]; }
    int64_t H() { return md_.dims[with_G() + 2]; } // works for both IH and KH
    int64_t W() { return md_.dims[with_G() + 3]; } // works for both IW and KW

    size_t size() const { return mkldnn_memory_desc_get_size(&md_); }

    int64_t nelems(bool with_padded_dims = false) const {
        auto dims = with_padded_dims
            ? md_.padded_dims
            : md_.dims;
        int64_t n = 1;
        for (int i = 0; i < md_.ndims; ++i)
            n *= dims[i];
        return n;
    }

    mkldnn_data_type_t dt() const { return md_.data_type; }
    size_t sizeof_dt() const { return ::sizeof_dt(dt()); }

    template <typename T>
    explicit operator T*() const { return static_cast<T*>(data_); }

    float get_elem(int64_t idx) const {
        float elem = 0.0;
        switch (dt()) {
            case mkldnn_s8: elem = static_cast<int8_t *>(data_)[idx]; break;
            case mkldnn_u8: elem = static_cast<uint8_t *>(data_)[idx]; break;
            case mkldnn_s32: elem = static_cast<int32_t *>(data_)[idx]; break;
            case mkldnn_f32: elem = static_cast<float *>(data_)[idx]; break;
            default: assert(!"bad data type");
        }
        return elem;
    }

    void set_elem(int64_t idx, float value) {
        switch (dt()) {
            case mkldnn_s8: ((int8_t *)data_)[idx] = value; break;
            case mkldnn_u8: ((uint8_t *)data_)[idx] = value; break;
            case mkldnn_s32: ((int32_t *)data_)[idx] = value; break;
            case mkldnn_f32: ((float *)data_)[idx] = value; break;
            default: assert(!"bad data type");
        }
    }

    int64_t get_scale_idx(int64_t data_idx, int scale_mask) const {
        const int ndims = md_.ndims;
        const auto &dims = md_.dims;
        int64_t stride = 1;
        int64_t offset = 0;

        if (scale_mask != 0) {
            for (int i = 0; i < ndims; ++i) {
                int d = md_.ndims - 1 - i;
                auto pos = data_idx % dims[d];
                data_idx /= dims[d];
                if (scale_mask & (1 << d)) {
                    offset += pos * stride;
                    stride *= dims[d];
                }
            }
        }

        return offset;
    }

    /* fields */

    mkldnn_memory_desc_t md_;
    mkldnn_memory_t m_;
    void *data_;
    bool is_data_owner_, active_;

private:
    int initialize(const mkldnn_memory_desc_t &md, mkldnn_data_type_t dt,
            mkldnn_format_tag_t tag, void *data) {
        if (tag == mkldnn_format_tag_undef) {
            md_ = md;
            md_.data_type = dt;
        } else {
            DNN_SAFE(mkldnn_memory_desc_init_by_tag(
                        &md_, md.ndims, md.dims, dt, tag), CRIT);
        }
        DNN_SAFE(mkldnn_memory_create(&m_, &md_, engine, NULL), CRIT);
        is_data_owner_ = data == NULL;
        if (data == NULL) {
            const size_t alignment = 1024 * 1024 * 2;
            size_t sz = mkldnn_memory_desc_get_size(&md_);
            data_ = zmalloc(sz, alignment);
            DNN_SAFE(data_ == NULL ? mkldnn_out_of_memory : mkldnn_success,
                    WARN);
        } else {
            data_ = data;
        }
        DNN_SAFE(mkldnn_memory_set_data_handle(m_, data_), CRIT);

        return OK;
    }

    int initialize(const mkldnn_memory_desc_t &md, void *data) {
        return initialize(md, md.data_type, mkldnn_format_tag_undef, data);
    }

    int initialize(int ndims, const mkldnn_dims_t dims, mkldnn_data_type_t dt,
                    mkldnn_format_tag_t tag, void* data) {
        mkldnn_memory_desc_t xmd;
        DNN_SAFE(mkldnn_memory_desc_init_by_tag(&xmd, ndims, dims, dt, tag), CRIT);
        SAFE(initialize(xmd, data), CRIT);
        return OK;
    }

    int initialize(int ndims, const mkldnn_dims_t dims, mkldnn_data_type_t dt,
            const mkldnn_dims_t strides, void *data) {
        mkldnn_memory_desc_t xmd;
        DNN_SAFE(mkldnn_memory_desc_init_by_strides(
                    &xmd, ndims, dims, dt, strides), CRIT);
        SAFE(initialize(xmd, data), CRIT);
        return OK;
    }

    int cleanup() {
        if (!active_) return OK;
        DNN_SAFE(mkldnn_memory_destroy(m_), CRIT);
        if (is_data_owner_) zfree(data_);
        return OK;
    }
};

#endif
