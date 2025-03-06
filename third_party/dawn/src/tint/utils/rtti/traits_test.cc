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

#include "src/tint/utils/rtti/traits.h"

#include <ostream>

#include "gtest/gtest.h"

namespace tint::traits {

namespace {

static_assert(traits::IsOStream<std::ostream>);
static_assert(traits::IsOStream<std::stringstream>);

static_assert(std::is_same_v<PtrElTy<int*>, int>);
static_assert(std::is_same_v<PtrElTy<int const*>, int>);
static_assert(std::is_same_v<PtrElTy<int const* const>, int>);
static_assert(std::is_same_v<PtrElTy<int const* const volatile>, int>);
static_assert(std::is_same_v<PtrElTy<int>, int>);
static_assert(std::is_same_v<PtrElTy<int const>, int>);
static_assert(std::is_same_v<PtrElTy<int const volatile>, int>);

static_assert(IsStringLike<std::string>);
static_assert(IsStringLike<std::string_view>);
static_assert(IsStringLike<const char*>);
static_assert(IsStringLike<const std::string&>);
static_assert(IsStringLike<const std::string_view&>);
static_assert(IsStringLike<const char*>);
static_assert(!IsStringLike<bool>);
static_assert(!IsStringLike<int>);
static_assert(!IsStringLike<const char**>);

struct S {};
void F1(S) {}
void F3(int, S, float) {}
}  // namespace

TEST(ParamType, Function) {
    F1({});        // Avoid unused method warning
    F3(0, {}, 0);  // Avoid unused method warning
    static_assert(std::is_same_v<ParameterType<decltype(&F1), 0>, S>);
    static_assert(std::is_same_v<ParameterType<decltype(&F3), 0>, int>);
    static_assert(std::is_same_v<ParameterType<decltype(&F3), 1>, S>);
    static_assert(std::is_same_v<ParameterType<decltype(&F3), 2>, float>);
    static_assert(std::is_same_v<ReturnType<decltype(&F1)>, void>);
    static_assert(std::is_same_v<ReturnType<decltype(&F3)>, void>);
    static_assert(SignatureOfT<decltype(&F1)>::parameter_count == 1);
    static_assert(SignatureOfT<decltype(&F3)>::parameter_count == 3);
}

TEST(ParamType, Method) {
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
    static_assert(SignatureOfT<decltype(&C::F1)>::parameter_count == 1);
    static_assert(SignatureOfT<decltype(&C::F3)>::parameter_count == 3);
}

TEST(ParamType, ConstMethod) {
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
    static_assert(SignatureOfT<decltype(&C::F1)>::parameter_count == 1);
    static_assert(SignatureOfT<decltype(&C::F3)>::parameter_count == 3);
}

TEST(ParamType, StaticMethod) {
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
    static_assert(SignatureOfT<decltype(&C::F1)>::parameter_count == 1);
    static_assert(SignatureOfT<decltype(&C::F3)>::parameter_count == 3);
}

TEST(ParamType, FunctionLike) {
    using F1 = std::function<void(S)>;
    using F3 = std::function<void(int, S, float)>;
    static_assert(std::is_same_v<ParameterType<F1, 0>, S>);
    static_assert(std::is_same_v<ParameterType<F3, 0>, int>);
    static_assert(std::is_same_v<ParameterType<F3, 1>, S>);
    static_assert(std::is_same_v<ParameterType<F3, 2>, float>);
    static_assert(std::is_same_v<ReturnType<F1>, void>);
    static_assert(std::is_same_v<ReturnType<F3>, void>);
    static_assert(SignatureOfT<F1>::parameter_count == 1);
    static_assert(SignatureOfT<F3>::parameter_count == 3);
}

TEST(ParamType, Lambda) {
    auto l1 = [](S) {};
    auto l3 = [](int, S, float) {};
    static_assert(std::is_same_v<ParameterType<decltype(l1), 0>, S>);
    static_assert(std::is_same_v<ParameterType<decltype(l3), 0>, int>);
    static_assert(std::is_same_v<ParameterType<decltype(l3), 1>, S>);
    static_assert(std::is_same_v<ParameterType<decltype(l3), 2>, float>);
    static_assert(std::is_same_v<ReturnType<decltype(l1)>, void>);
    static_assert(std::is_same_v<ReturnType<decltype(l3)>, void>);
    static_assert(SignatureOfT<decltype(l1)>::parameter_count == 1);
    static_assert(SignatureOfT<decltype(l3)>::parameter_count == 3);
}

TEST(Slice, Empty) {
    auto sliced = Slice<0, 0>(std::make_tuple<>());
    static_assert(std::tuple_size_v<decltype(sliced)> == 0);
}

TEST(Slice, SingleElementSliceEmpty) {
    auto sliced = Slice<0, 0>(std::make_tuple<int>(1));
    static_assert(std::tuple_size_v<decltype(sliced)> == 0);
}

TEST(Slice, SingleElementSliceFull) {
    auto sliced = Slice<0, 1>(std::make_tuple<int>(1));
    static_assert(std::tuple_size_v<decltype(sliced)> == 1);
    static_assert(std::is_same_v<std::tuple_element_t<0, decltype(sliced)>, int>, "");
    EXPECT_EQ(std::get<0>(sliced), 1);
}

TEST(Slice, MixedTupleSliceEmpty) {
    auto sliced = Slice<1, 0>(std::make_tuple<int, bool, float>(1, true, 2.0f));
    static_assert(std::tuple_size_v<decltype(sliced)> == 0);
}

TEST(Slice, MixedTupleSliceFull) {
    auto sliced = Slice<0, 3>(std::make_tuple<int, bool, float>(1, true, 2.0f));
    static_assert(std::tuple_size_v<decltype(sliced)> == 3);
    static_assert(std::is_same_v<std::tuple_element_t<0, decltype(sliced)>, int>, "");
    static_assert(std::is_same_v<std::tuple_element_t<1, decltype(sliced)>, bool>, "");
    static_assert(std::is_same_v<std::tuple_element_t<2, decltype(sliced)>, float>);
    EXPECT_EQ(std::get<0>(sliced), 1);
    EXPECT_EQ(std::get<1>(sliced), true);
    EXPECT_EQ(std::get<2>(sliced), 2.0f);
}

TEST(Slice, MixedTupleSliceLowPart) {
    auto sliced = Slice<0, 2>(std::make_tuple<int, bool, float>(1, true, 2.0f));
    static_assert(std::tuple_size_v<decltype(sliced)> == 2);
    static_assert(std::is_same_v<std::tuple_element_t<0, decltype(sliced)>, int>, "");
    static_assert(std::is_same_v<std::tuple_element_t<1, decltype(sliced)>, bool>, "");
    EXPECT_EQ(std::get<0>(sliced), 1);
    EXPECT_EQ(std::get<1>(sliced), true);
}

TEST(Slice, MixedTupleSliceHighPart) {
    auto sliced = Slice<1, 2>(std::make_tuple<int, bool, float>(1, true, 2.0f));
    static_assert(std::tuple_size_v<decltype(sliced)> == 2);
    static_assert(std::is_same_v<std::tuple_element_t<0, decltype(sliced)>, bool>, "");
    static_assert(std::is_same_v<std::tuple_element_t<1, decltype(sliced)>, float>);
    EXPECT_EQ(std::get<0>(sliced), true);
    EXPECT_EQ(std::get<1>(sliced), 2.0f);
}

TEST(Slice, PreservesRValueRef) {
    int i;
    int& int_ref = i;
    auto tuple = std::forward_as_tuple(std::move(int_ref));
    static_assert(std::is_same_v<int&&,  //
                                 std::tuple_element_t<0, decltype(tuple)>>);
    auto sliced = Slice<0, 1>(std::move(tuple));
    static_assert(std::is_same_v<int&&,  //
                                 std::tuple_element_t<0, decltype(sliced)>>);
}

TEST(SliceTuple, Empty) {
    using sliced = SliceTuple<0, 0, std::tuple<>>;
    static_assert(std::tuple_size_v<sliced> == 0);
}

TEST(SliceTuple, SingleElementSliceEmpty) {
    using sliced = SliceTuple<0, 0, std::tuple<int>>;
    static_assert(std::tuple_size_v<sliced> == 0);
}

TEST(SliceTuple, SingleElementSliceFull) {
    using sliced = SliceTuple<0, 1, std::tuple<int>>;
    static_assert(std::tuple_size_v<sliced> == 1);
    static_assert(std::is_same_v<std::tuple_element_t<0, sliced>, int>);
}

TEST(SliceTuple, MixedTupleSliceEmpty) {
    using sliced = SliceTuple<1, 0, std::tuple<int, bool, float>>;
    static_assert(std::tuple_size_v<sliced> == 0);
}

TEST(SliceTuple, MixedTupleSliceFull) {
    using sliced = SliceTuple<0, 3, std::tuple<int, bool, float>>;
    static_assert(std::tuple_size_v<sliced> == 3);
    static_assert(std::is_same_v<std::tuple_element_t<0, sliced>, int>);
    static_assert(std::is_same_v<std::tuple_element_t<1, sliced>, bool>);
    static_assert(std::is_same_v<std::tuple_element_t<2, sliced>, float>);
}

TEST(SliceTuple, MixedTupleSliceLowPart) {
    using sliced = SliceTuple<0, 2, std::tuple<int, bool, float>>;
    static_assert(std::tuple_size_v<sliced> == 2);
    static_assert(std::is_same_v<std::tuple_element_t<0, sliced>, int>);
    static_assert(std::is_same_v<std::tuple_element_t<1, sliced>, bool>);
}

TEST(SliceTuple, MixedTupleSliceHighPart) {
    using sliced = SliceTuple<1, 2, std::tuple<int, bool, float>>;
    static_assert(std::tuple_size_v<sliced> == 2);
    static_assert(std::is_same_v<std::tuple_element_t<0, sliced>, bool>);
    static_assert(std::is_same_v<std::tuple_element_t<1, sliced>, float>);
}

static_assert(std::is_same_v<char*, CharArrayToCharPtr<char[2]>>);
static_assert(std::is_same_v<const char*, CharArrayToCharPtr<const char[2]>>);
static_assert(std::is_same_v<int, CharArrayToCharPtr<int>>);
static_assert(std::is_same_v<int[2], CharArrayToCharPtr<int[2]>>);

}  // namespace tint::traits
