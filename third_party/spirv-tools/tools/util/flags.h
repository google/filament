// Copyright (c) 2023 Google LLC.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDE_SPIRV_TOOLS_UTIL_FLAGS_HPP_
#define INCLUDE_SPIRV_TOOLS_UTIL_FLAGS_HPP_

#include <stdint.h>

#include <functional>
#include <string>
#include <variant>
#include <vector>

// This file provides some utils to define a command-line interface with
// required and optional flags.
//  - Flag order is not checked.
//  - Currently supported flag types: BOOLEAN, STRING
//  - As with most nix tools, using '--' in the command-line means all following
//  tokens will be considered positional
//    arguments.
//    Example: binary -g -- -g --some-other-flag
//      - the first `-g` is a flag.
//      - the second `-g` is not a flag.
//      - `--some-other-flag` is not a flag.
//  - Both long-form and short-form flags are supported, but boolean flags don't
//    support split boolean literals (short and long form).
//    Example:
//        -g              : allowed, sets g to true.
//        --my-flag       : allowed, sets --my-flag to true.
//        --my-flag=true  : allowed, sets --my-flag to true.
//        --my-flag true  : NOT allowed.
//        -g true         : NOT allowed.
//        --my-flag=TRUE  : NOT allowed.
//
//  - This implementation also supports string flags:
//        -o myfile.spv       : allowed, sets -o to `myfile.spv`.
//        --output=myfile.spv : allowed, sets --output to `myfile.spv`.
//        --output myfile.spv : allowd, sets --output to `myfile.spv`.
//
//    Note: then second token is NOT checked for hyphens.
//          --output -file.spv
//          flag name:  `output`
//          flag value: `-file.spv`
//
//  - This implementation generates flag at compile time. Meaning flag names
//  must be valid C++ identifiers.
//    However, flags are usually using hyphens for word separation. Hence
//    renaming is done behind the scenes. Example:
//      // Declaring a long-form flag.
//      FLAG_LONG_bool(my_flag, [...])
//
//      ->  in the code: flags::my_flag.value()
//      -> command-line: --my-flag
//
//  - The only additional lexing done is around '='. Otherwise token list is
//  processed as received in the Parse()
//    function.
//    Lexing the '=' sign:
//      - This is only done when parsing a long-form flag name.
//      - the first '=' found is considered a marker for long-form, splitting
//      the token into 2.
//        Example: --option=value=abc -> [--option, value=abc]
//
// In most cases, you want to define some flags, parse them, and query them.
// Here is a small code sample:
//
// ```c
//  // Defines a '-h' boolean flag for help printing, optional.
//  FLAG_SHORT_bool(h, /*default=*/ false, "Print the help.", false);
//  // Defines a '--my-flag' string flag, required.
//  FLAG_LONG_string(my_flag, /*default=*/ "", "A magic flag!", true);
//
//  int main(int argc, const char** argv) {
//    if (!flags::Parse(argv)) {
//      return -1;
//    }
//
//    if (flags::h.value()) {
//      printf("usage: my-bin --my-flag=<value>\n");
//      return 0;
//    }
//
//    printf("flag value: %s\n", flags::my_flag.value().c_str());
//    for (const std::string& arg : flags::positional_arguments) {
//      printf("arg: %s\n", arg.c_str());
//    }
//    return 0;
//  }
// ```c

// Those macros can be used to define flags.
// - They should be used in the global scope.
// - Underscores in the flag variable name are replaced with hyphens ('-').
//
// Example:
//  FLAG_SHORT_bool(my_flag, false, "some help", false);
//    -  in the code: flags::my_flag
//    - command line: --my-flag=true
//
#define FLAG_LONG_string(Name, Default, Required) \
  UTIL_FLAGS_FLAG_LONG(std::string, Name, Default, Required)
#define FLAG_LONG_bool(Name, Default, Required) \
  UTIL_FLAGS_FLAG_LONG(bool, Name, Default, Required)
#define FLAG_LONG_uint(Name, Default, Required) \
  UTIL_FLAGS_FLAG_LONG(uint32_t, Name, Default, Required)

#define FLAG_SHORT_string(Name, Default, Required) \
  UTIL_FLAGS_FLAG_SHORT(std::string, Name, Default, Required)
#define FLAG_SHORT_bool(Name, Default, Required) \
  UTIL_FLAGS_FLAG_SHORT(bool, Name, Default, Required)
#define FLAG_SHORT_uint(Name, Default, Required) \
  UTIL_FLAGS_FLAG_SHORT(uint32_t, Name, Default, Required)

