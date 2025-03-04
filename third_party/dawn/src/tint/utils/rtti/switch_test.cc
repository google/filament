// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/utils/rtti/switch.h"

#include <memory>
#include <string>

#include "gmock/gmock.h"
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

TEST(Castable, SwitchNoDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        bool frog_matched_amphibian = false;
        Switch(
            frog.get(),  //
            [&](Reptile*) { FAIL() << "frog is not reptile"; },
            [&](Mammal*) { FAIL() << "frog is not mammal"; },
            [&](Amphibian* amphibian) {
                EXPECT_EQ(amphibian, frog.get());
                frog_matched_amphibian = true;
            });
        EXPECT_TRUE(frog_matched_amphibian);
    }
    {
        bool bear_matched_mammal = false;
        Switch(
            bear.get(),  //
            [&](Reptile*) { FAIL() << "bear is not reptile"; },
            [&](Amphibian*) { FAIL() << "bear is not amphibian"; },
            [&](Mammal* mammal) {
                EXPECT_EQ(mammal, bear.get());
                bear_matched_mammal = true;
            });
        EXPECT_TRUE(bear_matched_mammal);
    }
    {
        bool gecko_matched_reptile = false;
        Switch(
            gecko.get(),  //
            [&](Mammal*) { FAIL() << "gecko is not mammal"; },
            [&](Amphibian*) { FAIL() << "gecko is not amphibian"; },
            [&](Reptile* reptile) {
                EXPECT_EQ(reptile, gecko.get());
                gecko_matched_reptile = true;
            });
        EXPECT_TRUE(gecko_matched_reptile);
    }
}

TEST(Castable, SwitchWithUnusedDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        bool frog_matched_amphibian = false;
        Switch(
            frog.get(),  //
            [&](Reptile*) { FAIL() << "frog is not reptile"; },
            [&](Mammal*) { FAIL() << "frog is not mammal"; },
            [&](Amphibian* amphibian) {
                EXPECT_EQ(amphibian, frog.get());
                frog_matched_amphibian = true;
            },
            [&](Default) { FAIL() << "default should not have been selected"; });
        EXPECT_TRUE(frog_matched_amphibian);
    }
    {
        bool bear_matched_mammal = false;
        Switch(
            bear.get(),  //
            [&](Reptile*) { FAIL() << "bear is not reptile"; },
            [&](Amphibian*) { FAIL() << "bear is not amphibian"; },
            [&](Mammal* mammal) {
                EXPECT_EQ(mammal, bear.get());
                bear_matched_mammal = true;
            },
            [&](Default) { FAIL() << "default should not have been selected"; });
        EXPECT_TRUE(bear_matched_mammal);
    }
    {
        bool gecko_matched_reptile = false;
        Switch(
            gecko.get(),  //
            [&](Mammal*) { FAIL() << "gecko is not mammal"; },
            [&](Amphibian*) { FAIL() << "gecko is not amphibian"; },
            [&](Reptile* reptile) {
                EXPECT_EQ(reptile, gecko.get());
                gecko_matched_reptile = true;
            },
            [&](Default) { FAIL() << "default should not have been selected"; });
        EXPECT_TRUE(gecko_matched_reptile);
    }
}

TEST(Castable, SwitchDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        bool frog_matched_default = false;
        Switch(
            frog.get(),  //
            [&](Reptile*) { FAIL() << "frog is not reptile"; },
            [&](Mammal*) { FAIL() << "frog is not mammal"; },
            [&](Default) { frog_matched_default = true; });
        EXPECT_TRUE(frog_matched_default);
    }
    {
        bool bear_matched_default = false;
        Switch(
            bear.get(),  //
            [&](Reptile*) { FAIL() << "bear is not reptile"; },
            [&](Amphibian*) { FAIL() << "bear is not amphibian"; },
            [&](Default) { bear_matched_default = true; });
        EXPECT_TRUE(bear_matched_default);
    }
    {
        bool gecko_matched_default = false;
        Switch(
            gecko.get(),  //
            [&](Mammal*) { FAIL() << "gecko is not mammal"; },
            [&](Amphibian*) { FAIL() << "gecko is not amphibian"; },
            [&](Default) { gecko_matched_default = true; });
        EXPECT_TRUE(gecko_matched_default);
    }
}

