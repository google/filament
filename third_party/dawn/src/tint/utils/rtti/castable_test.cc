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
namespace {

struct Animal : public Castable<Animal> {};
struct Amphibian : public Castable<Amphibian, Animal> {};
struct Mammal : public Castable<Mammal, Animal> {};
struct Reptile : public Castable<Reptile, Animal> {};
struct Frog : public Castable<Frog, Amphibian> {};
struct Bear : public Castable<Bear, Mammal> {};
struct Lizard : public Castable<Lizard, Reptile> {};
struct Gecko : public Castable<Gecko, Lizard> {};
struct Iguana : public Castable<Iguana, Lizard> {};

TEST(CastableBase, Is) {
    std::unique_ptr<CastableBase> frog = std::make_unique<Frog>();
    std::unique_ptr<CastableBase> bear = std::make_unique<Bear>();
    std::unique_ptr<CastableBase> gecko = std::make_unique<Gecko>();

    ASSERT_TRUE(frog->Is<Animal>());
    ASSERT_TRUE(bear->Is<Animal>());
    ASSERT_TRUE(gecko->Is<Animal>());

    ASSERT_TRUE(frog->Is<Amphibian>());
    ASSERT_FALSE(bear->Is<Amphibian>());
    ASSERT_FALSE(gecko->Is<Amphibian>());

    ASSERT_FALSE(frog->Is<Mammal>());
    ASSERT_TRUE(bear->Is<Mammal>());
    ASSERT_FALSE(gecko->Is<Mammal>());

    ASSERT_FALSE(frog->Is<Reptile>());
    ASSERT_FALSE(bear->Is<Reptile>());
    ASSERT_TRUE(gecko->Is<Reptile>());
}

TEST(CastableBase, Is_kDontErrorOnImpossibleCast) {
    // Unlike TEST(CastableBase, Is), we're dynamically querying [A -> B] without
    // going via CastableBase.
    auto frog = std::make_unique<Frog>();
    auto bear = std::make_unique<Bear>();
    auto gecko = std::make_unique<Gecko>();

    ASSERT_TRUE((frog->Is<Animal, kDontErrorOnImpossibleCast>()));
    ASSERT_TRUE((bear->Is<Animal, kDontErrorOnImpossibleCast>()));
    ASSERT_TRUE((gecko->Is<Animal, kDontErrorOnImpossibleCast>()));

    ASSERT_TRUE((frog->Is<Amphibian, kDontErrorOnImpossibleCast>()));
    ASSERT_FALSE((bear->Is<Amphibian, kDontErrorOnImpossibleCast>()));
    ASSERT_FALSE((gecko->Is<Amphibian, kDontErrorOnImpossibleCast>()));

    ASSERT_FALSE((frog->Is<Mammal, kDontErrorOnImpossibleCast>()));
    ASSERT_TRUE((bear->Is<Mammal, kDontErrorOnImpossibleCast>()));
    ASSERT_FALSE((gecko->Is<Mammal, kDontErrorOnImpossibleCast>()));

    ASSERT_FALSE((frog->Is<Reptile, kDontErrorOnImpossibleCast>()));
    ASSERT_FALSE((bear->Is<Reptile, kDontErrorOnImpossibleCast>()));
    ASSERT_TRUE((gecko->Is<Reptile, kDontErrorOnImpossibleCast>()));
}

TEST(CastableBase, IsWithPredicate) {
    std::unique_ptr<CastableBase> frog = std::make_unique<Frog>();

    frog->Is<Animal>([&frog](const Animal* a) {
        EXPECT_EQ(a, frog.get());
        return true;
    });

    ASSERT_TRUE((frog->Is<Animal>([](const Animal*) { return true; })));
    ASSERT_FALSE((frog->Is<Animal>([](const Animal*) { return false; })));

    // Predicate not called if cast is invalid
    auto expect_not_called = [] { FAIL() << "Should not be called"; };
    ASSERT_FALSE((frog->Is<Bear>([&](const Animal*) {
        expect_not_called();
        return true;
    })));
}

TEST(CastableBase, IsAnyOf) {
    std::unique_ptr<CastableBase> frog = std::make_unique<Frog>();
    std::unique_ptr<CastableBase> bear = std::make_unique<Bear>();
    std::unique_ptr<CastableBase> gecko = std::make_unique<Gecko>();

    ASSERT_TRUE((frog->IsAnyOf<Animal, Mammal, Amphibian, Reptile>()));
    ASSERT_TRUE((frog->IsAnyOf<Mammal, Amphibian>()));
    ASSERT_TRUE((frog->IsAnyOf<Amphibian, Reptile>()));
    ASSERT_FALSE((frog->IsAnyOf<Mammal, Reptile>()));

    ASSERT_TRUE((bear->IsAnyOf<Animal, Mammal, Amphibian, Reptile>()));
    ASSERT_TRUE((bear->IsAnyOf<Mammal, Amphibian>()));
    ASSERT_TRUE((bear->IsAnyOf<Mammal, Reptile>()));
    ASSERT_FALSE((bear->IsAnyOf<Amphibian, Reptile>()));

    ASSERT_TRUE((gecko->IsAnyOf<Animal, Mammal, Amphibian, Reptile>()));
    ASSERT_TRUE((gecko->IsAnyOf<Mammal, Reptile>()));
    ASSERT_TRUE((gecko->IsAnyOf<Amphibian, Reptile>()));
    ASSERT_FALSE((gecko->IsAnyOf<Mammal, Amphibian>()));
}

TEST(CastableBase, As) {
    std::unique_ptr<CastableBase> frog = std::make_unique<Frog>();
    std::unique_ptr<CastableBase> bear = std::make_unique<Bear>();
    std::unique_ptr<CastableBase> gecko = std::make_unique<Gecko>();

    ASSERT_EQ(frog->As<Animal>(), static_cast<Animal*>(frog.get()));
    ASSERT_EQ(bear->As<Animal>(), static_cast<Animal*>(bear.get()));
    ASSERT_EQ(gecko->As<Animal>(), static_cast<Animal*>(gecko.get()));

    ASSERT_EQ(frog->As<Amphibian>(), static_cast<Amphibian*>(frog.get()));
    ASSERT_EQ(bear->As<Amphibian>(), nullptr);
    ASSERT_EQ(gecko->As<Amphibian>(), nullptr);

    ASSERT_EQ(frog->As<Mammal>(), nullptr);
    ASSERT_EQ(bear->As<Mammal>(), static_cast<Mammal*>(bear.get()));
    ASSERT_EQ(gecko->As<Mammal>(), nullptr);

    ASSERT_EQ(frog->As<Reptile>(), nullptr);
    ASSERT_EQ(bear->As<Reptile>(), nullptr);
    ASSERT_EQ(gecko->As<Reptile>(), static_cast<Reptile*>(gecko.get()));
}

TEST(CastableBase, As_kDontErrorOnImpossibleCast) {
    // Unlike TEST(CastableBase, As), we're dynamically casting [A -> B] without
    // going via CastableBase.
    auto frog = std::make_unique<Frog>();
    auto bear = std::make_unique<Bear>();
    auto gecko = std::make_unique<Gecko>();

    ASSERT_EQ((frog->As<Animal, kDontErrorOnImpossibleCast>()), static_cast<Animal*>(frog.get()));
    ASSERT_EQ((bear->As<Animal, kDontErrorOnImpossibleCast>()), static_cast<Animal*>(bear.get()));
    ASSERT_EQ((gecko->As<Animal, kDontErrorOnImpossibleCast>()), static_cast<Animal*>(gecko.get()));

    ASSERT_EQ((frog->As<Amphibian, kDontErrorOnImpossibleCast>()),
              static_cast<Amphibian*>(frog.get()));
    ASSERT_EQ((bear->As<Amphibian, kDontErrorOnImpossibleCast>()), nullptr);
    ASSERT_EQ((gecko->As<Amphibian, kDontErrorOnImpossibleCast>()), nullptr);

    ASSERT_EQ((frog->As<Mammal, kDontErrorOnImpossibleCast>()), nullptr);
    ASSERT_EQ((bear->As<Mammal, kDontErrorOnImpossibleCast>()), static_cast<Mammal*>(bear.get()));
    ASSERT_EQ((gecko->As<Mammal, kDontErrorOnImpossibleCast>()), nullptr);

    ASSERT_EQ((frog->As<Reptile, kDontErrorOnImpossibleCast>()), nullptr);
    ASSERT_EQ((bear->As<Reptile, kDontErrorOnImpossibleCast>()), nullptr);
    ASSERT_EQ((gecko->As<Reptile, kDontErrorOnImpossibleCast>()),
              static_cast<Reptile*>(gecko.get()));
}

TEST(Castable, Is) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();

