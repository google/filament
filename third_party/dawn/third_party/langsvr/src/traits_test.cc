// Copyright 2024 The langsvr Authors
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

#include "langsvr/traits.h"

#include <functional>

namespace langsvr {
namespace {

static_assert(TypeIndex<int, int, bool, float> == 0);
static_assert(TypeIndex<bool, int, bool, float> == 1);
static_assert(TypeIndex<float, int, bool, float> == 2);

struct S {};
void F1(S) {}
void F3(int, S, float) {}

struct StaticTests {
    [[maybe_unused]] void Function() {
        F1({});        // Avoid unused method warning
        F3(0, {}, 0);  // Avoid unused method warning
        static_assert(std::is_same_v<ParameterType<decltype(&F1), 0>, S>);
        static_assert(std::is_same_v<ParameterType<decltype(&F3), 0>, int>);
        static_assert(std::is_same_v<ParameterType<decltype(&F3), 1>, S>);
        static_assert(std::is_same_v<ParameterType<decltype(&F3), 2>, float>);
        static_assert(std::is_same_v<ReturnType<decltype(&F1)>, void>);
        static_assert(std::is_same_v<ReturnType<decltype(&F3)>, void>);
        static_assert(SignatureOf<decltype(&F1)>::parameter_count == 1);
        static_assert(SignatureOf<decltype(&F3)>::parameter_count == 3);
    }

    [[maybe_unused]] void Method() {
        class C {
          public:
            void F1(S) {}
            void F3(int, S, float) {}
        };
        C().F1({});        // Avoid unused method warning
        C().F3(0, {}, 0);  // Avoid unused method warning
        static_assert(std::is_same_v<ParameterType<decltype(&C::F1), 0>, S>);
        static_assert(std::is_same_v<ParameterType<decltype(&C::F3), 0>, int>);
        static_assert(std::is_same_v<ParameterType<decltype(&C::F3), 1>, S>);
        static_assert(std::is_same_v<ParameterType<decltype(&C::F3), 2>, float>);
        static_assert(std::is_same_v<ReturnType<decltype(&C::F1)>, void>);
        static_assert(std::is_same_v<ReturnType<decltype(&C::F3)>, void>);
        static_assert(SignatureOf<decltype(&C::F1)>::parameter_count == 1);
        static_assert(SignatureOf<decltype(&C::F3)>::parameter_count == 3);
    }

    [[maybe_unused]] void ConstMethod() {
        class C {
          public:
            void F1(S) const {}
            void F3(int, S, float) const {}
        };
        C().F1({});        // Avoid unused method warning
        C().F3(0, {}, 0);  // Avoid unused method warning
        static_assert(std::is_same_v<ParameterType<decltype(&C::F1), 0>, S>);
        static_assert(std::is_same_v<ParameterType<decltype(&C::F3), 0>, int>);
        static_assert(std::is_same_v<ParameterType<decltype(&C::F3), 1>, S>);
        static_assert(std::is_same_v<ParameterType<decltype(&C::F3), 2>, float>);
        static_assert(std::is_same_v<ReturnType<decltype(&C::F1)>, void>);
        static_assert(std::is_same_v<ReturnType<decltype(&C::F3)>, void>);
        static_assert(SignatureOf<decltype(&C::F1)>::parameter_count == 1);
        static_assert(SignatureOf<decltype(&C::F3)>::parameter_count == 3);
    }

    [[maybe_unused]] void StaticMethod() {
        class C {
          public:
            static void F1(S) {}
            static void F3(int, S, float) {}
        };
        C::F1({});        // Avoid unused method warning
        C::F3(0, {}, 0);  // Avoid unused method warning
        static_assert(std::is_same_v<ParameterType<decltype(&C::F1), 0>, S>);
        static_assert(std::is_same_v<ParameterType<decltype(&C::F3), 0>, int>);
        static_assert(std::is_same_v<ParameterType<decltype(&C::F3), 1>, S>);
        static_assert(std::is_same_v<ParameterType<decltype(&C::F3), 2>, float>);
        static_assert(std::is_same_v<ReturnType<decltype(&C::F1)>, void>);
        static_assert(std::is_same_v<ReturnType<decltype(&C::F3)>, void>);
        static_assert(SignatureOf<decltype(&C::F1)>::parameter_count == 1);
        static_assert(SignatureOf<decltype(&C::F3)>::parameter_count == 3);
    }

    [[maybe_unused]] void FunctionLike() {
        using F1 = std::function<void(S)>;
        using F3 = std::function<void(int, S, float)>;
        static_assert(std::is_same_v<ParameterType<F1, 0>, S>);
        static_assert(std::is_same_v<ParameterType<F3, 0>, int>);
        static_assert(std::is_same_v<ParameterType<F3, 1>, S>);
        static_assert(std::is_same_v<ParameterType<F3, 2>, float>);
        static_assert(std::is_same_v<ReturnType<F1>, void>);
        static_assert(std::is_same_v<ReturnType<F3>, void>);
        static_assert(SignatureOf<F1>::parameter_count == 1);
        static_assert(SignatureOf<F3>::parameter_count == 3);
    }

    [[maybe_unused]] void Lambda() {
        auto l1 = [](S) {};
        auto l3 = [](int, S, float) {};
        static_assert(std::is_same_v<ParameterType<decltype(l1), 0>, S>);
        static_assert(std::is_same_v<ParameterType<decltype(l3), 0>, int>);
        static_assert(std::is_same_v<ParameterType<decltype(l3), 1>, S>);
        static_assert(std::is_same_v<ParameterType<decltype(l3), 2>, float>);
        static_assert(std::is_same_v<ReturnType<decltype(l1)>, void>);
        static_assert(std::is_same_v<ReturnType<decltype(l3)>, void>);
        static_assert(SignatureOf<decltype(l1)>::parameter_count == 1);
        static_assert(SignatureOf<decltype(l3)>::parameter_count == 3);
    }
};

}  // namespace
}  // namespace langsvr
