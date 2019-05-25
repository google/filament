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

#include <memory>
#include <cstring>

#include "gtest/gtest.h"
#include "mkldnn_test_common.hpp"

#include "mkldnn.hpp"

namespace mkldnn {

typedef float data_t;

struct memory_test_params {
    memory::dims dims;
    memory::format_tag fmt_tag;
    bool expect_to_fail;
    mkldnn_status_t expected_status;
};

class memory_test: public ::testing::TestWithParam<memory_test_params> {
protected:
    virtual void SetUp() {
        memory_test_params p
            = ::testing::TestWithParam<decltype(p)>::GetParam();
        catch_expected_failures([=](){Test();}, p.expect_to_fail,
                    p.expected_status);
    }
    void Test() {
        memory_test_params p
            = ::testing::TestWithParam<decltype(p)>::GetParam();

        auto e = engine(engine::kind::cpu, 0);

        mkldnn::memory mem0({p.dims, memory::data_type::f32, p.fmt_tag}, e);
        data_t *mem0_ptr = (data_t *)mem0.get_data_handle();
        memory::dim phys_size = mem0.get_desc().get_size() / sizeof(data_t);
        fill_data<data_t>(phys_size, mem0_ptr);

        std::vector<data_t> mem1_vec(phys_size);
        mem1_vec.assign(mem0_ptr, mem0_ptr + phys_size);

        mkldnn::memory mem1({p.dims, memory::data_type::f32, p.fmt_tag}, e,
                &mem1_vec[0]);

        check_zero_tail<data_t>(0, mem1);
        check_zero_tail<data_t>(1, mem0);

        for (memory::dim i = 0; i < phys_size; ++i)
            EXPECT_NEAR(mem0_ptr[i], mem1_vec[i], 1e-7) << i;
    }
};

using fmt = memory::format_tag;

TEST_P(memory_test, TestsMemory) { }
INSTANTIATE_TEST_SUITE_P(TestMemory, memory_test,
        ::testing::Values(
            memory_test_params{{2, 0, 1, 1}, fmt::nChw16c},
            memory_test_params{{2, 15, 3, 2}, fmt::nChw16c},
            memory_test_params{{2, 15, 3, 2, 4}, fmt::nCdhw8c},
            memory_test_params{{2, 9, 3, 2}, fmt::OIhw8o8i},
            memory_test_params{{2, 9, 3, 2}, fmt::OIhw8i16o2i},
            memory_test_params{{2, 9, 3, 2}, fmt::OIhw8o16i2o},
            memory_test_params{{2, 9, 3, 2}, fmt::OIhw16o16i},
            memory_test_params{{2, 9, 3, 2}, fmt::OIhw16i16o},
            memory_test_params{{2, 9, 3, 2}, fmt::OIhw4i16o4i},
            memory_test_params{{2, 9, 4, 3, 2}, fmt::gOihw16o},
            memory_test_params{{1, 2, 9, 3, 2}, fmt::gOIhw8o8i},
            memory_test_params{{1, 2, 9, 3, 2}, fmt::gOIhw4o4i},
            memory_test_params{{1, 2, 9, 3, 2}, fmt::gOIhw8i8o},
            memory_test_params{{2, 17, 9, 3, 2}, fmt::gOIhw4i16o4i},
            memory_test_params{{2, 17, 9, 3, 2}, fmt::gOIhw2i8o4i},
            memory_test_params{{15, 16, 16, 3, 3}, fmt::Goihw8g}
            )
        );
}