namespace flags {

// Parse the command-line arguments, checking flags, and separating positional
// arguments from flags.
//
// * argv: the argv array received in the main function. This utility expects
// the last pointer to
//         be NULL, as it should if coming from the main() function.
//
// Returns `true` if the parsing succeeds, `false` otherwise.
bool Parse(const char** argv);

}  // namespace flags

// ===================== BEGIN NON-PUBLIC SECTION =============================
// All the code below belongs to the implementation, and there is no guaranteed
// around the API stability. Please do not use it directly.

// Defines the static variable holding the flag, allowing access like
// flags::my_flag.
// By creating the FlagRegistration object, the flag can be added to
// the global list.
// The final `extern` definition is ONLY useful for clang-format:
//  - if the macro doesn't ends with a semicolon, clang-format goes wild.
//  - cannot disable clang-format for those macros on clang < 16.
//    (https://github.com/llvm/llvm-project/issues/54522)
//  - cannot allow trailing semi (-Wextra-semi).
#define UTIL_FLAGS_FLAG(Type, Prefix, Name, Default, Required, IsShort)     \
  namespace flags {                                                         \
  Flag<Type> Name(Default);                                                 \
  namespace {                                                               \
  static FlagRegistration Name##_registration(Name, Prefix #Name, Required, \
                                              IsShort);                     \
  }                                                                         \
  }                                                                         \
  extern flags::Flag<Type> flags::Name

#define UTIL_FLAGS_FLAG_LONG(Type, Name, Default, Required) \
  UTIL_FLAGS_FLAG(Type, "--", Name, Default, Required, false)
#define UTIL_FLAGS_FLAG_SHORT(Type, Name, Default, Required) \
  UTIL_FLAGS_FLAG(Type, "-", Name, Default, Required, true)

namespace flags {

// Just a wrapper around the flag value.
template <typename T>
struct Flag {
 public:
  Flag(T&& default_value) : value_(default_value) {}
  Flag(Flag&& other) = delete;
  Flag(const Flag& other) = delete;

  const T& value() const { return value_; }
  T& value() { return value_; }

 private:
  T value_;
};

// To add support for new flag-types, this needs to be extended, and the visitor
// below.
using FlagType = std::variant<std::reference_wrapper<Flag<std::string>>,
                              std::reference_wrapper<Flag<bool>>,
                              std::reference_wrapper<Flag<uint32_t>>>;

template <class>
inline constexpr bool always_false_v = false;

extern std::vector<std::string> positional_arguments;

// Static class keeping track of the flags/arguments values.
class FlagList {
  struct FlagInfo {
    FlagInfo(FlagType&& flag_, std::string&& name_, bool required_,
             bool is_short_)
        : flag(std::move(flag_)),
          name(std::move(name_)),
          required(required_),
          is_short(is_short_) {}

    FlagType flag;
    std::string name;
    bool required;
    bool is_short;
  };

 public:
  template <typename T>
  static void register_flag(Flag<T>& flag, std::string&& name, bool required,
                            bool is_short) {
    get_flags().emplace_back(flag, std::move(name), required, is_short);
  }

  static bool parse(const char** argv);

#ifdef TESTING
  // Flags are supposed to be constant for the whole app execution, hence the
  // static storage. Gtest doesn't fork before running a test, meaning we have
  // to manually clear the context at teardown.
  static void reset() {
    get_flags().clear();
    positional_arguments.clear();
  }
#endif

 private:
  static std::vector<FlagInfo>& get_flags() {
    static std::vector<FlagInfo> flags;
    return flags;
  }

  static bool parse_flag_info(FlagInfo& info, const char*** iterator);
  static void print_usage(const char* binary_name,
                          const std::string& usage_format);
};

template <typename T>
struct FlagRegistration {
  FlagRegistration(Flag<T>& flag, std::string&& name, bool required,
                   bool is_short) {
    std::string fixed_name = name;
    for (auto& c : fixed_name) {
      if (c == '_') {
        c = '-';
      }
    }

    FlagList::register_flag(flag, std::move(fixed_name), required, is_short);
  }
};

// Explicit deduction guide to avoid `-Wctad-maybe-unsupported`.
template <typename T>
FlagRegistration(Flag<T>&, std::string&&, bool, bool) -> FlagRegistration<T>;

}  // namespace flags

#endif  // INCLUDE_SPIRV_TOOLS_UTIL_FLAGS_HPP_
