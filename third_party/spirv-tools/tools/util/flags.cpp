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

#include "flags.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

namespace flags {

std::vector<std::string> positional_arguments;

namespace {

using token_t = const char*;
using token_iterator_t = token_t*;

// Extracts the flag name from a potential token.
// This function only looks for a '=', to split the flag name from the value for
// long-form flags. Returns the name of the flag, prefixed with the hyphen(s).
inline std::string get_flag_name(const std::string& flag, bool is_short_flag) {
  if (is_short_flag) {
    return flag;
  }

  size_t equal_index = flag.find('=');
  if (equal_index == std::string::npos) {
    return flag;
  }
  return flag.substr(0, equal_index);
}

// Parse a boolean flag. Returns `true` if the parsing succeeded, `false`
// otherwise.
bool parse_bool_flag(Flag<bool>& flag, bool is_short_flag,
                     const std::string& token) {
  if (is_short_flag) {
    flag.value() = true;
    return true;
  }

  const std::string raw_flag(token);
  size_t equal_index = raw_flag.find('=');
  if (equal_index == std::string::npos) {
    flag.value() = true;
    return true;
  }

  const std::string value = raw_flag.substr(equal_index + 1);
  if (value == "true") {
    flag.value() = true;
    return true;
  }

  if (value == "false") {
    flag.value() = false;
    return true;
  }

  return false;
}

// Parse a uint32_t flag value.
bool parse_flag_value(Flag<uint32_t>& flag, const std::string& value) {
  std::regex unsigned_pattern("^ *[0-9]+ *$");
  if (!std::regex_match(value, unsigned_pattern)) {
    std::cerr << "'" << value << "' is not a unsigned number." << std::endl;
    return false;
  }

  errno = 0;
  char* end_ptr = nullptr;
  const uint64_t number = strtoull(value.c_str(), &end_ptr, 10);
  if (end_ptr == nullptr || end_ptr != value.c_str() + value.size() ||
      errno == EINVAL) {
    std::cerr << "'" << value << "' is not a unsigned number." << std::endl;
    return false;
  }

  if (errno == ERANGE || number > static_cast<size_t>(UINT32_MAX)) {
    std::cerr << "'" << value << "' cannot be represented as a 32bit unsigned."
              << std::endl;
    return false;
  }

  flag.value() = static_cast<uint32_t>(number);
  return true;
}

// "Parse" a string flag value (assigns it, cannot fail).
bool parse_flag_value(Flag<std::string>& flag, const std::string& value) {
  flag.value() = value;
  return true;
}

// Parse a potential multi-token flag. Moves the iterator to the last flag's
// token if it's a multi-token flag. Returns `true` if the parsing succeeded.
// The iterator is moved to the last parsed token.
template <typename T>
bool parse_flag(Flag<T>& flag, bool is_short_flag, const char*** iterator) {
  const std::string raw_flag(**iterator);
  std::string raw_value;
  const size_t equal_index = raw_flag.find('=');

  if (is_short_flag || equal_index == std::string::npos) {
    if ((*iterator)[1] == nullptr) {
      return false;
    }

    // This is a bi-token flag. Moving iterator to the last parsed token.
    raw_value = (*iterator)[1];
    *iterator += 1;
  } else {
    // This is a mono-token flag, no need to move the iterator.
    raw_value = raw_flag.substr(equal_index + 1);
  }

  return parse_flag_value(flag, raw_value);
}

}  // namespace

// This is the function to expand if you want to support a new type.
bool FlagList::parse_flag_info(FlagInfo& info, token_iterator_t* iterator) {
  bool success = false;

  std::visit(
      [&](auto&& item) {
        using T = std::decay_t<decltype(item.get())>;
        if constexpr (std::is_same_v<T, Flag<bool>>) {
          success = parse_bool_flag(item.get(), info.is_short, **iterator);
        } else if constexpr (std::is_same_v<T, Flag<std::string>>) {
          success = parse_flag(item.get(), info.is_short, iterator);
        } else if constexpr (std::is_same_v<T, Flag<uint32_t>>) {
          success = parse_flag(item.get(), info.is_short, iterator);
        } else {
          static_assert(always_false_v<T>, "Unsupported flag type.");
        }
      },
      info.flag);

  return success;
}

bool FlagList::parse(token_t* argv) {
  flags::positional_arguments.clear();
  std::unordered_set<const FlagInfo*> parsed_flags;

  bool ignore_flags = false;
  for (const char** it = argv + 1; *it != nullptr; it++) {
    if (ignore_flags) {
      flags::positional_arguments.emplace_back(*it);
      continue;
    }

    // '--' alone is used to mark the end of the flags.
    if (std::strcmp(*it, "--") == 0) {
      ignore_flags = true;
      continue;
    }

    // '-' alone is not a flag, but often used to say 'stdin'.
    if (std::strcmp(*it, "-") == 0) {
      flags::positional_arguments.emplace_back(*it);
      continue;
    }

    const std::string raw_flag(*it);
    if (raw_flag.size() == 0) {
      continue;
    }

    if (raw_flag[0] != '-') {
      flags::positional_arguments.emplace_back(*it);
      continue;
    }

    // Only case left: flags (long and shorts).
    if (raw_flag.size() < 2) {
      std::cerr << "Unknown flag " << raw_flag << std::endl;
      return false;
    }
    const bool is_short_flag = std::strncmp(*it, "--", 2) != 0;
    const std::string flag_name = get_flag_name(raw_flag, is_short_flag);

    auto needle = std::find_if(
        get_flags().begin(), get_flags().end(),
        [&flag_name](const auto& item) { return item.name == flag_name; });
    if (needle == get_flags().end()) {
      std::cerr << "Unknown flag " << flag_name << std::endl;
      return false;
    }

    if (parsed_flags.count(&*needle) != 0) {
      std::cerr << "The flag " << flag_name << " was specified multiple times."
                << std::endl;
      return false;
    }
    parsed_flags.insert(&*needle);

    if (!parse_flag_info(*needle, &it)) {
      std::cerr << "Invalid usage for flag " << flag_name << std::endl;
      return false;
    }
  }

  // Check that we parsed all required flags.
  for (const auto& flag : get_flags()) {
    if (!flag.required) {
      continue;
    }

    if (parsed_flags.count(&flag) == 0) {
      std::cerr << "Missing required flag " << flag.name << std::endl;
      return false;
    }
  }

  return true;
}

// Just the public wrapper around the parse function.
bool Parse(const char** argv) { return FlagList::parse(argv); }

}  // namespace flags
