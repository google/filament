// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/utils/rtti/castable.h"

#include <memory>
#include <string>

#include "gtest/gtest.h"

namespace tint {
namespace core {
class CoreClass : public Castable<CoreClass> {};

struct Vehicle : public Castable<Vehicle> {};
struct Airplane : public Castable<Airplane, Vehicle> {};
struct Wheeled : public Castable<Wheeled, Vehicle> {};
struct Bicycle : public Castable<Bicycle, Vehicle> {};
struct Jetplane : public Castable<Jetplane, Airplane> {};
struct Jeep : public Castable<Jeep, Wheeled> {};
struct Tricycle : public Castable<Tricycle, Bicycle> {};
struct Pogo : public Castable<Pogo, Tricycle> {};
struct Paddleboard : public Castable<Paddleboard, Tricycle> {};

}  // namespace core
namespace hlsl {
class HLSLClass : public Castable<HLSLClass, core::CoreClass> {};
}  // namespace hlsl
namespace msl {
class MSLClass : public Castable<MSLClass, core::CoreClass> {};
}  // namespace msl
namespace glsl {
class GLSLClass : public Castable<GLSLClass, core::CoreClass> {};
}  // namespace glsl
namespace spirv {
class SPIRVClass : public Castable<SPIRVClass, core::CoreClass> {};
}  // namespace spirv

TINT_INSTANTIATE_TYPEINFO(tint::core::CoreClass);
TINT_INSTANTIATE_TYPEINFO(tint::hlsl::HLSLClass);
TINT_INSTANTIATE_TYPEINFO(tint::msl::MSLClass);
TINT_INSTANTIATE_TYPEINFO(tint::glsl::GLSLClass);
TINT_INSTANTIATE_TYPEINFO(tint::spirv::SPIRVClass);

namespace core {
namespace {

TEST(CastableBase, Dialect) {
    std::unique_ptr<core::CoreClass> c = std::make_unique<core::CoreClass>();
    std::unique_ptr<hlsl::HLSLClass> h = std::make_unique<hlsl::HLSLClass>();
    std::unique_ptr<msl::MSLClass> m = std::make_unique<msl::MSLClass>();
    std::unique_ptr<glsl::GLSLClass> g = std::make_unique<glsl::GLSLClass>();
    std::unique_ptr<spirv::SPIRVClass> s = std::make_unique<spirv::SPIRVClass>();

    EXPECT_TRUE(c->IsDialect(tint::Dialect::kCore));
    EXPECT_FALSE(c->IsDialect(tint::Dialect::kHlsl));
    EXPECT_FALSE(c->IsDialect(tint::Dialect::kMsl));
    EXPECT_FALSE(c->IsDialect(tint::Dialect::kGlsl));
    EXPECT_FALSE(c->IsDialect(tint::Dialect::kSpirv));

    EXPECT_FALSE(h->IsDialect(tint::Dialect::kCore));
    EXPECT_TRUE(h->IsDialect(tint::Dialect::kHlsl));
    EXPECT_FALSE(h->IsDialect(tint::Dialect::kMsl));
    EXPECT_FALSE(h->IsDialect(tint::Dialect::kGlsl));
    EXPECT_FALSE(h->IsDialect(tint::Dialect::kSpirv));

    EXPECT_FALSE(m->IsDialect(tint::Dialect::kCore));
    EXPECT_FALSE(m->IsDialect(tint::Dialect::kHlsl));
    EXPECT_TRUE(m->IsDialect(tint::Dialect::kMsl));
    EXPECT_FALSE(m->IsDialect(tint::Dialect::kGlsl));
    EXPECT_FALSE(m->IsDialect(tint::Dialect::kSpirv));

    EXPECT_FALSE(g->IsDialect(tint::Dialect::kCore));
    EXPECT_FALSE(g->IsDialect(tint::Dialect::kHlsl));
    EXPECT_FALSE(g->IsDialect(tint::Dialect::kMsl));
    EXPECT_TRUE(g->IsDialect(tint::Dialect::kGlsl));
    EXPECT_FALSE(g->IsDialect(tint::Dialect::kSpirv));

    EXPECT_FALSE(s->IsDialect(tint::Dialect::kCore));
    EXPECT_FALSE(s->IsDialect(tint::Dialect::kHlsl));
    EXPECT_FALSE(s->IsDialect(tint::Dialect::kMsl));
    EXPECT_FALSE(s->IsDialect(tint::Dialect::kGlsl));
    EXPECT_TRUE(s->IsDialect(tint::Dialect::kSpirv));
}

TEST(CastableBase, Is) {
    std::unique_ptr<CastableBase> frog = std::make_unique<Jetplane>();
    std::unique_ptr<CastableBase> bear = std::make_unique<Jeep>();
    std::unique_ptr<CastableBase> gecko = std::make_unique<Pogo>();

    ASSERT_TRUE(frog->Is<Vehicle>());
    ASSERT_TRUE(bear->Is<Vehicle>());
    ASSERT_TRUE(gecko->Is<Vehicle>());

    ASSERT_TRUE(frog->Is<Airplane>());
    ASSERT_FALSE(bear->Is<Airplane>());
    ASSERT_FALSE(gecko->Is<Airplane>());

    ASSERT_FALSE(frog->Is<Wheeled>());
    ASSERT_TRUE(bear->Is<Wheeled>());
    ASSERT_FALSE(gecko->Is<Wheeled>());

    ASSERT_FALSE(frog->Is<Bicycle>());
    ASSERT_FALSE(bear->Is<Bicycle>());
    ASSERT_TRUE(gecko->Is<Bicycle>());
}

TEST(CastableBase, Is_kDontErrorOnImpossibleCast) {
    // Unlike TEST(CastableBase, Is), we're dynamically querying [A -> B] without
    // going via CastableBase.
    auto frog = std::make_unique<Jetplane>();
    auto bear = std::make_unique<Jeep>();
    auto gecko = std::make_unique<Pogo>();

    ASSERT_TRUE((frog->Is<Vehicle, kDontErrorOnImpossibleCast>()));
    ASSERT_TRUE((bear->Is<Vehicle, kDontErrorOnImpossibleCast>()));
    ASSERT_TRUE((gecko->Is<Vehicle, kDontErrorOnImpossibleCast>()));

    ASSERT_TRUE((frog->Is<Airplane, kDontErrorOnImpossibleCast>()));
    ASSERT_FALSE((bear->Is<Airplane, kDontErrorOnImpossibleCast>()));
    ASSERT_FALSE((gecko->Is<Airplane, kDontErrorOnImpossibleCast>()));

    ASSERT_FALSE((frog->Is<Wheeled, kDontErrorOnImpossibleCast>()));
    ASSERT_TRUE((bear->Is<Wheeled, kDontErrorOnImpossibleCast>()));
    ASSERT_FALSE((gecko->Is<Wheeled, kDontErrorOnImpossibleCast>()));

    ASSERT_FALSE((frog->Is<Bicycle, kDontErrorOnImpossibleCast>()));
    ASSERT_FALSE((bear->Is<Bicycle, kDontErrorOnImpossibleCast>()));
    ASSERT_TRUE((gecko->Is<Bicycle, kDontErrorOnImpossibleCast>()));
}

TEST(CastableBase, IsWithPredicate) {
    std::unique_ptr<CastableBase> frog = std::make_unique<Jetplane>();

    frog->Is<Vehicle>([&frog](const Vehicle* a) {
        EXPECT_EQ(a, frog.get());
        return true;
    });

    ASSERT_TRUE((frog->Is<Vehicle>([](const Vehicle*) { return true; })));
    ASSERT_FALSE((frog->Is<Vehicle>([](const Vehicle*) { return false; })));

    // Predicate not called if cast is invalid
    auto expect_not_called = [] { FAIL() << "Should not be called"; };
    ASSERT_FALSE((frog->Is<Jeep>([&](const Vehicle*) {
        expect_not_called();
        return true;
    })));
}

TEST(CastableBase, IsAnyOf) {
    std::unique_ptr<CastableBase> frog = std::make_unique<Jetplane>();
    std::unique_ptr<CastableBase> bear = std::make_unique<Jeep>();
    std::unique_ptr<CastableBase> gecko = std::make_unique<Pogo>();

    ASSERT_TRUE((frog->IsAnyOf<Vehicle, Wheeled, Airplane, Bicycle>()));
    ASSERT_TRUE((frog->IsAnyOf<Wheeled, Airplane>()));
    ASSERT_TRUE((frog->IsAnyOf<Airplane, Bicycle>()));
    ASSERT_FALSE((frog->IsAnyOf<Wheeled, Bicycle>()));

    ASSERT_TRUE((bear->IsAnyOf<Vehicle, Wheeled, Airplane, Bicycle>()));
    ASSERT_TRUE((bear->IsAnyOf<Wheeled, Airplane>()));
    ASSERT_TRUE((bear->IsAnyOf<Wheeled, Bicycle>()));
    ASSERT_FALSE((bear->IsAnyOf<Airplane, Bicycle>()));

    ASSERT_TRUE((gecko->IsAnyOf<Vehicle, Wheeled, Airplane, Bicycle>()));
    ASSERT_TRUE((gecko->IsAnyOf<Wheeled, Bicycle>()));
    ASSERT_TRUE((gecko->IsAnyOf<Airplane, Bicycle>()));
    ASSERT_FALSE((gecko->IsAnyOf<Wheeled, Airplane>()));
}

TEST(CastableBase, As) {
    std::unique_ptr<CastableBase> frog = std::make_unique<Jetplane>();
    std::unique_ptr<CastableBase> bear = std::make_unique<Jeep>();
    std::unique_ptr<CastableBase> gecko = std::make_unique<Pogo>();

    ASSERT_EQ(frog->As<Vehicle>(), static_cast<Vehicle*>(frog.get()));
    ASSERT_EQ(bear->As<Vehicle>(), static_cast<Vehicle*>(bear.get()));
    ASSERT_EQ(gecko->As<Vehicle>(), static_cast<Vehicle*>(gecko.get()));

    ASSERT_EQ(frog->As<Airplane>(), static_cast<Airplane*>(frog.get()));
    ASSERT_EQ(bear->As<Airplane>(), nullptr);
    ASSERT_EQ(gecko->As<Airplane>(), nullptr);

    ASSERT_EQ(frog->As<Wheeled>(), nullptr);
    ASSERT_EQ(bear->As<Wheeled>(), static_cast<Wheeled*>(bear.get()));
    ASSERT_EQ(gecko->As<Wheeled>(), nullptr);

    ASSERT_EQ(frog->As<Bicycle>(), nullptr);
    ASSERT_EQ(bear->As<Bicycle>(), nullptr);
    ASSERT_EQ(gecko->As<Bicycle>(), static_cast<Bicycle*>(gecko.get()));
}

TEST(CastableBase, As_kDontErrorOnImpossibleCast) {
    // Unlike TEST(CastableBase, As), we're dynamically casting [A -> B] without
    // going via CastableBase.
    auto frog = std::make_unique<Jetplane>();
    auto bear = std::make_unique<Jeep>();
    auto gecko = std::make_unique<Pogo>();

    ASSERT_EQ((frog->As<Vehicle, kDontErrorOnImpossibleCast>()), static_cast<Vehicle*>(frog.get()));
    ASSERT_EQ((bear->As<Vehicle, kDontErrorOnImpossibleCast>()), static_cast<Vehicle*>(bear.get()));
    ASSERT_EQ((gecko->As<Vehicle, kDontErrorOnImpossibleCast>()),
              static_cast<Vehicle*>(gecko.get()));

    ASSERT_EQ((frog->As<Airplane, kDontErrorOnImpossibleCast>()),
              static_cast<Airplane*>(frog.get()));
    ASSERT_EQ((bear->As<Airplane, kDontErrorOnImpossibleCast>()), nullptr);
    ASSERT_EQ((gecko->As<Airplane, kDontErrorOnImpossibleCast>()), nullptr);

    ASSERT_EQ((frog->As<Wheeled, kDontErrorOnImpossibleCast>()), nullptr);
    ASSERT_EQ((bear->As<Wheeled, kDontErrorOnImpossibleCast>()), static_cast<Wheeled*>(bear.get()));
    ASSERT_EQ((gecko->As<Wheeled, kDontErrorOnImpossibleCast>()), nullptr);

    ASSERT_EQ((frog->As<Bicycle, kDontErrorOnImpossibleCast>()), nullptr);
    ASSERT_EQ((bear->As<Bicycle, kDontErrorOnImpossibleCast>()), nullptr);
    ASSERT_EQ((gecko->As<Bicycle, kDontErrorOnImpossibleCast>()),
              static_cast<Bicycle*>(gecko.get()));
}

TEST(Castable, Is) {
    std::unique_ptr<Vehicle> frog = std::make_unique<Jetplane>();
    std::unique_ptr<Vehicle> bear = std::make_unique<Jeep>();
    std::unique_ptr<Vehicle> gecko = std::make_unique<Pogo>();

    ASSERT_TRUE(frog->Is<Vehicle>());
    ASSERT_TRUE(bear->Is<Vehicle>());
    ASSERT_TRUE(gecko->Is<Vehicle>());

    ASSERT_TRUE(frog->Is<Airplane>());
    ASSERT_FALSE(bear->Is<Airplane>());
    ASSERT_FALSE(gecko->Is<Airplane>());

    ASSERT_FALSE(frog->Is<Wheeled>());
    ASSERT_TRUE(bear->Is<Wheeled>());
    ASSERT_FALSE(gecko->Is<Wheeled>());

    ASSERT_FALSE(frog->Is<Bicycle>());
    ASSERT_FALSE(bear->Is<Bicycle>());
    ASSERT_TRUE(gecko->Is<Bicycle>());
}

TEST(Castable, IsWithPredicate) {
    std::unique_ptr<Vehicle> frog = std::make_unique<Jetplane>();

    frog->Is([&frog](const Vehicle* a) {
        EXPECT_EQ(a, frog.get());
        return true;
    });

    ASSERT_TRUE((frog->Is([](const Vehicle*) { return true; })));
    ASSERT_FALSE((frog->Is([](const Vehicle*) { return false; })));

    // Predicate not called if cast is invalid
    auto expect_not_called = [] { FAIL() << "Should not be called"; };
    ASSERT_FALSE((frog->Is([&](const Jeep*) {
        expect_not_called();
        return true;
    })));
}

TEST(Castable, As) {
    std::unique_ptr<Vehicle> frog = std::make_unique<Jetplane>();
    std::unique_ptr<Vehicle> bear = std::make_unique<Jeep>();
    std::unique_ptr<Vehicle> gecko = std::make_unique<Pogo>();

    ASSERT_EQ(frog->As<Vehicle>(), static_cast<Vehicle*>(frog.get()));
    ASSERT_EQ(bear->As<Vehicle>(), static_cast<Vehicle*>(bear.get()));
    ASSERT_EQ(gecko->As<Vehicle>(), static_cast<Vehicle*>(gecko.get()));

    ASSERT_EQ(frog->As<Airplane>(), static_cast<Airplane*>(frog.get()));
    ASSERT_EQ(bear->As<Airplane>(), nullptr);
    ASSERT_EQ(gecko->As<Airplane>(), nullptr);

    ASSERT_EQ(frog->As<Wheeled>(), nullptr);
    ASSERT_EQ(bear->As<Wheeled>(), static_cast<Wheeled*>(bear.get()));
    ASSERT_EQ(gecko->As<Wheeled>(), nullptr);

    ASSERT_EQ(frog->As<Bicycle>(), nullptr);
    ASSERT_EQ(bear->As<Bicycle>(), nullptr);
    ASSERT_EQ(gecko->As<Bicycle>(), static_cast<Bicycle*>(gecko.get()));
}

// IsCastable static tests
static_assert(IsCastable<CastableBase>);
static_assert(IsCastable<Vehicle>);
static_assert(IsCastable<Ignore, Jetplane, Jeep>);
static_assert(IsCastable<Wheeled, Ignore, Airplane, Pogo>);
static_assert(!IsCastable<Wheeled, int, Airplane, Ignore, Pogo>);
static_assert(!IsCastable<bool>);
static_assert(!IsCastable<int, float>);
static_assert(!IsCastable<Ignore>);

// CastableCommonBase static tests
static_assert(std::is_same_v<Vehicle, CastableCommonBase<Vehicle>>);
static_assert(std::is_same_v<Airplane, CastableCommonBase<Airplane>>);
static_assert(std::is_same_v<Wheeled, CastableCommonBase<Wheeled>>);
static_assert(std::is_same_v<Bicycle, CastableCommonBase<Bicycle>>);
static_assert(std::is_same_v<Jetplane, CastableCommonBase<Jetplane>>);
static_assert(std::is_same_v<Jeep, CastableCommonBase<Jeep>>);
static_assert(std::is_same_v<Tricycle, CastableCommonBase<Tricycle>>);
static_assert(std::is_same_v<Pogo, CastableCommonBase<Pogo>>);
static_assert(std::is_same_v<Paddleboard, CastableCommonBase<Paddleboard>>);

static_assert(std::is_same_v<Vehicle, CastableCommonBase<Vehicle, Vehicle>>);
static_assert(std::is_same_v<Airplane, CastableCommonBase<Airplane, Airplane>>);
static_assert(std::is_same_v<Wheeled, CastableCommonBase<Wheeled, Wheeled>>);
static_assert(std::is_same_v<Bicycle, CastableCommonBase<Bicycle, Bicycle>>);
static_assert(std::is_same_v<Jetplane, CastableCommonBase<Jetplane, Jetplane>>);
static_assert(std::is_same_v<Jeep, CastableCommonBase<Jeep, Jeep>>);
static_assert(std::is_same_v<Tricycle, CastableCommonBase<Tricycle, Tricycle>>);
static_assert(std::is_same_v<Pogo, CastableCommonBase<Pogo, Pogo>>);
static_assert(std::is_same_v<Paddleboard, CastableCommonBase<Paddleboard, Paddleboard>>);

static_assert(std::is_same_v<CastableBase, CastableCommonBase<CastableBase, Vehicle>>);
static_assert(std::is_same_v<CastableBase, CastableCommonBase<Vehicle, CastableBase>>);
static_assert(std::is_same_v<Airplane, CastableCommonBase<Airplane, Jetplane>>);
static_assert(std::is_same_v<Airplane, CastableCommonBase<Jetplane, Airplane>>);
static_assert(std::is_same_v<Vehicle, CastableCommonBase<Bicycle, Jetplane>>);
static_assert(std::is_same_v<Vehicle, CastableCommonBase<Jetplane, Bicycle>>);
static_assert(std::is_same_v<Vehicle, CastableCommonBase<Jeep, Jetplane>>);
static_assert(std::is_same_v<Vehicle, CastableCommonBase<Jetplane, Jeep>>);
static_assert(std::is_same_v<Tricycle, CastableCommonBase<Pogo, Paddleboard>>);

static_assert(std::is_same_v<Vehicle, CastableCommonBase<Jeep, Jetplane, Paddleboard>>);
static_assert(std::is_same_v<Tricycle, CastableCommonBase<Tricycle, Pogo, Paddleboard>>);
static_assert(std::is_same_v<Tricycle, CastableCommonBase<Pogo, Paddleboard, Tricycle>>);
static_assert(std::is_same_v<Tricycle, CastableCommonBase<Pogo, Tricycle, Paddleboard>>);
static_assert(std::is_same_v<Vehicle, CastableCommonBase<Jetplane, Pogo, Paddleboard>>);
static_assert(std::is_same_v<Vehicle, CastableCommonBase<Pogo, Paddleboard, Jetplane>>);
static_assert(std::is_same_v<Vehicle, CastableCommonBase<Pogo, Jetplane, Paddleboard>>);

static_assert(
    std::is_same_v<CastableBase, CastableCommonBase<Jeep, Jetplane, Paddleboard, CastableBase>>);

}  // namespace
}  // namespace core

TINT_INSTANTIATE_TYPEINFO(tint::core::Vehicle);
TINT_INSTANTIATE_TYPEINFO(tint::core::Airplane);
TINT_INSTANTIATE_TYPEINFO(tint::core::Wheeled);
TINT_INSTANTIATE_TYPEINFO(tint::core::Bicycle);
TINT_INSTANTIATE_TYPEINFO(tint::core::Jetplane);
TINT_INSTANTIATE_TYPEINFO(tint::core::Jeep);
TINT_INSTANTIATE_TYPEINFO(tint::core::Tricycle);
TINT_INSTANTIATE_TYPEINFO(tint::core::Pogo);

}  // namespace tint
