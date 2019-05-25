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

#include "mkldnn_test_common.hpp"
#include "gtest/gtest.h"

#include "mkldnn_types.h"
#include "mkldnn.h"

namespace mkldnn {

const mkldnn_status_t ok = mkldnn_success;

class pd_iter_test: public ::testing::Test {
protected:
    mkldnn_engine_t engine;
    virtual void SetUp() {
        EXPECT_EQ(mkldnn_engine_create(&engine, mkldnn_cpu, 0), ok);
    }
    virtual void TearDown() {
        mkldnn_engine_destroy(engine);
    }
};

TEST_F(pd_iter_test, TestReLUImpls) {
    mkldnn_memory_desc_t dense_md;
    mkldnn_dims_t dims = {4, 16, 16, 16};
    EXPECT_EQ(mkldnn_memory_desc_init_by_tag(&dense_md, 4, dims, mkldnn_f32,
                mkldnn_nchw), ok);

    mkldnn_eltwise_desc_t ed;
    EXPECT_EQ(mkldnn_eltwise_forward_desc_init(&ed, mkldnn_forward_inference,
                mkldnn_eltwise_relu, &dense_md, 0., 0.), ok);

    mkldnn_primitive_desc_iterator_t it;
    mkldnn_status_t rc;

    EXPECT_EQ(rc = mkldnn_primitive_desc_iterator_create(&it, &ed, nullptr,
                engine, nullptr), ok); /* there should be at least one impl */

    mkldnn_primitive_desc_t pd;
    EXPECT_NE(pd = mkldnn_primitive_desc_iterator_fetch(it), nullptr);
    mkldnn_primitive_desc_destroy(pd);

    while ((rc = mkldnn_primitive_desc_iterator_next(it)) == ok) {
        EXPECT_NE(pd = mkldnn_primitive_desc_iterator_fetch(it), nullptr);
        mkldnn_primitive_desc_destroy(pd);
    }

    EXPECT_EQ(rc, mkldnn_iterator_ends);
    mkldnn_primitive_desc_iterator_destroy(it);
}

TEST(pd_next_impl, TestEltwiseImpl) {
    auto eng = engine(engine::kind::cpu, 0);
    memory::desc md({8, 32, 4, 4}, memory::data_type::f32, memory::format_tag::nChw8c);

    eltwise_forward::desc ed(prop_kind::forward_training,
            algorithm::eltwise_relu, md, 0, 0);
    eltwise_forward::primitive_desc epd(ed, eng);

    std::string impl0(epd.impl_info_str());
    eltwise_forward e0(epd);

    while (epd.next_impl()) {
        std::string impl1(epd.impl_info_str());
        eltwise_forward e1(epd);
        EXPECT_NE(impl0, impl1);
        impl0 = impl1;
    }
}

}
