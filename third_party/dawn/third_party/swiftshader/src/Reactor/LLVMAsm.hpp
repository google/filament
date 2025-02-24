// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef rr_LLVMAsm_hpp
#define rr_LLVMAsm_hpp

#ifdef ENABLE_RR_EMIT_ASM_FILE

#	include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#	include <string>
#	include <vector>

namespace rr {
namespace AsmFile {

// Generate a unique name for the asm file
std::string generateFilename(const std::string &emitDir, std::string routineName);

// Emit an asm file for the current module
bool emitAsmFile(const std::string &filename, llvm::orc::JITTargetMachineBuilder builder, llvm::Module &module);

// Rewrites the previously generated asm file, adding extra useful information.
// In particular, it prepends the final resolved location (address) of each instruction.
// NOTE: Doing this is error-prone since we parse text, and are thus dependent on the
// exact format of LLVM's assembly output. It would be nice if LLVM's asm output included
// at least the 0-based relative address of each instruction.
void fixupAsmFile(const std::string &filename, std::vector<const void *> addresses);

}  // namespace AsmFile
}  // namespace rr

#endif  // ENABLE_RR_EMIT_ASM_FILE

#endif  // rr_LLVMAsm_hpp
