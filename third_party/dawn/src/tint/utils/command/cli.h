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

#ifndef SRC_TINT_UTILS_COMMAND_CLI_H_
#define SRC_TINT_UTILS_COMMAND_CLI_H_

#include <deque>
#include <optional>
#include <string>
#include <utility>

#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/memory/block_allocator.h"
#include "src/tint/utils/result.h"
#include "src/tint/utils/strconv/parse_num.h"
#include "src/tint/utils/text/string.h"

namespace tint::cli {

/// Alias is a fluent-constructor helper for Options
struct Alias {
    /// The alias to apply to an Option
    std::string value;

    /// @param option the option to apply the alias to
    template <typename T>
    void Apply(T& option) {
        option.alias = value;
    }
};

/// ShortName is a fluent-constructor helper for Options
struct ShortName {
    /// The short-name to apply to an Option
    std::string value;

    /// @param option the option to apply the short name to
    template <typename T>
    void Apply(T& option) {
        option.short_name = value;
    }
};

/// Parameter is a fluent-constructor helper for Options
struct Parameter {
    /// The parameter name to apply to an Option
    std::string value;

    /// @param option the option to apply the parameter name to
    template <typename T>
    void Apply(T& option) {
        option.parameter = value;
    }
};

/// Default is a fluent-constructor helper for Options
template <typename T>
struct Default {
    /// The default value to apply to an Option
    T value;

    /// @param option the option to apply the default value to
    template <typename O>
    void Apply(O& option) {
        option.default_value = value;
    }
};

/// Deduction guide for Default
template <typename T>
Default(T) -> Default<T>;

/// Option is the base class for all command line options
class Option {
  public:
    /// An alias to std::string, used to hold error messages.
    using Error = std::string;

    /// Constructor
    Option();

    /// Destructor
    virtual ~Option();

    /// @return the name of the option, without any leading hyphens.
    /// Example: 'help'
    virtual std::string Name() const = 0;

    /// @return the alias name of the option, without any leading hyphens. (optional)
    /// Example: 'flag'
    virtual std::string Alias() const = 0;

    /// @return the shorter name of the option, without any leading hyphens. (optional)
    /// Example: 'h'
    virtual std::string ShortName() const = 0;

    /// @return a string describing the parameter that the option expects.
    /// Empty represents no expected parameter.
    virtual std::string Parameter() const = 0;

    /// @return a description of the option.
    /// Example: 'shows this message'
    virtual std::string Description() const = 0;

    /// @return the default value of the option, or an empty string if there is no default value.
    virtual std::string DefaultValue() const = 0;

    /// Sets the option value to the default (called before arguments are parsed)
    virtual void SetDefault() = 0;

    /// Parses the option's arguments from the list of command line arguments, removing the consumed
    /// arguments before returning. @p arguments will have already had the option's name consumed
    /// before calling.
    /// @param arguments the queue of unprocessed arguments. Parse() may take from the front of @p
    /// arguments.
    /// @return empty Error if successfully parsed, otherwise an error string.
    virtual Error Parse(std::deque<std::string_view>& arguments) = 0;

  protected:
    /// An empty string, used to represent no-error.
    static constexpr const char* Success = "";

    /// @param expected the expected value(s) for the option
    /// @return an Error message for a missing argument
    Error ErrMissingArgument(std::string expected) const {
        Error err = "missing value for option '--" + Name() + "'";
        if (!expected.empty()) {
            err += "Expected: " + expected;
        }
        return err;
    }

    /// @param got the argument value provided
    /// @param reason the reason the argument is invalid (optional)
    /// @return an Error message for an invalid argument
    Error ErrInvalidArgument(std::string_view got, std::string reason) const {
        Error err = "invalid value '" + std::string(got) + "' for option '--" + Name() + "'";
        if (!reason.empty()) {
            err += "\n" + reason;
        }
        return err;
    }