TEST(Castable, SwitchMustMatch_MatchedWithoutReturnValue) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        bool ok = false;
        Switch(
            frog.get(),                      //
            [&](Amphibian*) { ok = true; },  //
            [&](Mammal*) {},                 //
            TINT_ICE_ON_NO_MATCH);
        EXPECT_TRUE(ok);
    }
    {
        bool ok = false;
        Switch(
            bear.get(),                   //
            [&](Amphibian*) {},           //
            [&](Mammal*) { ok = true; },  //
            TINT_ICE_ON_NO_MATCH);        //
        EXPECT_TRUE(ok);
    }
    {
        bool ok = false;
        Switch(
            gecko.get(),                   //
            [&](Reptile*) { ok = true; },  //
            [&](Amphibian*) {},            //
            TINT_ICE_ON_NO_MATCH);         //
        EXPECT_TRUE(ok);
    }
}

TEST(Castable, SwitchMustMatch_MatchedWithReturnValue) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        int res = Switch(
            frog.get(),                     //
            [&](Amphibian*) { return 1; },  //
            [&](Mammal*) { return 0; },     //
            TINT_ICE_ON_NO_MATCH);
        EXPECT_EQ(res, 1);
    }
    {
        int res = Switch(
            bear.get(),                     //
            [&](Amphibian*) { return 0; },  //
            [&](Mammal*) { return 2; },     //
            TINT_ICE_ON_NO_MATCH);
        EXPECT_EQ(res, 2);
    }
    {
        int res = Switch(
            gecko.get(),                    //
            [&](Reptile*) { return 3; },    //
            [&](Amphibian*) { return 0; },  //
            TINT_ICE_ON_NO_MATCH);
        EXPECT_EQ(res, 3);
    }
}

TEST(CastableDeathTest, SwitchMustMatch_NoMatchWithoutReturnValue) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            std::unique_ptr<Animal> frog = std::make_unique<Frog>();
            Switch(
                frog.get(),        //
                [&](Reptile*) {},  //
                [&](Mammal*) {},   //
                TINT_ICE_ON_NO_MATCH);
        },
        testing::HasSubstr("internal compiler error: Switch() matched no cases. Type: Frog"));
}

TEST(CastableDeathTest, SwitchMustMatch_NoMatchWithReturnValue) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            std::unique_ptr<Animal> frog = std::make_unique<Frog>();
            int res = Switch(
                frog.get(),                   //
                [&](Reptile*) { return 1; },  //
                [&](Mammal*) { return 2; },   //
                TINT_ICE_ON_NO_MATCH);
            ASSERT_EQ(res, 0);
        },
        testing::HasSubstr("internal compiler error: Switch() matched no cases. Type: Frog"));
}

TEST(CastableDeathTest, SwitchMustMatch_NullptrWithoutReturnValue) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Switch(
                static_cast<CastableBase*>(nullptr),  //
                [&](Reptile*) {},                     //
                [&](Mammal*) {},                      //
                TINT_ICE_ON_NO_MATCH);
        },
        testing::HasSubstr("internal compiler error: Switch() passed nullptr"));
}

TEST(CastableDeathTest, SwitchMustMatch_NullptrWithReturnValue) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            int res = Switch(
                static_cast<CastableBase*>(nullptr),  //
                [&](Reptile*) { return 1; },          //
                [&](Mammal*) { return 2; },           //
                TINT_ICE_ON_NO_MATCH);
            ASSERT_EQ(res, 0);
        },
        testing::HasSubstr("internal compiler error: Switch() passed nullptr"));
}

