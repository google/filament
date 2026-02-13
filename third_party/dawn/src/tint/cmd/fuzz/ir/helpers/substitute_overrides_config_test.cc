// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/cmd/fuzz/ir/helpers/substitute_overrides_config.h"

#include "gtest/gtest.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"

namespace tint::fuzz::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

using SubstituteOverridesConfigTest = testing::Test;

TEST_F(SubstituteOverridesConfigTest, Bool) {
    core::ir::Module mod;
    core::ir::Builder b(mod);
    core::type::Manager& ty = mod.Types();

    b.Append(mod.root_block, [&] {
        auto* oa = b.Override("a", ty.bool_());
        oa->SetOverrideId(OverrideId{1});

        auto* ob = b.Override("b", true);
        ob->SetOverrideId(OverrideId{20});

        auto* oc = b.Override("c", ob);
        oc->SetOverrideId(OverrideId{300});

        auto* oe = b.Override("e", b.Constant(true));
        oe->SetOverrideId(OverrideId{500});
    });

    auto cfg = SubstituteOverridesConfig(mod);
    ASSERT_EQ(1u, cfg.map.size());

    ASSERT_TRUE(cfg.map.find(OverrideId{1}) != cfg.map.end());
    EXPECT_FLOAT_EQ(cfg.map[OverrideId{1}], 0.f);
}

TEST_F(SubstituteOverridesConfigTest, U32) {
    core::ir::Module mod;
    core::ir::Builder b(mod);
    core::type::Manager& ty = mod.Types();

    b.Append(mod.root_block, [&] {
        auto* oa = b.Override("a", ty.u32());
        oa->SetOverrideId(OverrideId{1});

        auto* ob = b.Override("b", 42_u);
        ob->SetOverrideId(OverrideId{20});

        auto* od = b.Override("d", b.Add(ty.u32(), 42_u, 10_u));
        od->SetOverrideId(OverrideId{400});

        auto* oe = b.Override("e", b.Constant(100_u));
        oe->SetOverrideId(OverrideId{500});
    });

    auto cfg = SubstituteOverridesConfig(mod);
    ASSERT_EQ(1u, cfg.map.size());

    ASSERT_TRUE(cfg.map.find(OverrideId{1}) != cfg.map.end());
    EXPECT_FLOAT_EQ(0.f, cfg.map[OverrideId{1}]);
}

TEST_F(SubstituteOverridesConfigTest, I32) {
    core::ir::Module mod;
    core::ir::Builder b(mod);
    core::type::Manager& ty = mod.Types();

    b.Append(mod.root_block, [&] {
        auto* oa = b.Override("a", ty.i32());
        oa->SetOverrideId(OverrideId{1});

        auto* ob = b.Override("b", -42_i);
        ob->SetOverrideId(OverrideId{20});

        auto* oc = b.Override("b", 42_i);
        oc->SetOverrideId(OverrideId{300});

        auto* od = b.Override("d", b.Add(ty.i32(), 45_i, 10_i));
        od->SetOverrideId(OverrideId{400});

        auto* oe = b.Override("e", b.Constant(100_i));
        oe->SetOverrideId(OverrideId{500});
    });

    auto cfg = SubstituteOverridesConfig(mod);
    ASSERT_EQ(1u, cfg.map.size());

    ASSERT_TRUE(cfg.map.find(OverrideId{1}) != cfg.map.end());
    EXPECT_FLOAT_EQ(0.f, cfg.map[OverrideId{1}]);
}

TEST_F(SubstituteOverridesConfigTest, F32) {
    core::ir::Module mod;
    core::ir::Builder b(mod);
    core::type::Manager& ty = mod.Types();

    b.Append(mod.root_block, [&] {
        auto* oa = b.Override("a", ty.f32());
        oa->SetOverrideId(OverrideId{1});

        auto* ob = b.Override("b", -42_f);
        ob->SetOverrideId(OverrideId{20});

        auto* oc = b.Override("b", 42_f);
        oc->SetOverrideId(OverrideId{300});

        auto* od = b.Override("d", b.Add(ty.f32(), 45_f, 10_f));
        od->SetOverrideId(OverrideId{400});

        auto* oe = b.Override("e", b.Constant(100_f));
        oe->SetOverrideId(OverrideId{500});
    });

    auto cfg = SubstituteOverridesConfig(mod);
    ASSERT_EQ(1u, cfg.map.size());

    ASSERT_TRUE(cfg.map.find(OverrideId{1}) != cfg.map.end());
    EXPECT_FLOAT_EQ(0.f, cfg.map[OverrideId{1}]);
}

TEST_F(SubstituteOverridesConfigTest, F16) {
    core::ir::Module mod;
    core::ir::Builder b(mod);
    core::type::Manager& ty = mod.Types();

    b.Append(mod.root_block, [&] {
        auto* oa = b.Override("a", ty.f16());
        oa->SetOverrideId(OverrideId{1});

        auto* ob = b.Override("b", -42_h);
        ob->SetOverrideId(OverrideId{20});

        auto* oc = b.Override("b", 42_h);
        oc->SetOverrideId(OverrideId{300});

        auto* od = b.Override("d", b.Add(ty.f32(), 45_h, 10_h));
        od->SetOverrideId(OverrideId{400});

        auto* oe = b.Override("e", b.Constant(100_h));
        oe->SetOverrideId(OverrideId{500});
    });

    auto cfg = SubstituteOverridesConfig(mod);
    ASSERT_EQ(1u, cfg.map.size());

    ASSERT_TRUE(cfg.map.find(OverrideId{1}) != cfg.map.end());
    EXPECT_FLOAT_EQ(0.f, cfg.map[OverrideId{1}]);
}

}  // namespace
}  // namespace tint::fuzz::ir
