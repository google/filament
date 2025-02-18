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

#include "src/tint/utils/command/cli.h"

#include <algorithm>
#include <sstream>
#include <utility>

#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/text/string.h"

namespace tint::cli {

Option::Option() = default;
Option::~Option() = default;

void OptionSet::ShowHelp(std::ostream& s_out, bool show_equal_form) {
    Vector<const Option*, 32> sorted_options;
    for (auto* opt : options.Objects()) {
        sorted_options.Push(opt);
    }
    sorted_options.Sort([](const Option* a, const Option* b) { return a->Name() < b->Name(); });

    struct CmdInfo {
        std::string left;
        std::string right;
    };
    Vector<CmdInfo, 64> cmd_infos;

    for (auto* opt : sorted_options) {
        {
            std::stringstream left, right;
            left << "--" << opt->Name();
            if (auto param = opt->Parameter(); !param.empty()) {
                left << (show_equal_form ? '=' : ' ') << "<" << param << ">";
            }
            right << opt->Description();
            if (auto def = opt->DefaultValue(); !def.empty()) {
                right << "\ndefault: " << def;
            }
            cmd_infos.Push({left.str(), right.str()});
        }
        if (auto alias = opt->Alias(); !alias.empty()) {
            std::stringstream left, right;
            left << "--" << alias;
            right << "alias for --" << opt->Name();
            cmd_infos.Push({left.str(), right.str()});
        }
        if (auto sn = opt->ShortName(); !sn.empty()) {
            std::stringstream left, right;
            left << " -" << sn;
            right << "short name for --" << opt->Name();
            cmd_infos.Push({left.str(), right.str()});
        }
    }

    const size_t kMaxRightOffset = 30;

    // Measure
    size_t left_width = 0;
    for (auto& cmd_info : cmd_infos) {
        for (auto line : tint::Split(cmd_info.left, "\n")) {
            if (line.length() < kMaxRightOffset) {
                left_width = std::max(left_width, line.length());
            }
        }
    }

    // Print
    left_width = std::min(left_width, kMaxRightOffset);

    auto pad = [&](size_t n) {
        while (n--) {
            s_out << " ";
        }
    };

    for (auto& cmd_info : cmd_infos) {
        auto left_lines = tint::Split(cmd_info.left, "\n");
        auto right_lines = tint::Split(cmd_info.right, "\n");

        size_t num_lines = std::max(left_lines.Length(), right_lines.Length());
        for (size_t i = 0; i < num_lines; i++) {
            bool has_left = (i < left_lines.Length()) && !left_lines[i].empty();
            bool has_right = (i < right_lines.Length()) && !right_lines[i].empty();
            if (has_left) {
                s_out << left_lines[i];
                if (has_right) {
                    if (left_lines[i].length() > left_width) {
                        // Left exceeds column width.
                        // Insert a new line and indent to the right
                        s_out << "\n";
                        pad(left_width);
                    } else {
                        pad(left_width - left_lines[i].length());
                    }
                }
            } else if (has_right) {
                pad(left_width);
            }
            if (has_right) {
                s_out << "  " << right_lines[i];
            }
            s_out << "\n";
        }
    }
}

Result<OptionSet::Unconsumed> OptionSet::Parse(VectorRef<std::string_view> arguments_raw,
                                               const ParseOptions& parse_options /* = {} */) {
    // Build a map of name to option, and set defaults
    Hashmap<std::string, Option*, 32> options_by_name;
    for (auto* opt : options.Objects()) {
        opt->SetDefault();
        for (auto name : {opt->Name(), opt->Alias(), opt->ShortName()}) {
            if (!name.empty() && !options_by_name.Add(name, opt)) {
                return Failure{"multiple options with name '" + name + "'"};
            }
        }
    }

    // Canonicalize arguments by splitting '--foo=x' into '--foo' 'x'.
    std::deque<std::string_view> arguments;
    for (auto arg : arguments_raw) {
        if (HasPrefix(arg, "-")) {
            if (auto eq_idx = arg.find("="); eq_idx != std::string_view::npos) {
                arguments.push_back(arg.substr(0, eq_idx));
                arguments.push_back(arg.substr(eq_idx + 1));
                continue;
            }
        }
        arguments.push_back(arg);
    }

    Hashset<Option*, 8> options_parsed;

    Unconsumed unconsumed;
    while (!arguments.empty()) {
        auto arg = std::move(arguments.front());
        arguments.pop_front();
        auto name = TrimLeft(arg, [](char c) { return c == '-'; });
        if (arg == name || name.length() == 0) {
            unconsumed.Push(arg);
            continue;
        }
        if (auto opt = options_by_name.Get(name)) {
            if (auto err = (*opt)->Parse(arguments); !err.empty()) {
                return Failure{err};
            }
        } else if (!parse_options.ignore_unknown) {
            StyledText err;
            err << "unknown flag: " << arg << "\n";
            auto names = options_by_name.Keys();
            auto alternatives =
                Transform(names, [&](const std::string& s) { return std::string_view(s); });
            tint::SuggestAlternativeOptions opts;
            opts.prefix = "--";
            opts.list_possible_values = false;
            SuggestAlternatives(arg, alternatives.Slice(), err, opts);
            return Failure{err.Plain()};
        }
    }

    return unconsumed;
}

}  // namespace tint::cli