TEST(Castable, SwitchMatchFirst) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    {
        bool frog_matched_animal = false;
        Switch(
            frog.get(),
            [&](Animal* animal) {
                EXPECT_EQ(animal, frog.get());
                frog_matched_animal = true;
            },
            [&](Amphibian*) { FAIL() << "animal should have been matched first"; });
        EXPECT_TRUE(frog_matched_animal);
    }
    {
        bool frog_matched_amphibian = false;
        Switch(
            frog.get(),
            [&](Amphibian* amphibain) {
                EXPECT_EQ(amphibain, frog.get());
                frog_matched_amphibian = true;
            },
            [&](Animal*) { FAIL() << "amphibian should have been matched first"; });
        EXPECT_TRUE(frog_matched_amphibian);
    }
}

TEST(Castable, SwitchReturnValueWithDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        const char* result = Switch(
            frog.get(),                              //
            [](Mammal*) { return "mammal"; },        //
            [](Amphibian*) { return "amphibian"; },  //
            [](Default) { return "unknown"; });
        static_assert(std::is_same_v<decltype(result), const char*>);
        EXPECT_EQ(std::string(result), "amphibian");
    }
    {
        const char* result = Switch(
            bear.get(),                              //
            [](Mammal*) { return "mammal"; },        //
            [](Amphibian*) { return "amphibian"; },  //
            [](Default) { return "unknown"; });
        static_assert(std::is_same_v<decltype(result), const char*>);
        EXPECT_EQ(std::string(result), "mammal");
    }
    {
        const char* result = Switch(
            gecko.get(),                             //
            [](Mammal*) { return "mammal"; },        //
            [](Amphibian*) { return "amphibian"; },  //
            [](Default) { return "unknown"; });
        static_assert(std::is_same_v<decltype(result), const char*>);
        EXPECT_EQ(std::string(result), "unknown");
    }
}

TEST(Castable, SwitchReturnValueWithoutDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        const char* result = Switch(
            frog.get(),                        //
            [](Mammal*) { return "mammal"; },  //
            [](Amphibian*) { return "amphibian"; });
        static_assert(std::is_same_v<decltype(result), const char*>);
        EXPECT_EQ(std::string(result), "amphibian");
    }
    {
        const char* result = Switch(
            bear.get(),                        //
            [](Mammal*) { return "mammal"; },  //
            [](Amphibian*) { return "amphibian"; });
        static_assert(std::is_same_v<decltype(result), const char*>);
        EXPECT_EQ(std::string(result), "mammal");
    }
    {
        auto* result = Switch(
            gecko.get(),                       //
            [](Mammal*) { return "mammal"; },  //
            [](Amphibian*) { return "amphibian"; });
        static_assert(std::is_same_v<decltype(result), const char*>);
        EXPECT_EQ(result, nullptr);
    }
}

