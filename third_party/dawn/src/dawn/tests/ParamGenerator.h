// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_TESTS_PARAMGENERATOR_H_
#define SRC_DAWN_TESTS_PARAMGENERATOR_H_

#include <array>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "dawn/common/Preprocessor.h"

#include "gtest/gtest.h"

namespace dawn {
namespace detail {

template <typename T>
struct IsOptional {
    static constexpr bool value = false;
};
template <typename T>
struct IsOptional<std::optional<T>> {
    static constexpr bool value = true;
};
template <typename T>
struct IsOptional<const std::optional<T>> {
    static constexpr bool value = true;
};

}  // namespace detail

template <typename T>
void PrintParamStructField(std::ostream& o, const T& param, const char* type) {
    if constexpr (detail::IsOptional<T>::value) {
        if (param) {
            o << "_" << type << "_" << *param;
        }
    } else {
        o << "_" << type << "_" << param;
    }
}

// Implementation for DAWN_TEST_PARAM_STRUCT to declare/print struct fields.
#define DAWN_TEST_PARAM_STRUCT_DECL_STRUCT_FIELD(Type) Type DAWN_PP_CONCATENATE(m, Type);
#define DAWN_TEST_PARAM_STRUCT_PRINT_STRUCT_FIELD(Type) \
    PrintParamStructField(o, static_cast<const Type&>(param.DAWN_PP_CONCATENATE(m, Type)), #Type);

// Usage: DAWN_TEST_PARAM_STRUCT_BASE(BaseParam, Foo, TypeA, TypeB, ...)
// Generate a test param struct called Foo which extends BaseParam and generated
// struct _Dawn_Foo. _Dawn_Foo has members of types TypeA, TypeB, etc. which are named mTypeA,
// mTypeB, etc. in the order they are placed in the macro argument list. Struct Foo should be
// constructed with an BaseParam as the first argument, followed by a list of values
// to initialize the base _Dawn_Foo struct.
// It is recommended to use alias declarations so that stringified types are more readable.
// Example:
//   using MyParam = unsigned int;
//   DAWN_TEST_PARAM_STRUCT_BASE(AdapterTestParam, FooParams, MyParam);
#define DAWN_TEST_PARAM_STRUCT_BASE(BaseStructName, StructName, ...)                               \
    struct DAWN_PP_CONCATENATE(_Dawn_, StructName) {                                               \
        DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH)(DAWN_TEST_PARAM_STRUCT_DECL_STRUCT_FIELD,  \
                                                        __VA_ARGS__))                              \
    };                                                                                             \
    inline std::ostream& operator<<(std::ostream& o,                                               \
                                    const DAWN_PP_CONCATENATE(_Dawn_, StructName) & param) {       \
        DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH)(DAWN_TEST_PARAM_STRUCT_PRINT_STRUCT_FIELD, \
                                                        __VA_ARGS__))                              \
        return o;                                                                                  \
    }                                                                                              \
    struct StructName : BaseStructName, DAWN_PP_CONCATENATE(_Dawn_, StructName) {                  \
        template <typename... Args>                                                                \
        StructName(const BaseStructName& param, Args&&... args)                                    \
            : BaseStructName(param), DAWN_PP_CONCATENATE(_Dawn_, StructName) {                     \
            std::forward<Args>(args)...                                                            \
        }                                                                                          \
        {}                                                                                         \
    };                                                                                             \
    inline std::ostream& operator<<(std::ostream& o, const StructName& param) {                    \
        o << static_cast<const BaseStructName&>(param);                                            \
        o << static_cast<const DAWN_PP_CONCATENATE(_Dawn_, StructName)&>(param);                   \
        return o;                                                                                  \
    }                                                                                              \
    static_assert(true, "require semicolon")

