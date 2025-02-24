/* Copyright (c) 2024-2025 The Khronos Group Inc.
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "spirv_logging.h"
#include <string>
#include <cstring>

// TODO https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9384
// Fix GCC 13 issues with regex
#if defined(__GNUC__) && (__GNUC__ > 12)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#include <regex>
#if defined(__GNUC__) && (__GNUC__ > 12)
#pragma GCC diagnostic pop
#endif

#include "state_tracker/shader_instruction.h"
#include <spirv/unified1/NonSemanticShaderDebugInfo100.h>
#include <spirv/unified1/spirv.hpp>
#include "generated/spirv_grammar_helper.h"

namespace spirv {

static const int kModuleStartingOffset = 5;  // first 5 words of module are the headers
static inline uint32_t Opcode(uint32_t instruction) { return instruction & 0x0ffffu; }
static inline uint32_t Length(uint32_t instruction) { return instruction >> 16; }

struct SpirvLoggingInfo {
    uint32_t file_string_id = 0;  // OpString with filename
    uint32_t line_number_start = 0;
    uint32_t line_number_end = 0;
    uint32_t column_number = 0;            // most compiler will just give zero here, so just try and get a start column
    bool using_shader_debug_info = false;  // NonSemantic.Shader.DebugInfo.100
    std::string reported_filename;
};

// Read the contents of the SPIR-V OpSource instruction and any following continuation instructions.
// Split the single string into a vector of strings, one for each line, for easier processing.
static void ReadOpSource(const std::vector<uint32_t> &instructions, const uint32_t reported_file_id,
                         std::vector<std::string> &out_source_lines) {
    uint32_t offset = kModuleStartingOffset;
    while (offset < instructions.size()) {
        const uint32_t instruction = instructions[offset];
        const uint32_t length = Length(instruction);
        const uint32_t opcode = Opcode(instruction);
        if (opcode != spv::OpSource || length < 5 || instructions[offset + 3] != reported_file_id) {
            offset += length;
            continue;
        }

        // OpSource has been found
        std::istringstream in_stream;
        std::string current_line;
        const char *str = reinterpret_cast<const char *>(&instructions[offset + 4]);
        in_stream.str(str);
        while (std::getline(in_stream, current_line)) {
            out_source_lines.emplace_back(current_line);
        }

        offset += length;  // point to next instruction after OpSource
        break;
    }

    // Look for OpSourceContinued, it must be right after
    while (offset < instructions.size()) {
        const uint32_t continue_insn = instructions[offset];
        const uint32_t length = Length(continue_insn);
        const uint32_t opcode = Opcode(continue_insn);
        if (opcode != spv::OpSourceContinued) {
            break;
        }

        std::istringstream in_stream;
        std::string current_line;
        const char *str = reinterpret_cast<const char *>(&instructions[offset + 1]);
        in_stream.str(str);
        while (std::getline(in_stream, current_line)) {
            out_source_lines.emplace_back(current_line);
        }
        offset += length;
    }
}

static void ReadDebugSource(const std::vector<uint32_t> &instructions, const uint32_t debug_source_id, uint32_t &out_file_string_id,
                            std::vector<std::string> &out_source_lines) {
    uint32_t offset = kModuleStartingOffset;
    while (offset < instructions.size()) {
        const uint32_t instruction = instructions[offset];
        const uint32_t length = Length(instruction);
        const uint32_t opcode = Opcode(instruction);
        if (opcode != spv::OpExtInst || instructions[offset + 2] != debug_source_id ||
            instructions[offset + 4] != NonSemanticShaderDebugInfo100DebugSource) {
            offset += length;
            continue;
        }

        // We have now found the proper OpExtInst DebugSource
        out_file_string_id = instructions[offset + 5];

        // Optional source Text not provided so nothing left to do
        if (length < 7) {
            return;
        }

        const uint32_t string_id = instructions[offset + 6];
        const char *source_text = spirv::GetOpString(instructions, string_id);
        if (!source_text) {
            return;  // error should be caught in spirv-val, but don't crash here
        }

        std::istringstream in_stream;
        std::string current_line;
        in_stream.str(source_text);
        while (std::getline(in_stream, current_line)) {
            out_source_lines.emplace_back(current_line);
        }

        offset += length;  // point to next instruction after OpSource
        break;
    }

    // Look for DebugSourceContinued, it must be right after
    while (offset < instructions.size()) {
        const uint32_t continue_insn = instructions[offset];
        const uint32_t length = Length(continue_insn);
        const uint32_t opcode = Opcode(continue_insn);

        if (opcode != spv::OpExtInst || instructions[offset + 4] != NonSemanticShaderDebugInfo100DebugSourceContinued) {
            break;
        }

        const uint32_t string_id = instructions[offset + 5];
        const char *continue_text = spirv::GetOpString(instructions, string_id);
        if (!continue_text) {
            return;  // error should be caught in spirv-val, but don't crash here
        }

        std::istringstream in_stream;
        std::string current_line;
        in_stream.str(continue_text);
        while (std::getline(in_stream, current_line)) {
            out_source_lines.emplace_back(current_line);
        }

        offset += length;
    }
}

// The task here is to search the OpSource content to find the #line directive with the
// line number that is closest to, but still prior to the reported error line number and
// still within the reported filename.
// From this known position in the OpSource content we can add the difference between
// the #line line number and the reported error line number to determine the location
// in the OpSource content of the reported error line.
//
// Considerations:
// - Look only at #line directives that specify the reported_filename since
//   the reported error line number refers to its location in the reported filename.
// - If a #line directive does not have a filename, the file is the reported filename, or
//   the filename found in a prior #line directive.  (This is C-preprocessor behavior)
// - It is possible (e.g., inlining) for blocks of code to get shuffled out of their
//   original order and the #line directives are used to keep the numbering correct.  This
//   is why we need to examine the entire contents of the source, instead of leaving early
//   when finding a #line line number larger than the reported error line number.
//
static bool GetLineFromDirective(const std::string &string, uint32_t *linenumber, std::string &filename) {
    static const std::regex line_regex(  // matches #line directives
        "^"                              // beginning of line
        "\\s*"                           // optional whitespace
        "#"                              // required text
        "\\s*"                           // optional whitespace
        "line"                           // required text
        "\\s+"                           // required whitespace
        "([0-9]+)"                       // required first capture - line number
        "(\\s+)?"                        // optional second capture - whitespace
        "(\".+\")?"                      // optional third capture - quoted filename with at least one char inside
        ".*");                           // rest of line (needed when using std::regex_match since the entire line is tested)

    std::smatch captures;

    const bool found_line = std::regex_match(string, captures, line_regex);
    if (!found_line) return false;

    // filename is optional and considered found only if the whitespace and the filename are captured
    if (captures[2].matched && captures[3].matched) {
        // Remove enclosing double quotes.  The regex guarantees the quotes and at least one char.
        filename = captures[3].str().substr(1, captures[3].str().size() - 2);
    }
    *linenumber = (uint32_t)std::stoul(captures[1]);
    return true;
}

// Return false if any error arise
static bool GetLineAndFilename(std::ostringstream &ss, const std::vector<uint32_t> &instructions, SpirvLoggingInfo &logging_info) {
    const std::string debug_info_type = (logging_info.using_shader_debug_info) ? "DebugSource" : "OpLine";
    if (logging_info.file_string_id == 0) {
        // This error should be caught in spirv-val
        ss << "(Unable to find file string from SPIR-V " << debug_info_type << ")\n";
        return false;
    }

    const char *file_string_insn = spirv::GetOpString(instructions, logging_info.file_string_id);
    if (!file_string_insn) {
        ss << "(Unable to find SPIR-V OpString from " << debug_info_type << " instruction.\n";
        ss << "File ID = " << logging_info.file_string_id << ", Line Number = " << logging_info.line_number_start
           << ", Column = " << logging_info.column_number << ")\n";
        return false;
    }

    logging_info.reported_filename = std::string(file_string_insn);
    if (!logging_info.reported_filename.empty()) {
        ss << "in file " << logging_info.reported_filename << " ";
    }

    ss << "at line " << logging_info.line_number_start;
    if (logging_info.line_number_end > logging_info.line_number_start) {
        ss << " to " << logging_info.line_number_end;
    }

    if (logging_info.column_number != 0) {
        ss << ", column " << logging_info.column_number;
    }
    ss << '\n';

    return true;
}

static void GetSourceLines(std::ostringstream &ss, const std::vector<std::string> &source_lines,
                           const SpirvLoggingInfo &logging_info) {
    if (source_lines.empty()) {
        if (logging_info.using_shader_debug_info) {
            ss << "No Text operand found in DebugSource\n";
        } else {
            ss << "Unable to find SPIR-V OpSource\n";
        }
        return;
    }

    // Find the line in the OpSource content that corresponds to the reported error file and line.
    uint32_t saved_line_number = 0;
    std::string current_filename = logging_info.reported_filename;  // current "preprocessor" filename state.
    std::vector<std::string>::size_type saved_opsource_offset = 0;

    // This was designed to fine the best line if using #line in GLSL
    bool found_best_line = false;
    if (!logging_info.using_shader_debug_info) {
        for (auto it = source_lines.begin(); it != source_lines.end(); ++it) {
            uint32_t parsed_line_number;
            std::string parsed_filename;
            const bool found_line = GetLineFromDirective(*it, &parsed_line_number, parsed_filename);
            if (!found_line) continue;

            const bool found_filename = parsed_filename.size() > 0;
            if (found_filename) {
                current_filename = parsed_filename;
            }
            if ((!found_filename) || (current_filename == logging_info.reported_filename)) {
                // Update the candidate best line directive, if the current one is prior and closer to the reported line
                if (logging_info.line_number_start >= parsed_line_number) {
                    if (!found_best_line || (logging_info.line_number_start - parsed_line_number <=
                                             logging_info.line_number_start - saved_line_number)) {
                        saved_line_number = parsed_line_number;
                        saved_opsource_offset = std::distance(source_lines.begin(), it);
                        found_best_line = true;
                    }
                }
            }
        }
    }

    if (logging_info.using_shader_debug_info) {
        // For Shader Debug Info, we should have all the information we need
        ss << '\n';
        for (uint32_t line_index = logging_info.line_number_start; line_index <= logging_info.line_number_end; line_index++) {
            if (line_index > source_lines.size()) {
                ss << line_index << ": [No line found in source]";
                break;
            }
            ss << line_index << ": " << source_lines[line_index - 1] << '\n';
        }
        // Only show column if since line is displayed
        if (logging_info.column_number > 0 && logging_info.line_number_start == logging_info.line_number_end) {
            std::string spaces(logging_info.column_number - 1, ' ');
            ss << spaces << '^';
        }

    } else if (found_best_line) {
        assert(logging_info.line_number_start >= saved_line_number);
        const size_t opsource_index = (logging_info.line_number_start - saved_line_number) + 1 + saved_opsource_offset;
        if (opsource_index < source_lines.size()) {
            ss << '\n' << logging_info.line_number_start << ": " << source_lines[opsource_index] << '\n';
        } else {
            ss << "Internal error: calculated source line of " << opsource_index << " for source size of " << source_lines.size()
               << " lines\n";
        }
    } else if (logging_info.line_number_start < source_lines.size() && logging_info.line_number_start != 0) {
        // file lines normally start at 1 index
        ss << '\n' << source_lines[logging_info.line_number_start - 1] << '\n';
        if (logging_info.column_number > 0) {
            std::string spaces(logging_info.column_number - 1, ' ');
            ss << spaces << '^';
        }
    } else {
        ss << "Unable to find a suitable line in SPIR-V OpSource\n";
    }
}

void GetShaderSourceInfo(std::ostringstream &ss, const std::vector<uint32_t> &instructions, const Instruction &last_line_insn) {
    // Read the source code and split it up into separate lines.
    //
    // 1. OpLine will point to a OpSource/OpSourceContinued which have the string built-in
    // 2. DebugLine will point to a DebugSource/DebugSourceContinued that each point to a OpString
    //
    // For the second one, we need to build the source lines up sooner
    std::vector<std::string> source_lines;

    SpirvLoggingInfo logging_info = {};
    if (last_line_insn.Opcode() == spv::OpLine) {
        logging_info.using_shader_debug_info = false;
        logging_info.file_string_id = last_line_insn.Word(1);
        logging_info.line_number_start = last_line_insn.Word(2);
        logging_info.line_number_end = logging_info.line_number_start;  // OpLine only give a single line granularity
        logging_info.column_number = last_line_insn.Word(3);
    } else {
        // NonSemanticShaderDebugInfo100DebugLine
        logging_info.using_shader_debug_info = true;
        logging_info.line_number_start = GetConstantValue(instructions, last_line_insn.Word(6));
        logging_info.line_number_end = GetConstantValue(instructions, last_line_insn.Word(7));
        logging_info.column_number = GetConstantValue(instructions, last_line_insn.Word(8));
        const uint32_t debug_source_id = last_line_insn.Word(5);
        ReadDebugSource(instructions, debug_source_id, logging_info.file_string_id, source_lines);
    }

    if (!GetLineAndFilename(ss, instructions, logging_info)) {
        return;
    }

    // Defer finding source from OpLine until we know we have a valid file string to tie it too
    if (!logging_info.using_shader_debug_info) {
        ReadOpSource(instructions, logging_info.file_string_id, source_lines);
    }

    GetSourceLines(ss, source_lines, logging_info);
}

const char *GetOpString(const std::vector<uint32_t> &instructions, uint32_t string_id) {
    uint32_t offset = kModuleStartingOffset;
    while (offset < instructions.size()) {
        const uint32_t instruction = instructions[offset];
        const uint32_t length = Length(instruction);
        const uint32_t opcode = Opcode(instruction);

        // if here, seen all OpString and can return early
        if (opcode == spv::OpFunction) break;

        if (opcode == spv::OpString) {
            const uint32_t result_id = instructions[offset + 1];
            if (result_id == string_id) {
                return reinterpret_cast<const char *>(&instructions[offset + 2]);
            }
        }
        offset += length;
    }
    return nullptr;
}

uint32_t GetConstantValue(const std::vector<uint32_t> &instructions, uint32_t constant_id) {
    uint32_t offset = kModuleStartingOffset;
    while (offset < instructions.size()) {
        const uint32_t instruction = instructions[offset];
        const uint32_t length = Length(instruction);
        const uint32_t opcode = Opcode(instruction);

        // if here, seen all OpConstant and can return early
        if (opcode == spv::OpFunction) break;

        if (opcode == spv::OpConstant) {
            const uint32_t result_id = instructions[offset + 2];
            if (result_id == constant_id) {
                return instructions[offset + 3];
            }
        }
        offset += length;
    }
    assert(false);
    return 0;
}

void GetExecutionModelNames(const std::vector<uint32_t> &instructions, std::ostringstream &ss) {
    bool first_stage = true;

    uint32_t offset = kModuleStartingOffset;
    while (offset < instructions.size()) {
        const uint32_t instruction = instructions[offset];
        const uint32_t length = Length(instruction);
        const uint32_t opcode = Opcode(instruction);

        // if here, seen all OpEntryPoint and can return early
        if (opcode == spv::OpFunction) break;

        if (opcode == spv::OpEntryPoint) {
            if (first_stage) {
                first_stage = false;
            } else {
                ss << ", ";
            }
            const uint32_t entry_point_id = instructions[offset + 1];
            ss << string_SpvExecutionModel(entry_point_id);
        }

        offset += length;
    }
}

// Find the OpLine/DebugLine just before the failing instruction indicated by the debug info.
// Return the offset into the instructions array
uint32_t GetDebugLineOffset(const std::vector<uint32_t> &instructions, uint32_t instruction_position) {
    uint32_t index = 0;
    uint32_t shader_debug_info_set_id = 0;
    uint32_t last_line_inst_offset = 0;

    uint32_t offset = kModuleStartingOffset;
    while (offset < instructions.size()) {
        const uint32_t instruction = instructions[offset];
        const uint32_t length = Length(instruction);
        const uint32_t opcode = Opcode(instruction);

        if (opcode == spv::OpExtInstImport) {
            const char *str = reinterpret_cast<const char *>(&instructions[offset + 2]);
            if (strcmp(str, "NonSemantic.Shader.DebugInfo.100") == 0) {
                shader_debug_info_set_id = instructions[offset + 1];
            }
        }

        if (opcode == spv::OpExtInst && instructions[offset + 3] == shader_debug_info_set_id &&
            instructions[offset + 4] == NonSemanticShaderDebugInfo100DebugLine) {
            last_line_inst_offset = offset;
        } else if (opcode == spv::OpLine) {
            last_line_inst_offset = offset;
        } else if (opcode == spv::OpFunctionEnd) {
            last_line_inst_offset = 0;  // debug lines can't cross functions boundaries
        }

        if (index == instruction_position) {
            break;
        }
        index++;

        offset += length;
    }

    return last_line_inst_offset;
}

}  // namespace spirv