  private:
    Option(const Option&) = delete;
    Option& operator=(const Option&) = delete;
    Option(Option&&) = delete;
    Option& operator=(Option&&) = delete;
};

/// Options for OptionSet::Parse
struct ParseOptions {
    /// If true, then unknown flags will be ignored instead of treated as an error
    bool ignore_unknown = false;
};

/// OptionSet is a set of Options, which can parse the command line arguments.
class OptionSet {
  public:
    /// Unconsumed is a list of unconsumed command line arguments
    using Unconsumed = Vector<std::string_view, 8>;

    /// Constructs and returns a new Option to be owned by the OptionSet
    /// @tparam T the Option type
    /// @tparam ARGS the constructor argument types
    /// @param args the constructor arguments
    /// @return the constructed Option
    template <typename T, typename... ARGS>
    T& Add(ARGS&&... args) {
        return *options.Create<T>(std::forward<ARGS>(args)...);
    }

    /// Prints to @p out the description of all the command line options.
    /// @param out the output stream
    /// @param show_equals_form if true, show the '--option=value' syntax
    /// instead of '--option value'
    void ShowHelp(std::ostream& out, bool show_equals_form = false);

    /// Parses all the options in @p options.
    /// @param arguments the command line arguments, excluding the initial executable name
    /// @param parse_options the optional parser options
    /// @return a Result holding a list of arguments that were not consumed as options
    Result<Unconsumed> Parse(VectorRef<std::string_view> arguments,
                             const ParseOptions& parse_options = {});

  private:
    /// The list of options to parse
    BlockAllocator<Option, 1024> options;
};

/// ValueOption is an option that accepts a single value
template <typename T>
class ValueOption : public Option {
    static constexpr bool is_bool = std::is_same_v<T, bool>;
    static constexpr bool is_number =
        !is_bool && (std::is_integral_v<T> || std::is_floating_point_v<T>);
    static constexpr bool is_string = std::is_same_v<T, std::string>;
    static_assert(is_bool || is_number || is_string, "unsupported data type");

  public:
    /// The name of the option, without any leading hyphens.
    std::string name;
    /// The alias name of the option, without any leading hyphens.
    std::string alias;
    /// The shorter name of the option, without any leading hyphens.
    std::string short_name;
    /// A description of the option.
    std::string description;
    /// The default value.
    std::optional<T> default_value;
    /// The option value. Populated with Parse().
    std::optional<T> value;
    /// A string describing the name of the option's value.
    std::string parameter = "value";

    /// Constructor
    ValueOption() = default;

    /// Constructor
    /// @param option_name the option name
    /// @param option_description the option description
    /// @param settings a number of fluent-constructor values that configure the option
    /// @see ShortName, Parameter, Default
    template <typename... SETTINGS>
    ValueOption(std::string option_name, std::string option_description, SETTINGS&&... settings)
        : name(std::move(option_name)), description(std::move(option_description)) {
        (settings.Apply(*this), ...);
    }

    std::string Name() const override { return name; }

    std::string Alias() const override { return alias; }

    std::string ShortName() const override { return short_name; }

    std::string Parameter() const override { return parameter; }

    std::string Description() const override { return description; }

    std::string DefaultValue() const override {
        return default_value.has_value() ? ToString(*default_value) : "";
    }

    void SetDefault() override { value = default_value; }

