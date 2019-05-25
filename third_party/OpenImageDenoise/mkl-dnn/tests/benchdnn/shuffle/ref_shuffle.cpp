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

#include "shuffle/shuffle.hpp"
#include "src/common/mkldnn_thread.hpp"

namespace shuffle {

void compute_shuffle(const prb_t *p, const dnn_mem_t &src, dnn_mem_t &dst)
{
    const int ndims = (int)p->dims.size();
    const int axis = p->a;
    const int64_t group_size = p->g;
    const int64_t axis_size = p->dims[axis];
    int64_t inner_size = 1, outer_size = 1;

    auto transpose = [=] (int64_t a) {
        int64_t R, C;
        if (p->dir == FWD_D) {
            R = group_size;
            C = axis_size / group_size;
        } else {
            R = axis_size / group_size;
            C = group_size;
        }
        int64_t col = a / R;
        int64_t row = a % R;
        return C * row + col;
    };

    for (int i = 0; i < axis ; ++i)
        outer_size *= (size_t)p->dims[i];
    for (int i = axis + 1; i < ndims; ++i)
        inner_size *= (size_t)p->dims[i];
    const size_t dim = axis_size * inner_size;

    mkldnn::impl::parallel_nd(outer_size, axis_size, inner_size,
           [&](int64_t ou, int64_t a, int64_t in) {
        auto src_off = ou * dim + a * inner_size + in;
        auto dst_off = ou * dim + transpose(a) * inner_size + in;
        dst.set_elem(dst_off, src.get_elem(src_off));
    });
}

}