// Usage: DAWN_TEST_PARAM_STRUCT_TYPES(Foo, TypeA, TypeB, ...)
// Generate a test param struct called Foo which extends generated struct _Dawn_Foo. _Dawn_Foo has
// members of types TypeA, TypeB, etc. which are named mTypeA, mTypeB, etc. in the order they are
// placed in the macro argument list. Struct Foo should be constructed with a list of values to
// initialize the base _Dawn_Foo struct.
// It is recommended to use alias declarations so that stringified types are more readable.
// Example:
//   using MyParam = unsigned int;
//   DAWN_TEST_PARAM_STRUCT(FooParams, MyParam);
struct Placeholder {};
#define DAWN_TEST_PARAM_STRUCT_TYPES(StructName, ...)                                              \
    struct DAWN_PP_CONCATENATE(_Dawn_, StructName) {                                               \
        DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH)(DAWN_TEST_PARAM_STRUCT_DECL_STRUCT_FIELD,  \
                                                        __VA_ARGS__))                              \
    };                                                                                             \
    inline std::ostream& operator<<(std::ostream& o,                                               \
                                    const DAWN_PP_CONCATENATE(_Dawn_, StructName) & param) {       \
        DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH)(DAWN_TEST_PARAM_STRUCT_PRINT_STRUCT_FIELD, \
                                                        __VA_ARGS__))                              \
        return o;                                                                                  \
    }                                                                                              \
    struct StructName : DAWN_PP_CONCATENATE(_Dawn_, StructName) {                                  \
        template <typename... Args>                                                                \
        StructName(Args&&... args) : DAWN_PP_CONCATENATE(_Dawn_, StructName) {                     \
            std::forward<Args>(args)...                                                            \
        }                                                                                          \
        {}                                                                                         \
    };                                                                                             \
    inline std::ostream& operator<<(std::ostream& o, const StructName& param) {                    \
        o << static_cast<const DAWN_PP_CONCATENATE(_Dawn_, StructName)&>(param);                   \
        return o;                                                                                  \
    }                                                                                              \
    static_assert(true, "require semicolon")

template <typename ParamStruct>
std::string TestParamToString(const testing::TestParamInfo<ParamStruct>& info) {
    std::ostringstream output;
    output << info.param;
    auto result = output.str();
    if (result[0] == '_') {
        return result.substr(1);
    }
    return output.str();
}

// ParamStruct is a custom struct which ParamStruct will yield when iterating.
// The types Params... should be the same as the types passed to the constructor
// of ParamStruct.
// TODO: When std::span becomes available via c++20, use std::span over std::vector.
template <typename ParamStruct, typename... Params>
class ParamGenerator {
    using ParamTuple = std::tuple<std::vector<Params>...>;
    using Index = std::array<size_t, sizeof...(Params)>;

    static constexpr auto s_indexSequence = std::make_index_sequence<sizeof...(Params)>{};

    // Using an N-dimensional Index, extract params from ParamTuple and pass
    // them to the constructor of ParamStruct.
    template <size_t... Is>
    static ParamStruct GetParam(const ParamTuple& params,
                                const Index& index,
                                std::index_sequence<Is...>) {
        return ParamStruct(std::get<Is>(params)[std::get<Is>(index)]...);
    }

    // Get the last value index into a ParamTuple.
    template <size_t... Is>
    static Index GetLastIndex(const ParamTuple& params, std::index_sequence<Is...>) {
        return Index{std::get<Is>(params).size() - 1 ...};
    }

  public:
    using value_type = ParamStruct;

    explicit ParamGenerator(std::vector<Params>... params) : mParams(params...), mIsEmpty(false) {
        for (bool isEmpty : {params.empty()...}) {
            mIsEmpty |= isEmpty;
        }
    }

    class Iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = ParamStruct;
        using difference_type = size_t;
        using pointer = ParamStruct*;
        using reference = ParamStruct&;

        Iterator& operator++() {
            // Increment the Index by 1. If the i'th place reaches the maximum,
            // reset it to 0 and continue with the i+1'th place.
            for (int i = mIndex.size() - 1; i >= 0; --i) {
                if (mIndex[i] >= mLastIndex[i]) {
                    mIndex[i] = 0;
                } else {
                    mIndex[i]++;
                    return *this;
                }
            }

            // Set a marker that the iterator has reached the end.
            mEnd = true;
            return *this;
        }

        bool operator==(const Iterator& other) const {
            return mEnd == other.mEnd && mIndex == other.mIndex;
        }

        bool operator!=(const Iterator& other) const { return !(*this == other); }

        ParamStruct operator*() const { return GetParam(mParams, mIndex, s_indexSequence); }

      private:
        friend class ParamGenerator;

        Iterator(ParamTuple params, Index index)
            : mParams(params), mIndex(index), mLastIndex{GetLastIndex(params, s_indexSequence)} {}

        ParamTuple mParams;
        Index mIndex;
        Index mLastIndex;
        bool mEnd = false;
    };

    Iterator begin() const {
        if (mIsEmpty) {
            return end();
        }
        return Iterator(mParams, {});
    }

    Iterator end() const {
        Iterator iter(mParams, GetLastIndex(mParams, s_indexSequence));
        ++iter;
        return iter;
    }

  private:
    ParamTuple mParams;
    bool mIsEmpty;
};

}  // namespace dawn

#endif  // SRC_DAWN_TESTS_PARAMGENERATOR_H_
