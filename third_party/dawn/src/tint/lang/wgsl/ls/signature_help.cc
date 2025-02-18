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

#include <sstream>
#include "src/tint/lang/wgsl/diagnostic_severity.h"
#include "src/tint/lang/wgsl/ls/server.h"

#include "langsvr/lsp/comparators.h"
#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/wgsl/intrinsic/dialect.h"
#include "src/tint/lang/wgsl/ls/utils.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string_stream.h"
#include "src/tint/utils/text/text_style.h"

namespace lsp = langsvr::lsp;

namespace tint::wgsl::ls {

namespace {

/// @returns the parameter information for all the parameters of the intrinsic overload @p overload.
std::vector<lsp::ParameterInformation> Params(const core::intrinsic::OverloadInfo& overload) {
    auto& data = wgsl::intrinsic::Dialect::kData;

    std::vector<lsp::ParameterInformation> params;
    for (size_t i = 0; i < overload.num_parameters; i++) {
        lsp::ParameterInformation param_out;
        auto& param_in = data[overload.parameters + i];
        if (param_in.usage != core::ParameterUsage::kNone) {
            param_out.label = std::string(core::ToString(param_in.usage));
        } else {
            param_out.label = "param-" + std::to_string(i);
        }
        params.push_back(std::move(param_out));
    }
    return params;
}

/// @returns the zero-based index of the parameter at with the cursor at @p position, for a call
/// with the source @p call_source.
size_t CalcParamIndex(const File& file,
                      const Source& call_source,
                      const Source::Location& position) {
    size_t index = 0;
    int depth = 0;

    auto range = file.Conv(call_source.range);
    auto start = range.start;
    auto end = std::min(range.end, file.Conv(position));
    auto& lines = call_source.file->content.lines;

    for (auto line_idx = start.line; line_idx <= end.line; line_idx++) {
        auto& line = lines[line_idx];
        auto start_character = (line_idx == start.line) ? start.character : 0;
        auto end_character = (line_idx == end.line) ? end.character : line.size();
        auto text = line.substr(start_character, end_character - start_character);
        for (char c : text) {
            switch (c) {
                case '(':
                case '[':
                    depth++;
                    break;
                case ')':
                case ']':
                    depth--;
                    break;
                case ',':
                    if (depth == 1) {
                        index++;
                    }
            }
        }
    }
    return index;
}

/// PrintOverload() emits a description of the intrinsic overload @p overload of the function with
/// name @p intrinsic_name to @p name and @p description.
void PrintOverload(std::string& name,
                   StyledText& description,
                   core::intrinsic::Context& context,
                   const core::intrinsic::OverloadInfo& overload,
                   std::string_view intrinsic_name) {
    core::intrinsic::TemplateState templates;

    auto earliest_eval_stage = core::EvaluationStage::kConstant;

    StyledText name_st;
    name_st << style::Code << intrinsic_name;
    if (overload.num_explicit_templates > 0) {
        name_st << "<";
        for (size_t i = 0; i < overload.num_explicit_templates; i++) {
            const auto& tmpl = context.data[overload.templates + i];
            if (i > 0) {
                name_st << ", ";
            }
            name_st << style::Type(tmpl.name) << " ";
        }
        name_st << ">";
    }

    name_st << "(";
    for (size_t i = 0; i < overload.num_parameters; i++) {
        const auto& parameter = context.data[overload.parameters + i];
        auto* matcher_indices = context.data[parameter.matcher_indices];

        if (i > 0) {
            name_st << ", ";
        }

        if (parameter.usage != core::ParameterUsage::kNone) {
            name_st << style::Variable(parameter.usage, ": ");
        }
        context.Match(templates, overload, matcher_indices, earliest_eval_stage).PrintType(name_st);
    }
    name_st << ")";
    if (overload.return_matcher_indices.IsValid()) {
        name_st << " -> ";
        auto* matcher_indices = context.data[overload.return_matcher_indices];
        context.Match(templates, overload, matcher_indices, earliest_eval_stage).PrintType(name_st);
    }

    {  // Like name_st.Plain(), but no code quotes.
        StringStream ss;
        name_st.Walk([&](std::string_view text, TextStyle) { ss << text; });
        name = ss.str();
    }

    for (size_t i = 0; i < overload.num_templates; i++) {
        auto& tmpl = context.data[overload.templates + i];
        if (auto* matcher_indices = context.data[tmpl.matcher_indices]) {
            description << "\n" << style::Type(tmpl.name) << style::Plain(" is ");
            if (tmpl.kind == core::intrinsic::TemplateInfo::Kind::kType) {
                context.Match(templates, overload, matcher_indices, earliest_eval_stage)
                    .PrintType(description);
            } else {
                context.Match(templates, overload, matcher_indices, earliest_eval_stage)
                    .PrintNum(description);
            }
        }
    }
}

}  // namespace

typename lsp::TextDocumentSignatureHelpRequest::ResultType  //
Server::Handle(const lsp::TextDocumentSignatureHelpRequest& r) {
    auto file = files_.Get(r.text_document.uri);
    if (!file) {
        return lsp::Null{};
    }

    auto& program = (*file)->program;
    auto pos = (*file)->Conv(r.position);

    auto call = (*file)->NodeAt<sem::Call>(pos);
    if (!call) {
        return lsp::Null{};
    }

    lsp::SignatureHelp help;
    help.active_parameter = CalcParamIndex(**file, call->Declaration()->source, pos);
    Switch(call->Target(),  //
           [&](const sem::BuiltinFn* target) {
               auto& data = wgsl::intrinsic::Dialect::kData;
               auto& builtins = data.builtins;
               auto& intrinsic_info = builtins[static_cast<size_t>(target->Fn())];

               for (size_t i = 0; i < intrinsic_info.num_overloads; i++) {
                   auto& overload = data[intrinsic_info.overloads + i];

                   auto params = Params(overload);

                   auto type_mgr = core::type::Manager::Wrap(program.Types());
                   auto symbols = SymbolTable::Wrap(program.Symbols());

                   StyledText description;
                   core::intrinsic::Context ctx{data, type_mgr, symbols};
                   std::string name;
                   PrintOverload(name, description, ctx, overload, wgsl::str(target->Fn()));

                   lsp::SignatureInformation sig;
                   sig.parameters = params;
                   sig.label = name;
                   sig.documentation = Conv(description);
                   help.signatures.push_back(sig);

                   if (&overload == &target->Overload()) {
                       help.active_signature = i;
                   }
               }
           });

    return help;
}

}  // namespace tint::wgsl::ls