    ASSERT_TRUE(frog->Is<Animal>());
    ASSERT_TRUE(bear->Is<Animal>());
    ASSERT_TRUE(gecko->Is<Animal>());

    ASSERT_TRUE(frog->Is<Amphibian>());
    ASSERT_FALSE(bear->Is<Amphibian>());
    ASSERT_FALSE(gecko->Is<Amphibian>());

    ASSERT_FALSE(frog->Is<Mammal>());
    ASSERT_TRUE(bear->Is<Mammal>());
    ASSERT_FALSE(gecko->Is<Mammal>());

    ASSERT_FALSE(frog->Is<Reptile>());
    ASSERT_FALSE(bear->Is<Reptile>());
    ASSERT_TRUE(gecko->Is<Reptile>());
}

TEST(Castable, IsWithPredicate) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();

    frog->Is([&frog](const Animal* a) {
        EXPECT_EQ(a, frog.get());
        return true;
    });

    ASSERT_TRUE((frog->Is([](const Animal*) { return true; })));
    ASSERT_FALSE((frog->Is([](const Animal*) { return false; })));

    // Predicate not called if cast is invalid
    auto expect_not_called = [] { FAIL() << "Should not be called"; };
    ASSERT_FALSE((frog->Is([&](const Bear*) {
        expect_not_called();
        return true;
    })));
}

