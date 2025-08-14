// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_UTILS_COMMANDLINEPARSER_H_
#define SRC_DAWN_UTILS_COMMANDLINEPARSER_H_

#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/types/span.h"  // TODO(343500108): Use std::span when we have C++20.
#include "dawn/common/Assert.h"
#include "dawn/common/NonMovable.h"

namespace dawn::utils {

// A helper class to parse command line arguments.
//
//   CommandLineParser parser;
//   auto& dryRun = parser.AddBool("dry-run", "fake operations").ShortName('d');
//   auto& input = parser.AddString("input", "the input file to process").ShortName('i');
//   auto& help = opts.AddHelp();
//
//   auto result = parser.Parse(argc, argv);
//   if (!result.success) {
//       std::cerr << result.errorMessage << "\n";
//       return 1;
//   }
//
//   if (help.GetValue()) {
//       std::cout << "Usage: " << argv[0] << " <options>\n\noptions\n";
//       parser.PrintHelp(std::cout);
//       return 0;
//   }
//
//   if (dryRun.GetValue() && input.IsSet()) {
//     doStuffWith(input.GetValue());
//   }
//   // ...
//
// Command line options can use short-form for boolean options "(-f") and use both spaces or = to
// separate the value for an option ("-f=foo" and  "-f foo").

// TODO(42241992): Considering supporting more types of options and command line parsing niceties.
// - Support "-" with a bunch of short names (like grep -rniI)
// - Support returning the part that hasn't been parsed at the end so that users can do something
// with it.
// - Support "--" being used to separate remaining args.
// - Support setting a default to show it in the help.

class CommandLineParser {
  public:
    // The base class for all options to let them interact with the parser.
    class OptionBase : NonMovable {
      public:
        OptionBase(absl::string_view name, absl::string_view desc);
        virtual ~OptionBase();

        const std::string& GetName() const;
        std::string GetShortName() const;
        const std::string& GetDescription() const;
        virtual std::string GetParameter() const;
        // Returns whether parser saw that option in the command line.
        bool IsSet() const;

        struct ParseResult {
            bool success;
            absl::Span<const absl::string_view> remainingArgs = {};
            std::string errorMessage = {};
        };
        ParseResult Parse(absl::Span<const absl::string_view> args);

      protected:
        virtual ParseResult ParseImpl(absl::Span<const absl::string_view> args) = 0;

        bool mSet = false;
        std::string mName;
        std::string mShortName;
        std::string mDescription;
        std::string mParameter;
    };

    // A CRTP wrapper around OptionBase to support the fluent setters better.
    template <typename Child>
    class Option : public OptionBase {
      public:
        using OptionBase::OptionBase;

        // Adds a short name for the option, like "-f".
        Child& ShortName(char shortName);
        // Changes the name of the parameter when generating the --help.
        Child& Parameter(std::string parameter);
    };

    // An option returning a bool. Defaults to false.
    // Can be set multiple times on the command line if not using the explicit true/false version.
    class BoolOption : public Option<BoolOption> {
      public:
        BoolOption(absl::string_view name, absl::string_view desc);
        ~BoolOption() override;

        bool GetValue() const;
        std::string GetParameter() const override;

      private:
        ParseResult ParseImpl(absl::Span<const absl::string_view> args) override;
        bool mValue = false;
    };
    BoolOption& AddBool(absl::string_view name, absl::string_view desc = {});

    // An option returning a string. Defaults to the empty string.
    class StringOption : public Option<StringOption> {
      public:
        StringOption(absl::string_view name, absl::string_view desc);
        ~StringOption() override;

        std::string GetValue() const;

      private:
        ParseResult ParseImpl(absl::Span<const absl::string_view> args) override;
        std::string mValue;
    };
    StringOption& AddString(absl::string_view name, absl::string_view desc = {});

    // An option returning a list of string split from a comma-separated argument, or the argument
    // being set multiple times (or both). Defaults to an empty list.
    class StringListOption : public Option<StringListOption> {
      public:
        StringListOption(absl::string_view name, absl::string_view desc);
        ~StringListOption() override;

        absl::Span<const std::string> GetValue() const;
        std::vector<std::string> GetOwnedValue() const;

      private:
        ParseResult ParseImpl(absl::Span<const absl::string_view> args) override;
        std::vector<std::string> mValue;
    };
    StringListOption& AddStringList(absl::string_view name, absl::string_view desc = {});