    Error Parse(std::deque<std::string_view>& arguments) override {
        TINT_BEGIN_DISABLE_WARNING(UNREACHABLE_CODE);

        if (arguments.empty()) {
            if constexpr (is_bool) {
                // Treat as flag (--blah)
                value = true;
                return Success;
            } else {
                return ErrMissingArgument(parameter);
            }
        }

        auto arg = arguments.front();

        if constexpr (is_number) {
            auto result = strconv::ParseNumber<T>(arg);
            if (result == tint::Success) {
                value = result.Get();
                arguments.pop_front();
                return Success;
            }
            if (result.Failure() == strconv::ParseNumberError::kResultOutOfRange) {
                return ErrInvalidArgument(arg, "value out of range");
            }
            return ErrInvalidArgument(arg, "failed to parse value");
        } else if constexpr (is_string) {
            value = arg;
            arguments.pop_front();
            return Success;
        } else if constexpr (is_bool) {
            if (arg == "true") {
                value = true;
                arguments.pop_front();
                return Success;
            }
            if (arg == "false") {
                value = false;
                arguments.pop_front();
                return Success;
            }
            // Next argument is assumed to be another option, or unconsumed argument.
            // Treat as flag (--blah)
            value = true;
            return Success;
        }

        TINT_END_DISABLE_WARNING(UNREACHABLE_CODE);
    }
};

/// BoolOption is an alias to ValueOption<bool>
using BoolOption = ValueOption<bool>;

/// StringOption is an alias to ValueOption<std::string>
using StringOption = ValueOption<std::string>;

/// EnumName is a pair of enum value and name.
/// @tparam ENUM the enum type
template <typename ENUM>
struct EnumName {
    /// Constructor
    EnumName() = default;

    /// Constructor
    /// @param v the enum value
    /// @param n the name of the enum value
    EnumName(ENUM v, std::string n) : value(v), name(std::move(n)) {}

    /// the enum value
    ENUM value;
    /// the name of the enum value
    std::string name;
};

/// Deduction guide for EnumName
template <typename ENUM>
EnumName(ENUM, std::string) -> EnumName<ENUM>;

/// EnumOption is an option that accepts an enumerator of values
template <typename ENUM>
class EnumOption : public Option {
  public:
    /// The name of the option, without any leading hyphens.
    std::string name;
    /// The alias name of the option, without any leading hyphens.
    std::string alias;
    /// The shorter name of the option, without any leading hyphens.
    std::string short_name;
    /// A description of the option.
    std::string description;
    /// The enum options as a pair of enum value to name
    Vector<EnumName<ENUM>, 8> enum_names;
    /// The default value.
    std::optional<ENUM> default_value;
    /// The option value. Populated with Parse().
    std::optional<ENUM> value;

    /// Constructor
    EnumOption() = default;

    /// Constructor
    /// @param option_name the option name
    /// @param option_description the option description
    /// @param names The enum options as a pair of enum value to name
    /// @param settings a number of fluent-constructor values that configure the option
    /// @see ShortName, Parameter, Default
    template <typename... SETTINGS>
    EnumOption(std::string option_name,
               std::string option_description,
               VectorRef<EnumName<ENUM>> names,
               SETTINGS&&... settings)
        : name(std::move(option_name)),
          description(std::move(option_description)),
          enum_names(std::move(names)) {
        (settings.Apply(*this), ...);
    }

    std::string Name() const override { return name; }

    std::string ShortName() const override { return short_name; }

    std::string Alias() const override { return alias; }

    std::string Parameter() const override { return PossibleValues("|"); }

    std::string Description() const override { return description; }

    std::string DefaultValue() const override {
        for (auto& enum_name : enum_names) {
            if (enum_name.value == default_value) {
                return enum_name.name;
            }
        }
        return "";
    }

    void SetDefault() override { value = default_value; }

    Error Parse(std::deque<std::string_view>& arguments) override {
        if (arguments.empty()) {
            return ErrMissingArgument("one of: " + PossibleValues(", "));
        }
        auto& arg = arguments.front();
        for (auto& enum_name : enum_names) {
            if (enum_name.name == arg) {
                value = enum_name.value;
                arguments.pop_front();
                return Success;
            }
        }
        return ErrInvalidArgument(arg, "Must be one of: " + PossibleValues(", "));
    }

    /// @param delimiter the delimiter between each enum option
    /// @returns the accepted enum names delimited with @p delimiter
    std::string PossibleValues(std::string delimiter) const {
        std::string out;
        for (auto& enum_name : enum_names) {
            if (!out.empty()) {
                out += delimiter;
            }
            out += enum_name.name;
        }
        return out;
    }
};

}  // namespace tint::cli

#endif  // SRC_TINT_UTILS_COMMAND_CLI_H_