TEST(Castable, SwitchInferPODReturnTypeWithDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        auto result = Switch(
            frog.get(),                       //
            [](Mammal*) { return 1; },        //
            [](Amphibian*) { return 2.0f; },  //
            [](Default) { return 3.0; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 2.0);
    }
    {
        auto result = Switch(
            bear.get(),                       //
            [](Mammal*) { return 1.0; },      //
            [](Amphibian*) { return 2.0f; },  //
            [](Default) { return 3; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 1.0);
    }
    {
        auto result = Switch(
            gecko.get(),                   //
            [](Mammal*) { return 1.0f; },  //
            [](Amphibian*) { return 2; },  //
            [](Default) { return 3.0; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 3.0);
    }
}

TEST(Castable, SwitchInferPODReturnTypeWithoutDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        auto result = Switch(
            frog.get(),                 //
            [](Mammal*) { return 1; },  //
            [](Amphibian*) { return 2.0f; });
        static_assert(std::is_same_v<decltype(result), float>);
        EXPECT_EQ(result, 2.0f);
    }
    {
        auto result = Switch(
            bear.get(),                    //
            [](Mammal*) { return 1.0f; },  //
            [](Amphibian*) { return 2; });
        static_assert(std::is_same_v<decltype(result), float>);
        EXPECT_EQ(result, 1.0f);
    }
    {
        auto result = Switch(
            gecko.get(),                  //
            [](Mammal*) { return 1.0; },  //
            [](Amphibian*) { return 2.0f; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 0.0);
    }
}

TEST(Castable, SwitchInferCastableReturnTypeWithDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        auto* result = Switch(
            frog.get(),                          //
            [](Mammal* p) { return p; },         //
            [](Amphibian*) { return nullptr; },  //
            [](Default) { return nullptr; });
        static_assert(std::is_same_v<decltype(result), Mammal*>);
        EXPECT_EQ(result, nullptr);
    }
    {
        auto* result = Switch(
            bear.get(),                   //
            [](Mammal* p) { return p; },  //
            [](Amphibian* p) { return const_cast<const Amphibian*>(p); },
            [](Default) { return nullptr; });
        static_assert(std::is_same_v<decltype(result), const Animal*>);
        EXPECT_EQ(result, bear.get());
    }
    {
        auto* result = Switch(
            gecko.get(),                     //
            [](Mammal* p) { return p; },     //
            [](Amphibian* p) { return p; },  //
            [](Default) -> CastableBase* { return nullptr; });
        static_assert(std::is_same_v<decltype(result), CastableBase*>);
        EXPECT_EQ(result, nullptr);
    }
}

TEST(Castable, SwitchInferCastableReturnTypeWithoutDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        auto* result = Switch(
            frog.get(),                   //
            [](Mammal* p) { return p; },  //
            [](Amphibian*) { return nullptr; });
        static_assert(std::is_same_v<decltype(result), Mammal*>);
        EXPECT_EQ(result, nullptr);
    }
    {
        auto* result = Switch(
            bear.get(),                                                     //
            [](Mammal* p) { return p; },                                    //
            [](Amphibian* p) { return const_cast<const Amphibian*>(p); });  //
        static_assert(std::is_same_v<decltype(result), const Animal*>);
        EXPECT_EQ(result, bear.get());
    }
    {
        auto* result = Switch(
            gecko.get(),                  //
            [](Mammal* p) { return p; },  //
            [](Amphibian* p) { return p; });
        static_assert(std::is_same_v<decltype(result), Animal*>);
        EXPECT_EQ(result, nullptr);
    }
}