TEST(Castable, As) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();

    ASSERT_EQ(frog->As<Animal>(), static_cast<Animal*>(frog.get()));
    ASSERT_EQ(bear->As<Animal>(), static_cast<Animal*>(bear.get()));
    ASSERT_EQ(gecko->As<Animal>(), static_cast<Animal*>(gecko.get()));

    ASSERT_EQ(frog->As<Amphibian>(), static_cast<Amphibian*>(frog.get()));
    ASSERT_EQ(bear->As<Amphibian>(), nullptr);
    ASSERT_EQ(gecko->As<Amphibian>(), nullptr);

    ASSERT_EQ(frog->As<Mammal>(), nullptr);
    ASSERT_EQ(bear->As<Mammal>(), static_cast<Mammal*>(bear.get()));
    ASSERT_EQ(gecko->As<Mammal>(), nullptr);

    ASSERT_EQ(frog->As<Reptile>(), nullptr);
    ASSERT_EQ(bear->As<Reptile>(), nullptr);
    ASSERT_EQ(gecko->As<Reptile>(), static_cast<Reptile*>(gecko.get()));
}

// IsCastable static tests
static_assert(IsCastable<CastableBase>);
static_assert(IsCastable<Animal>);
static_assert(IsCastable<Ignore, Frog, Bear>);
static_assert(IsCastable<Mammal, Ignore, Amphibian, Gecko>);
static_assert(!IsCastable<Mammal, int, Amphibian, Ignore, Gecko>);
static_assert(!IsCastable<bool>);
static_assert(!IsCastable<int, float>);
static_assert(!IsCastable<Ignore>);

