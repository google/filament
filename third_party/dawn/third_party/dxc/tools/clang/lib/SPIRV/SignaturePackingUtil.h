//===--- SignaturePackingUtil.h - Utility functions for signature packing -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_SIGNATUREPACKINGUTIL_H
#define LLVM_CLANG_LIB_SPIRV_SIGNATUREPACKINGUTIL_H

#include <vector>

#include "clang/SPIRV/SpirvBuilder.h"
#include "llvm/ADT/STLExtras.h"

#include "StageVar.h"

namespace clang {
namespace spirv {

/// \brief Packs signature by assigning locations and components to stage
/// variables |vars|. |nextLocs| is a function that returns the next available
/// location for the given number of required locations. |spvBuilder| is used to
/// create OpDecorate instructions. |forInput| is true when |vars| are input
/// stage variables.
bool packSignature(SpirvBuilder &spvBuilder,
                   const std::vector<const StageVar *> &vars,
                   llvm::function_ref<uint32_t(uint32_t)> nextLocs,
                   bool forInput);

} // end namespace spirv
} // end namespace clang

#endif
