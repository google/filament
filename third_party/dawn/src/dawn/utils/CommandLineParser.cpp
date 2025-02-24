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

#include "dawn/utils/CommandLineParser.h"

#include <algorithm>
#include <tuple>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"

namespace dawn::utils {

// OptionBase

CommandLineParser::OptionBase::OptionBase(absl::string_view name, absl::string_view desc)
    : mName(name), mDescription(desc), mParameter("value") {}

CommandLineParser::OptionBase::~OptionBase() = default;

const std::string& CommandLineParser::OptionBase::GetName() const {
    return mName;
}

std::string CommandLineParser::OptionBase::GetShortName() const {
    return mShortName;
}

const std::string& CommandLineParser::OptionBase::GetDescription() const {
    return mDescription;
}

std::string CommandLineParser::OptionBase::GetParameter() const {
    return mParameter;
}

bool CommandLineParser::OptionBase::IsSet() const {
    return mSet;
}

CommandLineParser::OptionBase::ParseResult CommandLineParser::OptionBase::Parse(
    absl::Span<const absl::string_view> args) {
    auto result = ParseImpl(args);
    if (result.success) {
        mSet = true;
    }
    return result;
}

// BoolOption

CommandLineParser::BoolOption::BoolOption(absl::string_view name, absl::string_view desc)
    : Option(name, desc) {}

CommandLineParser::BoolOption::~BoolOption() = default;

bool CommandLineParser::BoolOption::GetValue() const {
    return mValue;
}

std::string CommandLineParser::BoolOption::GetParameter() const {
    return {};
}

CommandLineParser::OptionBase::ParseResult CommandLineParser::BoolOption::ParseImpl(
    absl::Span<const absl::string_view> args) {
    // Explicit true
    if (!args.empty() && args.front() == "true") {
        if (IsSet()) {
            return {false, args, "cannot set multiple times with explicit true/false arguments"};
        }

        mValue = true;
        return {true, args.subspan(1)};
    }

    // Explicit false
    if (!args.empty() && args.front() == "false") {
        if (IsSet()) {
            return {false, args, "cannot set multiple times with explicit true/false arguments"};
        }

        mValue = false;
        return {true, args.subspan(1)};
    }

    // Assuming --option just means to set it to true. Allow setting the option multiple times
    // to true with this form.
    if (IsSet() && !mValue) {
        return {false, args, "cannot be set to both true and false"};
    }

    mValue = true;
    return {true, args};
}

// StringOption

CommandLineParser::StringOption::StringOption(absl::string_view name, absl::string_view desc)
    : Option(name, desc) {}

CommandLineParser::StringOption::~StringOption() = default;

std::string CommandLineParser::StringOption::GetValue() const {
    return mValue;
}

CommandLineParser::OptionBase::ParseResult CommandLineParser::StringOption::ParseImpl(
    absl::Span<const absl::string_view> args) {
    if (IsSet()) {
        return {false, args, "cannot be set multiple times"};
    }

    if (args.empty()) {
        return {false, args, "expected a value"};
    }

    mValue = args.front();
    return {true, args.subspan(1)};
}

// StringListOption

CommandLineParser::StringListOption::StringListOption(absl::string_view name,
                                                      absl::string_view desc)
    : Option(name, desc) {}

CommandLineParser::StringListOption::~StringListOption() = default;

absl::Span<const std::string> CommandLineParser::StringListOption::GetValue() const {
    return mValue;
}

std::vector<std::string> CommandLineParser::StringListOption::GetOwnedValue() const {
    std::vector<std::string> result;
    for (auto& v : mValue) {
        result.push_back(v);
    }
    return result;
}

CommandLineParser::OptionBase::ParseResult CommandLineParser::StringListOption::ParseImpl(
    absl::Span<const absl::string_view> args) {
    if (args.empty()) {
        return {false, args, "expected a value"};
    }

    for (absl::string_view s : absl::StrSplit(args.front(), ",")) {
        mValue.push_back(std::string{s});
    }
    return {true, args.subspan(1)};
}

// CommandLineParser

CommandLineParser::BoolOption& CommandLineParser::AddBool(absl::string_view name,
                                                          absl::string_view desc) {
    return AddOption(std::make_unique<BoolOption>(name, desc));
}

CommandLineParser::StringOption& CommandLineParser::AddString(absl::string_view name,
                                                              absl::string_view desc) {
    return AddOption(std::make_unique<StringOption>(name, desc));
}

CommandLineParser::StringListOption& CommandLineParser::AddStringList(absl::string_view name,
                                                                      absl::string_view desc) {
    return AddOption(std::make_unique<StringListOption>(name, desc));
}

// static
std::string CommandLineParser::JoinConversionNames(absl::Span<const absl::string_view> names,
                                                   absl::string_view separator) {
    return absl::StrJoin(names, separator);
}

CommandLineParser::BoolOption& CommandLineParser::AddHelp() {
    return AddBool("help", "Shows the help").ShortName('h');
}

// static
const CommandLineParser::ParseOptions CommandLineParser::kDefaultParseOptions = {};

CommandLineParser::ParseResult CommandLineParser::Parse(absl::Span<const absl::string_view> args,
                                                        const ParseOptions& parseOptions) {
    // Build the map of name to option.
    absl::flat_hash_map<std::string, OptionBase*> nameToOption;

    for (auto& option : mOptions) {
        if (!nameToOption.emplace(option->GetName(), option.get()).second) {
            return {false,
                    absl::StrFormat("Duplicate options with name \"%s\"", option->GetName())};
        }

        if (!option->GetShortName().empty() &&
            !nameToOption.emplace(option->GetShortName(), option.get()).second) {
            return {false,
                    absl::StrFormat("Duplicate options with name \"%s\"", option->GetShortName())};
        }
    }

    auto nextArgs = args;
    while (!nextArgs.empty()) {
        auto arg = nextArgs.front();
        nextArgs = nextArgs.subspan(1);

        // Skip or error if it is not an option.
        if (arg.empty() || arg[0] != '-') {
            if (parseOptions.unknownIsError) {
                return {false, absl::StrFormat("Unknown option \"%s\"", arg)};
            }
            continue;
        }

        // Remove starting - or --
        arg = arg.substr(1);
        if (!arg.empty() && arg[0] == '-') {
            arg = arg.substr(1);
        }

        // Split at the = if there is one and use the right side as the option's arguments
        if (auto equalPos = arg.find('='); equalPos != std::string::npos) {
            auto name = arg.substr(0, equalPos);
            auto rest = arg.substr(equalPos + 1);

            // Skip or error if it is an unknown option.
            if (!nameToOption.contains(name)) {
                if (parseOptions.unknownIsError) {
                    return {false, absl::StrFormat("Unknown option \"%s\"", name)};
                }
                continue;
            }

            auto option = nameToOption[name];
            auto optionResult = option->Parse({&rest, 1});
            if (!optionResult.success) {
                return {false, absl::StrFormat("Failure while parsing \"%s\": %s",
                                               option->GetName(), optionResult.errorMessage)};
            }
            if (!optionResult.remainingArgs.empty()) {
                return {false, absl::StrFormat("Argument \"%s\" was not valid for option \"%s\"",
                                               rest, option->GetName())};
            }
            continue;
        }

        // Otherwise make the option greedily parse from the rest of the command line.
        // Skip or error if it is an unknown option.
        if (!nameToOption.contains(arg)) {
            if (parseOptions.unknownIsError) {
                return {false, absl::StrFormat("Unknown option \"%s\"", arg)};
            }
            continue;
        }

        // Try to parse the arg.
        auto option = nameToOption[arg];
        auto optionResult = option->Parse(nextArgs);
        if (!optionResult.success) {
            return {false, absl::StrFormat("Failure while parsing \"%s\": %s", option->GetName(),
                                           optionResult.errorMessage)};
        }
        nextArgs = optionResult.remainingArgs;
    }

    return {true};
}

CommandLineParser::ParseResult CommandLineParser::Parse(const std::vector<std::string>& args,
                                                        const ParseOptions& parseOptions) {
    std::vector<absl::string_view> viewArgs;
    for (const auto& arg : args) {
        viewArgs.push_back(arg);
    }

    return Parse(viewArgs, parseOptions);
}

CommandLineParser::ParseResult CommandLineParser::Parse(int argc,
                                                        const char** argv,
                                                        const ParseOptions& parseOptions) {
    std::vector<absl::string_view> args;
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }

    return Parse(args, parseOptions);
}

void CommandLineParser::PrintHelp(std::ostream& s) {
    // Sort options in alphabetical order using a trick that std::tuple is sorted lexicographically.
    std::vector<std::tuple<absl::string_view, OptionBase*>> sortedOptions;
    for (auto& option : mOptions) {
        sortedOptions.emplace_back(option->GetName(), option.get());
    }
    std::sort(sortedOptions.begin(), sortedOptions.end());

    for (auto& [name, option] : sortedOptions) {
        s << "--" << name;

        std::string parameter = option->GetParameter();
        if (!parameter.empty()) {
            s << " <" << parameter << ">";
        }

        const auto& desc = option->GetDescription();
        if (!desc.empty()) {
            s << "  " << desc;
        }

        s << "\n";

        if (!option->GetShortName().empty()) {
            s << "-" << option->GetShortName() << "  alias for --" << name << "\n";
        }
    }
}

}  // namespace dawn::utils