TEST(Castable, SwitchExplicitPODReturnTypeWithDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        auto result = Switch<double>(
            frog.get(),                       //
            [](Mammal*) { return 1; },        //
            [](Amphibian*) { return 2.0f; },  //
            [](Default) { return 3.0; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 2.0f);
    }
    {
        auto result = Switch<double>(
            bear.get(),                    //
            [](Mammal*) { return 1; },     //
            [](Amphibian*) { return 2; },  //
            [](Default) { return 3; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 1.0f);
    }
    {
        auto result = Switch<double>(
            gecko.get(),                      //
            [](Mammal*) { return 1.0f; },     //
            [](Amphibian*) { return 2.0f; },  //
            [](Default) { return 3.0f; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 3.0f);
    }
}

TEST(Castable, SwitchExplicitPODReturnTypeWithoutDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        auto result = Switch<double>(
            frog.get(),                 //
            [](Mammal*) { return 1; },  //
            [](Amphibian*) { return 2.0f; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 2.0f);
    }
    {
        auto result = Switch<double>(
            bear.get(),                    //
            [](Mammal*) { return 1.0f; },  //
            [](Amphibian*) { return 2; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 1.0f);
    }
    {
        auto result = Switch<double>(
            gecko.get(),                  //
            [](Mammal*) { return 1.0; },  //
            [](Amphibian*) { return 2.0f; });
        static_assert(std::is_same_v<decltype(result), double>);
        EXPECT_EQ(result, 0.0);
    }
}

TEST(Castable, SwitchExplicitCastableReturnTypeWithDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        auto* result = Switch<Animal>(
            frog.get(),                          //
            [](Mammal* p) { return p; },         //
            [](Amphibian*) { return nullptr; },  //
            [](Default) { return nullptr; });
        static_assert(std::is_same_v<decltype(result), Animal*>);
        EXPECT_EQ(result, nullptr);
    }
    {
        auto* result = Switch<CastableBase>(
            bear.get(),                   //
            [](Mammal* p) { return p; },  //
            [](Amphibian* p) { return const_cast<const Amphibian*>(p); },
            [](Default) { return nullptr; });
        static_assert(std::is_same_v<decltype(result), const CastableBase*>);
        EXPECT_EQ(result, bear.get());
    }
    {
        auto* result = Switch<const Animal>(
            gecko.get(),                     //
            [](Mammal* p) { return p; },     //
            [](Amphibian* p) { return p; },  //
            [](Default) { return nullptr; });
        static_assert(std::is_same_v<decltype(result), const Animal*>);
        EXPECT_EQ(result, nullptr);
    }
}

TEST(Castable, SwitchExplicitCastableReturnTypeWithoutDefault) {
    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    std::unique_ptr<Animal> bear = std::make_unique<Bear>();
    std::unique_ptr<Animal> gecko = std::make_unique<Gecko>();
    {
        auto* result = Switch<Animal>(
            frog.get(),                   //
            [](Mammal* p) { return p; },  //
            [](Amphibian*) { return nullptr; });
        static_assert(std::is_same_v<decltype(result), Animal*>);
        EXPECT_EQ(result, nullptr);
    }
    {
        auto* result = Switch<CastableBase>(
            bear.get(),                                                     //
            [](Mammal* p) { return p; },                                    //
            [](Amphibian* p) { return const_cast<const Amphibian*>(p); });  //
        static_assert(std::is_same_v<decltype(result), const CastableBase*>);
        EXPECT_EQ(result, bear.get());
    }
    {
        auto* result = Switch<const Animal*>(
            gecko.get(),                  //
            [](Mammal* p) { return p; },  //
            [](Amphibian* p) { return p; });
        static_assert(std::is_same_v<decltype(result), const Animal*>);
        EXPECT_EQ(result, nullptr);
    }
}

TEST(Castable, SwitchNull) {
    Animal* null = nullptr;
    Switch(
        null,  //
        [&](Amphibian*) { FAIL() << "should not be called"; },
        [&](Animal*) { FAIL() << "should not be called"; });
}

TEST(Castable, SwitchNullNoDefault) {
    Animal* null = nullptr;
    bool default_called = false;
    Switch(
        null,  //
        [&](Amphibian*) { FAIL() << "should not be called"; },
        [&](Animal*) { FAIL() << "should not be called"; },
        [&](Default) { default_called = true; });
    EXPECT_TRUE(default_called);
}

TEST(Castable, SwitchReturnNoDefaultInitializer) {
    struct Object {
        explicit Object(int v) : value(v) {}
        int value;
    };

    std::unique_ptr<Animal> frog = std::make_unique<Frog>();
    {
        auto result = Switch(
            frog.get(),                            //
            [](Mammal*) { return Object(1); },     //
            [](Amphibian*) { return Object(2); },  //
            [](Default) { return Object(3); });
        static_assert(std::is_same_v<decltype(result), Object>);
        EXPECT_EQ(result.value, 2);
    }
    {
        auto result = Switch(
            frog.get(),                         //
            [](Mammal*) { return Object(1); },  //
            [](Default) { return Object(3); });
        static_assert(std::is_same_v<decltype(result), Object>);
        EXPECT_EQ(result.value, 3);
    }
}

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
