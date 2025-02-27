// Copyright 2024 The Abseil Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "absl/debugging/internal/demangle_rust.h"

#include <cstddef>
#include <string>

#include "gtest/gtest.h"
#include "absl/base/config.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace debugging_internal {
namespace {

// If DemangleRustSymbolEncoding(mangled, <buffer with room for buffer_size
// chars>, buffer_size) returns true and seems not to have overrun its output
// buffer, returns the string written by DemangleRustSymbolEncoding; otherwise
// returns an error message.
std::string ResultOfDemangling(const char* mangled, std::size_t buffer_size) {
  // Fill the buffer with something other than NUL so we test whether Demangle
  // appends trailing NUL as expected.
  std::string buffer(buffer_size + 1, '~');
  constexpr char kCanaryCharacter = 0x7f;  // arbitrary unlikely value
  buffer[buffer_size] = kCanaryCharacter;
  if (!DemangleRustSymbolEncoding(mangled, &buffer[0], buffer_size)) {
    return "Failed parse";
  }
  if (buffer[buffer_size] != kCanaryCharacter) {
    return "Buffer overrun by output: " + buffer.substr(0, buffer_size + 1)
        + "...";
  }
  return buffer.data();  // Not buffer itself: this trims trailing padding.
}

// Tests that DemangleRustSymbolEncoding converts mangled into plaintext given
// enough output buffer space but returns false and avoids overrunning a buffer
// that is one byte too short.
//
// The lambda wrapping allows ASSERT_EQ to branch out the first time an
// expectation is not satisfied, preventing redundant errors for the same bug.
//
// We test first with excess space so that if the algorithm just computes the
// wrong answer, it will be clear from the error log that the bounds checks are
// unlikely to be the code at fault.
#define EXPECT_DEMANGLING(mangled, plaintext) \
  do { \
    [] { \
      constexpr std::size_t plenty_of_space = sizeof(plaintext) + 128; \
      constexpr std::size_t just_enough_space = sizeof(plaintext); \
      constexpr std::size_t one_byte_too_few = sizeof(plaintext) - 1; \
      const char* expected_plaintext = plaintext; \
      const char* expected_error = "Failed parse"; \
      ASSERT_EQ(ResultOfDemangling(mangled, plenty_of_space), \
                expected_plaintext); \
      ASSERT_EQ(ResultOfDemangling(mangled, just_enough_space), \
                expected_plaintext); \
      ASSERT_EQ(ResultOfDemangling(mangled, one_byte_too_few), \
                expected_error); \
    }(); \
  } while (0)

// Tests that DemangleRustSymbolEncoding rejects the given input (typically, a
// truncation of a real Rust symbol name).
#define EXPECT_DEMANGLING_FAILS(mangled) \
    do { \
      constexpr std::size_t plenty_of_space = 1024; \
      const char* expected_error = "Failed parse"; \
      EXPECT_EQ(ResultOfDemangling(mangled, plenty_of_space), expected_error); \
    } while (0)

// Piping grep -C 1 _R demangle_test.cc into your favorite c++filt
// implementation allows you to verify that the goldens below are reasonable.

TEST(DemangleRust, EmptyDemangling) {
  EXPECT_TRUE(DemangleRustSymbolEncoding("_RC0", nullptr, 0));
}

TEST(DemangleRust, FunctionAtCrateLevel) {
  EXPECT_DEMANGLING("_RNvC10crate_name9func_name", "crate_name::func_name");
  EXPECT_DEMANGLING(
      "_RNvCs09azAZ_10crate_name9func_name", "crate_name::func_name");
}

TEST(DemangleRust, TruncationsOfFunctionAtCrateLevel) {
  EXPECT_DEMANGLING_FAILS("_R");
  EXPECT_DEMANGLING_FAILS("_RN");
  EXPECT_DEMANGLING_FAILS("_RNvC");
  EXPECT_DEMANGLING_FAILS("_RNvC10");
  EXPECT_DEMANGLING_FAILS("_RNvC10crate_nam");
  EXPECT_DEMANGLING_FAILS("_RNvC10crate_name");
  EXPECT_DEMANGLING_FAILS("_RNvC10crate_name9");
  EXPECT_DEMANGLING_FAILS("_RNvC10crate_name9func_nam");
  EXPECT_DEMANGLING_FAILS("_RNvCs");
  EXPECT_DEMANGLING_FAILS("_RNvCs09azAZ");
  EXPECT_DEMANGLING_FAILS("_RNvCs09azAZ_");
}

TEST(DemangleRust, VendorSuffixes) {
  EXPECT_DEMANGLING("_RNvC10crate_name9func_name.!@#", "crate_name::func_name");
  EXPECT_DEMANGLING("_RNvC10crate_name9func_name$!@#", "crate_name::func_name");
}

TEST(DemangleRust, UnicodeIdentifiers) {
  EXPECT_DEMANGLING("_RNvC7ice_cap17Eyjafjallajökull",
                    "ice_cap::Eyjafjallajökull");
  EXPECT_DEMANGLING("_RNvC7ice_caps_u19Eyjafjallajkull_jtb",
                    "ice_cap::{Punycode Eyjafjallajkull_jtb}");
}

TEST(DemangleRust, FunctionInModule) {
  EXPECT_DEMANGLING("_RNvNtCs09azAZ_10crate_name11module_name9func_name",
                    "crate_name::module_name::func_name");
}

TEST(DemangleRust, FunctionInFunction) {
  EXPECT_DEMANGLING(
      "_RNvNvCs09azAZ_10crate_name15outer_func_name15inner_func_name",
      "crate_name::outer_func_name::inner_func_name");
}

TEST(DemangleRust, ClosureInFunction) {
  EXPECT_DEMANGLING(
      "_RNCNvCs09azAZ_10crate_name9func_name0",
      "crate_name::func_name::{closure#0}");
  EXPECT_DEMANGLING(
      "_RNCNvCs09azAZ_10crate_name9func_name0Cs123_12client_crate",
      "crate_name::func_name::{closure#0}");
}

TEST(DemangleRust, ClosureNumbering) {
  EXPECT_DEMANGLING(
      "_RNCNvCs09azAZ_10crate_name9func_names_0Cs123_12client_crate",
      "crate_name::func_name::{closure#1}");
  EXPECT_DEMANGLING(
      "_RNCNvCs09azAZ_10crate_name9func_names0_0Cs123_12client_crate",
      "crate_name::func_name::{closure#2}");
  EXPECT_DEMANGLING(
      "_RNCNvCs09azAZ_10crate_name9func_names9_0Cs123_12client_crate",
      "crate_name::func_name::{closure#11}");
  EXPECT_DEMANGLING(
      "_RNCNvCs09azAZ_10crate_name9func_namesa_0Cs123_12client_crate",
      "crate_name::func_name::{closure#12}");
  EXPECT_DEMANGLING(
      "_RNCNvCs09azAZ_10crate_name9func_namesz_0Cs123_12client_crate",
      "crate_name::func_name::{closure#37}");
  EXPECT_DEMANGLING(
      "_RNCNvCs09azAZ_10crate_name9func_namesA_0Cs123_12client_crate",
      "crate_name::func_name::{closure#38}");
  EXPECT_DEMANGLING(
      "_RNCNvCs09azAZ_10crate_name9func_namesZ_0Cs123_12client_crate",
      "crate_name::func_name::{closure#63}");
  EXPECT_DEMANGLING(
      "_RNCNvCs09azAZ_10crate_name9func_names10_0Cs123_12client_crate",
      "crate_name::func_name::{closure#64}");
  EXPECT_DEMANGLING(
      "_RNCNvCs09azAZ_10crate_name9func_namesg6_0Cs123_12client_crate",
      "crate_name::func_name::{closure#1000}");
}

TEST(DemangleRust, ClosureNumberOverflowingInt) {
  EXPECT_DEMANGLING(
      "_RNCNvCs09azAZ_10crate_name9func_names1234567_0Cs123_12client_crate",
      "crate_name::func_name::{closure#?}");
}

TEST(DemangleRust, UnexpectedlyNamedClosure) {
  EXPECT_DEMANGLING(
      "_RNCNvCs123_10crate_name9func_name12closure_nameCs456_12client_crate",
      "crate_name::func_name::{closure:closure_name#0}");
  EXPECT_DEMANGLING(
      "_RNCNvCs123_10crate_name9func_names2_12closure_nameCs456_12client_crate",
      "crate_name::func_name::{closure:closure_name#4}");
}

TEST(DemangleRust, ItemNestedInsideClosure) {
  EXPECT_DEMANGLING(
      "_RNvNCNvCs123_10crate_name9func_name015inner_func_nameCs_12client_crate",
      "crate_name::func_name::{closure#0}::inner_func_name");
}

TEST(DemangleRust, Shim) {
  EXPECT_DEMANGLING(
      "_RNSNvCs123_10crate_name9func_name6vtableCs456_12client_crate",
      "crate_name::func_name::{shim:vtable#0}");
}

TEST(DemangleRust, UnknownUppercaseNamespace) {
  EXPECT_DEMANGLING(
      "_RNXNvCs123_10crate_name9func_name14mystery_objectCs456_12client_crate",
      "crate_name::func_name::{X:mystery_object#0}");
}

TEST(DemangleRust, NestedUppercaseNamespaces) {
  EXPECT_DEMANGLING(
      "_RNCNXNYCs123_10crate_names0_1ys1_1xs2_0Cs456_12client_crate",
      "crate_name::{Y:y#2}::{X:x#3}::{closure#4}");
}

TEST(DemangleRust, TraitDefinition) {
  EXPECT_DEMANGLING(
      "_RNvYNtC7crate_a9my_structNtC7crate_b8my_trait1f",
      "<crate_a::my_struct as crate_b::my_trait>::f");
}

TEST(DemangleRust, BasicTypeNames) {
  EXPECT_DEMANGLING("_RNvYaNtC1c1t1f", "<i8 as c::t>::f");
  EXPECT_DEMANGLING("_RNvYbNtC1c1t1f", "<bool as c::t>::f");
  EXPECT_DEMANGLING("_RNvYcNtC1c1t1f", "<char as c::t>::f");
  EXPECT_DEMANGLING("_RNvYdNtC1c1t1f", "<f64 as c::t>::f");
  EXPECT_DEMANGLING("_RNvYeNtC1c1t1f", "<str as c::t>::f");
  EXPECT_DEMANGLING("_RNvYfNtC1c1t1f", "<f32 as c::t>::f");
  EXPECT_DEMANGLING("_RNvYhNtC1c1t1f", "<u8 as c::t>::f");
  EXPECT_DEMANGLING("_RNvYiNtC1c1t1f", "<isize as c::t>::f");
  EXPECT_DEMANGLING("_RNvYjNtC1c1t1f", "<usize as c::t>::f");
  EXPECT_DEMANGLING("_RNvYlNtC1c1t1f", "<i32 as c::t>::f");
  EXPECT_DEMANGLING("_RNvYmNtC1c1t1f", "<u32 as c::t>::f");
  EXPECT_DEMANGLING("_RNvYnNtC1c1t1f", "<i128 as c::t>::f");
  EXPECT_DEMANGLING("_RNvYoNtC1c1t1f", "<u128 as c::t>::f");
  EXPECT_DEMANGLING("_RNvYpNtC1c1t1f", "<_ as c::t>::f");
  EXPECT_DEMANGLING("_RNvYsNtC1c1t1f", "<i16 as c::t>::f");
  EXPECT_DEMANGLING("_RNvYtNtC1c1t1f", "<u16 as c::t>::f");
  EXPECT_DEMANGLING("_RNvYuNtC1c1t1f", "<() as c::t>::f");
  EXPECT_DEMANGLING("_RNvYvNtC1c1t1f", "<... as c::t>::f");
  EXPECT_DEMANGLING("_RNvYxNtC1c1t1f", "<i64 as c::t>::f");
  EXPECT_DEMANGLING("_RNvYyNtC1c1t1f", "<u64 as c::t>::f");
  EXPECT_DEMANGLING("_RNvYzNtC1c1t1f", "<! as c::t>::f");

  EXPECT_DEMANGLING_FAILS("_RNvYkNtC1c1t1f");
}

TEST(DemangleRust, SliceTypes) {
  EXPECT_DEMANGLING("_RNvYSlNtC1c1t1f", "<[i32] as c::t>::f");
  EXPECT_DEMANGLING("_RNvYSNtC1d1sNtC1c1t1f", "<[d::s] as c::t>::f");
}

TEST(DemangleRust, ImmutableReferenceTypes) {
  EXPECT_DEMANGLING("_RNvYRlNtC1c1t1f", "<&i32 as c::t>::f");
  EXPECT_DEMANGLING("_RNvYRNtC1d1sNtC1c1t1f", "<&d::s as c::t>::f");
}

TEST(DemangleRust, MutableReferenceTypes) {
  EXPECT_DEMANGLING("_RNvYQlNtC1c1t1f", "<&mut i32 as c::t>::f");
  EXPECT_DEMANGLING("_RNvYQNtC1d1sNtC1c1t1f", "<&mut d::s as c::t>::f");
}

TEST(DemangleRust, ConstantRawPointerTypes) {
  EXPECT_DEMANGLING("_RNvYPlNtC1c1t1f", "<*const i32 as c::t>::f");
  EXPECT_DEMANGLING("_RNvYPNtC1d1sNtC1c1t1f", "<*const d::s as c::t>::f");
}

TEST(DemangleRust, MutableRawPointerTypes) {
  EXPECT_DEMANGLING("_RNvYOlNtC1c1t1f", "<*mut i32 as c::t>::f");
  EXPECT_DEMANGLING("_RNvYONtC1d1sNtC1c1t1f", "<*mut d::s as c::t>::f");
}

TEST(DemangleRust, TupleLength0) {
  EXPECT_DEMANGLING("_RNvYTENtC1c1t1f", "<() as c::t>::f");
}

TEST(DemangleRust, TupleLength1) {
  EXPECT_DEMANGLING("_RNvYTlENtC1c1t1f", "<(i32,) as c::t>::f");
  EXPECT_DEMANGLING("_RNvYTNtC1d1sENtC1c1t1f", "<(d::s,) as c::t>::f");
}

TEST(DemangleRust, TupleLength2) {
  EXPECT_DEMANGLING("_RNvYTlmENtC1c1t1f", "<(i32, u32) as c::t>::f");
  EXPECT_DEMANGLING("_RNvYTNtC1d1xNtC1e1yENtC1c1t1f",
                    "<(d::x, e::y) as c::t>::f");
}

TEST(DemangleRust, TupleLength3) {
  EXPECT_DEMANGLING("_RNvYTlmnENtC1c1t1f", "<(i32, u32, i128) as c::t>::f");
  EXPECT_DEMANGLING("_RNvYTNtC1d1xNtC1e1yNtC1f1zENtC1c1t1f",
                    "<(d::x, e::y, f::z) as c::t>::f");
}

TEST(DemangleRust, LongerTuplesAbbreviated) {
  EXPECT_DEMANGLING("_RNvYTlmnoENtC1c1t1f",
                    "<(i32, u32, i128, ...) as c::t>::f");
  EXPECT_DEMANGLING("_RNvYTlmnNtC1d1xNtC1e1yENtC1c1t1f",
                    "<(i32, u32, i128, ...) as c::t>::f");
}

TEST(DemangleRust, PathBackrefToCrate) {
  EXPECT_DEMANGLING("_RNvYNtC8my_crate9my_structNtB4_8my_trait1f",
                    "<my_crate::my_struct as my_crate::my_trait>::f");
}

TEST(DemangleRust, PathBackrefToNestedPath) {
  EXPECT_DEMANGLING("_RNvYNtNtC1c1m1sNtB4_1t1f", "<c::m::s as c::m::t>::f");
}

TEST(DemangleRust, PathBackrefAsInstantiatingCrate) {
  EXPECT_DEMANGLING("_RNCNvC8my_crate7my_func0B3_",
                    "my_crate::my_func::{closure#0}");
}

TEST(DemangleRust, TypeBackrefsNestedInTuple) {
  EXPECT_DEMANGLING("_RNvYTTRlB4_ERB3_ENtC1c1t1f",
                    "<((&i32, &i32), &(&i32, &i32)) as c::t>::f");
}

TEST(DemangleRust, NoInfiniteLoopOnBackrefToTheWhole) {
  EXPECT_DEMANGLING_FAILS("_RB_");
  EXPECT_DEMANGLING_FAILS("_RNvB_1sNtC1c1t1f");
}

TEST(DemangleRust, NoCrashOnForwardBackref) {
  EXPECT_DEMANGLING_FAILS("_RB0_");
  EXPECT_DEMANGLING_FAILS("_RB1_");
  EXPECT_DEMANGLING_FAILS("_RB2_");
  EXPECT_DEMANGLING_FAILS("_RB3_");
  EXPECT_DEMANGLING_FAILS("_RB4_");
}

TEST(DemangleRust, PathBackrefsDoNotRecurseDuringSilence) {
  // B_ points at the value f (the whole mangling), so the cycle would lead to
  // parse failure if the parser tried to parse what was pointed to.
  EXPECT_DEMANGLING("_RNvYTlmnNtB_1sENtC1c1t1f",
                    "<(i32, u32, i128, ...) as c::t>::f");
}

TEST(DemangleRust, TypeBackrefsDoNotRecurseDuringSilence) {
  // B2_ points at the tuple type, likewise making a cycle that the parser
  // avoids following.
  EXPECT_DEMANGLING("_RNvYTlmnB2_ENtC1c1t1f",
                    "<(i32, u32, i128, ...) as c::t>::f");
}

TEST(DemangleRust, ReturnFromBackrefToInputPosition256) {
  // Show that we can resume at input positions that don't fit into a byte.
  EXPECT_DEMANGLING("_RNvYNtC1c238very_long_type_"
                    "ABCDEFGHIJabcdefghijABCDEFGHIJabcdefghij"
                    "ABCDEFGHIJabcdefghijABCDEFGHIJabcdefghij"
                    "ABCDEFGHIJabcdefghijABCDEFGHIJabcdefghij"
                    "ABCDEFGHIJabcdefghijABCDEFGHIJabcdefghij"
                    "ABCDEFGHIJabcdefghijABCDEFGHIJabcdefghij"
                    "ABCDEFGHIJabcdefghijABC"
                    "NtB4_1t1f",
                    "<c::very_long_type_"
                    "ABCDEFGHIJabcdefghijABCDEFGHIJabcdefghij"
                    "ABCDEFGHIJabcdefghijABCDEFGHIJabcdefghij"
                    "ABCDEFGHIJabcdefghijABCDEFGHIJabcdefghij"
                    "ABCDEFGHIJabcdefghijABCDEFGHIJabcdefghij"
                    "ABCDEFGHIJabcdefghijABCDEFGHIJabcdefghij"
                    "ABCDEFGHIJabcdefghijABC"
                    " as c::t>::f");
}

}  // namespace
}  // namespace debugging_internal
ABSL_NAMESPACE_END
}  // namespace absl