// CastableCommonBase static tests
static_assert(std::is_same_v<Animal, CastableCommonBase<Animal>>);
static_assert(std::is_same_v<Amphibian, CastableCommonBase<Amphibian>>);
static_assert(std::is_same_v<Mammal, CastableCommonBase<Mammal>>);
static_assert(std::is_same_v<Reptile, CastableCommonBase<Reptile>>);
static_assert(std::is_same_v<Frog, CastableCommonBase<Frog>>);
static_assert(std::is_same_v<Bear, CastableCommonBase<Bear>>);
static_assert(std::is_same_v<Lizard, CastableCommonBase<Lizard>>);
static_assert(std::is_same_v<Gecko, CastableCommonBase<Gecko>>);
static_assert(std::is_same_v<Iguana, CastableCommonBase<Iguana>>);

static_assert(std::is_same_v<Animal, CastableCommonBase<Animal, Animal>>);
static_assert(std::is_same_v<Amphibian, CastableCommonBase<Amphibian, Amphibian>>);
static_assert(std::is_same_v<Mammal, CastableCommonBase<Mammal, Mammal>>);
static_assert(std::is_same_v<Reptile, CastableCommonBase<Reptile, Reptile>>);
static_assert(std::is_same_v<Frog, CastableCommonBase<Frog, Frog>>);
static_assert(std::is_same_v<Bear, CastableCommonBase<Bear, Bear>>);
static_assert(std::is_same_v<Lizard, CastableCommonBase<Lizard, Lizard>>);
static_assert(std::is_same_v<Gecko, CastableCommonBase<Gecko, Gecko>>);
static_assert(std::is_same_v<Iguana, CastableCommonBase<Iguana, Iguana>>);

static_assert(std::is_same_v<CastableBase, CastableCommonBase<CastableBase, Animal>>);
static_assert(std::is_same_v<CastableBase, CastableCommonBase<Animal, CastableBase>>);
static_assert(std::is_same_v<Amphibian, CastableCommonBase<Amphibian, Frog>>);
static_assert(std::is_same_v<Amphibian, CastableCommonBase<Frog, Amphibian>>);
static_assert(std::is_same_v<Animal, CastableCommonBase<Reptile, Frog>>);
static_assert(std::is_same_v<Animal, CastableCommonBase<Frog, Reptile>>);
static_assert(std::is_same_v<Animal, CastableCommonBase<Bear, Frog>>);
static_assert(std::is_same_v<Animal, CastableCommonBase<Frog, Bear>>);
static_assert(std::is_same_v<Lizard, CastableCommonBase<Gecko, Iguana>>);

static_assert(std::is_same_v<Animal, CastableCommonBase<Bear, Frog, Iguana>>);
static_assert(std::is_same_v<Lizard, CastableCommonBase<Lizard, Gecko, Iguana>>);
static_assert(std::is_same_v<Lizard, CastableCommonBase<Gecko, Iguana, Lizard>>);
static_assert(std::is_same_v<Lizard, CastableCommonBase<Gecko, Lizard, Iguana>>);
static_assert(std::is_same_v<Animal, CastableCommonBase<Frog, Gecko, Iguana>>);
static_assert(std::is_same_v<Animal, CastableCommonBase<Gecko, Iguana, Frog>>);
static_assert(std::is_same_v<Animal, CastableCommonBase<Gecko, Frog, Iguana>>);

static_assert(std::is_same_v<CastableBase, CastableCommonBase<Bear, Frog, Iguana, CastableBase>>);

}  // namespace

TINT_INSTANTIATE_TYPEINFO(Animal);
TINT_INSTANTIATE_TYPEINFO(Amphibian);
TINT_INSTANTIATE_TYPEINFO(Mammal);
TINT_INSTANTIATE_TYPEINFO(Reptile);
TINT_INSTANTIATE_TYPEINFO(Frog);
TINT_INSTANTIATE_TYPEINFO(Bear);
TINT_INSTANTIATE_TYPEINFO(Lizard);
TINT_INSTANTIATE_TYPEINFO(Gecko);

}  // namespace tint
