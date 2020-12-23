// Copyright (c) 2019 Google LLC
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

#ifndef TEST_FUZZ_FUZZ_TEST_UTIL_H_
#define TEST_FUZZ_FUZZ_TEST_UTIL_H_

#include <vector>

#include "source/fuzz/protobufs/spirvfuzz_protobufs.h"
#include "source/fuzz/transformation.h"
#include "source/fuzz/transformation_context.h"
#include "source/opt/build_module.h"
#include "source/opt/ir_context.h"
#include "spirv-tools/libspirv.h"

namespace spvtools {
namespace fuzz {

extern const spvtools::MessageConsumer kConsoleMessageConsumer;

// Returns true if and only if the given binaries are bit-wise equal.
bool IsEqual(spv_target_env env, const std::vector<uint32_t>& expected_binary,
             const std::vector<uint32_t>& actual_binary);

// Assembles the given text and returns true if and only if the resulting binary
// is bit-wise equal to the given binary.
bool IsEqual(spv_target_env env, const std::string& expected_text,
             const std::vector<uint32_t>& actual_binary);

// Assembles the given text and turns the given IR into binary, then returns
// true if and only if the resulting binaries are bit-wise equal.
bool IsEqual(spv_target_env env, const std::string& expected_text,
             const opt::IRContext* actual_ir);

// Turns the given IRs into binaries, then returns true if and only if the
// resulting binaries are bit-wise equal.
bool IsEqual(spv_target_env env, const opt::IRContext* ir_1,
             const opt::IRContext* ir_2);

// Turns |ir_2| into a binary, then returns true if and only if the resulting
// binary is bit-wise equal to |binary_1|.
bool IsEqual(spv_target_env env, const std::vector<uint32_t>& binary_1,
             const opt::IRContext* ir_2);

// Assembles the given IR context, then returns its disassembly as a string.
// Useful for debugging.
std::string ToString(spv_target_env env, const opt::IRContext* ir);

// Returns the disassembly of the given binary as a string.
// Useful for debugging.
std::string ToString(spv_target_env env, const std::vector<uint32_t>& binary);

// Assembly options for writing fuzzer tests.  It simplifies matters if
// numeric ids do not change.
const uint32_t kFuzzAssembleOption =
    SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS;
// Disassembly options for writing fuzzer tests.
const uint32_t kFuzzDisassembleOption =
    SPV_BINARY_TO_TEXT_OPTION_NO_HEADER | SPV_BINARY_TO_TEXT_OPTION_INDENT;

// Dumps the SPIRV-V module in |context| to file |filename|. Useful for
// interactive debugging.
void DumpShader(opt::IRContext* context, const char* filename);

// Dumps |binary| to file |filename|. Useful for interactive debugging.
void DumpShader(const std::vector<uint32_t>& binary, const char* filename);

// Dumps |transformations| to file |filename| in binary format. Useful for
// interactive debugging.
void DumpTransformationsBinary(
    const protobufs::TransformationSequence& transformations,
    const char* filename);

// Dumps |transformations| to file |filename| in JSON format. Useful for
// interactive debugging.
void DumpTransformationsJson(
    const protobufs::TransformationSequence& transformations,
    const char* filename);

// Applies |transformation| to |ir_context| and |transformation_context|, and
// asserts that any ids in |ir_context| that are only present post-
// transformation are either contained in |transformation.GetFreshIds()|, or
// in |issued_overflow_ids|.
void ApplyAndCheckFreshIds(
    const Transformation& transformation, opt::IRContext* ir_context,
    TransformationContext* transformation_context,
    const std::unordered_set<uint32_t>& issued_overflow_ids = {{}});

}  // namespace fuzz
}  // namespace spvtools

#endif  // TEST_FUZZ_FUZZ_TEST_UTIL_H_
