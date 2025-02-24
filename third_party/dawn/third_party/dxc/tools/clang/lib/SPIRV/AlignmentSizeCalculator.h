//===--- AlignmentSizeCalculator.h - Alignment And Size Calc -----*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_ALIGNMENTSIZECALCULATOR_H
#define LLVM_CLANG_LIB_SPIRV_ALIGNMENTSIZECALCULATOR_H

#include "dxc/Support/SPIRVOptions.h"
#include "clang/AST/ASTContext.h"
#include "clang/SPIRV/AstTypeProbe.h"

namespace clang {
namespace spirv {

/// The class responsible to translate Clang frontend types into SPIR-V types.
class AlignmentSizeCalculator {
public:
  AlignmentSizeCalculator(ASTContext &astCtx, const SpirvCodeGenOptions &opts)
      : astContext(astCtx), spvOptions(opts) {}

  /// \brief Returns the alignment and size in bytes for the given type
  /// according to the given LayoutRule. If the caller has information about
  /// whether the type is a row-major matrix, that should also be passed in. If
  /// this information is not provided, the function tries to find any majorness
  /// attributes on the given type and use it.
  ///
  /// If the type is an array/matrix type, writes the array/matrix stride to
  /// stride.
  ///
  /// Note that the size returned is not exactly how many bytes the type
  /// will occupy in memory; rather it is used in conjunction with alignment
  /// to get the next available location (alignment + size), which means
  /// size contains post-paddings required by the given type.
  std::pair<uint32_t, uint32_t>
  getAlignmentAndSize(QualType type, SpirvLayoutRule rule,
                      llvm::Optional<bool> isRowMajor, uint32_t *stride) const;

  /// \brief Aligns currentOffset properly to allow packing vectors in the HLSL
  /// way: using the element type's alignment as the vector alignment, as long
  /// as there is no improper straddle.
  /// fieldSize and fieldAlignment are the original size and alignment
  /// calculated without considering the HLSL vector relaxed rule.
  void alignUsingHLSLRelaxedLayout(QualType fieldType, uint32_t fieldSize,
                                   uint32_t fieldAlignment,
                                   uint32_t *currentOffset) const;

  /// \brief Returns true if we use row-major matrix for type. Otherwise,
  /// returns false.
  bool useRowMajor(llvm::Optional<bool> isRowMajor,
                   clang::QualType type) const {
    return isRowMajor.hasValue() ? isRowMajor.getValue()
                                 : isRowMajorMatrix(spvOptions, type);
  }

private:
  /// Emits error to the diagnostic engine associated with this visitor.
  template <unsigned N>
  DiagnosticBuilder emitError(const char (&message)[N],
                              SourceLocation srcLoc = {}) const {
    const auto diagId = astContext.getDiagnostics().getCustomDiagID(
        clang::DiagnosticsEngine::Error, message);
    return astContext.getDiagnostics().Report(srcLoc, diagId);
  }

  /// Emits warning to the diagnostic engine associated with this visitor.
  template <unsigned N>
  DiagnosticBuilder emitWarning(const char (&message)[N],
                                SourceLocation srcLoc = {}) const {
    const auto diagId = astContext.getDiagnostics().getCustomDiagID(
        clang::DiagnosticsEngine::Warning, message);
    return astContext.getDiagnostics().Report(srcLoc, diagId);
  }

  // Returns the alignment and size in bytes for the given struct
  // according to the given LayoutRule.
  std::pair<uint32_t, uint32_t>
  getAlignmentAndSize(QualType type, const RecordType *structType,
                      SpirvLayoutRule rule, llvm::Optional<bool> isRowMajor,
                      uint32_t *stride) const;

private:
  ASTContext &astContext;                /// AST context
  const SpirvCodeGenOptions &spvOptions; /// SPIR-V options
};

} // end namespace spirv
} // end namespace clang

#endif // LLVM_CLANG_LIB_SPIRV_ALIGNMENTSIZECALCULATOR_H