    // An option converting a string name to a value. The default value can be set with .Default().
    //
    //   parser.AddEnum({{{"a", E::A}, {"b", E::B}}});
    template <typename E>
    class EnumOption : public Option<EnumOption<E>> {
      public:
        EnumOption(std::vector<std::pair<absl::string_view, E>> conversions,
                   absl::string_view name,
                   absl::string_view desc);
        ~EnumOption() override = default;

        E GetValue() const;
        std::string GetParameter() const override;

        EnumOption<E>& Default(E value);

      private:
        std::string JoinNames(absl::string_view separator) const;
        OptionBase::ParseResult ParseImpl(absl::Span<const absl::string_view> args) override;
        E mValue;
        bool mHasDefault;
        std::vector<std::pair<absl::string_view, E>> mConversions;
    };
    static std::string JoinConversionNames(absl::Span<const absl::string_view> names,
                                           absl::string_view separator);
    template <typename E>
    EnumOption<E>& AddEnum(std::vector<std::pair<absl::string_view, E>> conversions,
                           absl::string_view name,
                           absl::string_view desc = {}) {
        return AddOption(std::make_unique<EnumOption<E>>(std::move(conversions), name, desc));
    }

    // Helper to add a --help option.
    BoolOption& AddHelp();

    // Helper structs for the Parse calls.
    struct ParseResult {
        bool success;
        std::string errorMessage = {};
    };
    struct ParseOptions {
        bool unknownIsError = true;
    };
    // TODO(343500108): Use designated initializers when we have C++20.
    static const ParseOptions kDefaultParseOptions;

    // Parse the arguments provided and set the options.
    ParseResult Parse(absl::Span<const absl::string_view> args,
                      const ParseOptions& parseOptions = kDefaultParseOptions);

    // Small wrappers around the previous Parse for ease of use.
    ParseResult Parse(const std::vector<std::string>& args,
                      const ParseOptions& parseOptions = kDefaultParseOptions);
    ParseResult Parse(int argc,
                      const char** argv,
                      const ParseOptions& parseOptions = kDefaultParseOptions);

    // Generate summary of options for a --help and add it to the stream.
    void PrintHelp(std::ostream& s);

  private:
    template <typename T>
    T& AddOption(std::unique_ptr<T> option) {
        T& result = *option.get();
        mOptions.push_back(std::move(option));
        return result;
    }

    std::vector<std::unique_ptr<OptionBase>> mOptions;
};

// Option<Child>
template <typename Child>
Child& CommandLineParser::Option<Child>::ShortName(char shortName) {
    mShortName = shortName;
    return static_cast<Child&>(*this);
}

template <typename Child>
Child& CommandLineParser::Option<Child>::Parameter(std::string parameter) {
    mParameter = parameter;
    return static_cast<Child&>(*this);
}

// EnumOption<E>
template <typename E>
CommandLineParser::EnumOption<E>::EnumOption(
    std::vector<std::pair<absl::string_view, E>> conversions,
    absl::string_view name,
    absl::string_view desc)
    : Option<EnumOption<E>>(name, desc), mConversions(conversions) {}

template <typename E>
E CommandLineParser::EnumOption<E>::GetValue() const {
    DAWN_ASSERT(this->IsSet() || mHasDefault);
    return mValue;
}

template <typename E>
CommandLineParser::OptionBase::ParseResult CommandLineParser::EnumOption<E>::ParseImpl(
    absl::Span<const absl::string_view> args) {
    if (this->IsSet()) {
        return {false, args, "cannot be set multiple times"};
    }

    if (args.empty()) {
        return {false, args, "expected a value"};
    }

    for (auto conversion : mConversions) {
        if (conversion.first == args.front()) {
            mValue = conversion.second;
            return {true, args.subspan(1)};
        }
    }

    // Do manual string sums to avoid include absl_format in this header.
    return {false, args,
            "unknown value \"" + std::string(args.front()) + "\". Expected one of " +
                JoinNames(", ") + "."};
}

template <typename E>
std::string CommandLineParser::EnumOption<E>::GetParameter() const {
    return JoinNames("|");
}

template <typename E>
CommandLineParser::EnumOption<E>& CommandLineParser::EnumOption<E>::Default(E value) {
    mValue = value;
    mHasDefault = true;
    return *this;
}

template <typename E>
std::string CommandLineParser::EnumOption<E>::JoinNames(absl::string_view separator) const {
    std::vector<absl::string_view> names;
    for (auto conversion : mConversions) {
        names.push_back(conversion.first);
    }
    return JoinConversionNames(names, separator);
}

}  // namespace dawn::utils

#endif  // SRC_DAWN_UTILS_COMMANDLINEPARSER_H_
