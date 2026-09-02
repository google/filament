//===--- LowerTypeVisitor.h - AST type to SPIR-V type visitor ----*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_LOWERTYPEVISITOR_H
#define LLVM_CLANG_LIB_SPIRV_LOWERTYPEVISITOR_H

#include "AlignmentSizeCalculator.h"
#include "clang/AST/ASTContext.h"
#include "clang/SPIRV/SpirvBuilder.h"
#include "clang/SPIRV/SpirvContext.h"
#include "clang/SPIRV/SpirvVisitor.h"
#include "llvm/ADT/Optional.h"

namespace clang {
namespace spirv {

/// The class responsible to translate Clang frontend types into SPIR-V types.
class LowerTypeVisitor : public Visitor {
public:
  LowerTypeVisitor(ASTContext &astCtx, SpirvContext &spvCtx,
                   const SpirvCodeGenOptions &opts, SpirvBuilder &builder)
      : Visitor(opts, spvCtx), astContext(astCtx), spvContext(spvCtx),
        alignmentCalc(astCtx, opts), useArrayForMat1xN(false),
        spvBuilder(builder) {}

  // Visiting different SPIR-V constructs.
  bool visit(SpirvModule *, Phase) override { return true; }
  bool visit(SpirvFunction *, Phase) override;
  bool visit(SpirvBasicBlock *, Phase) override { return true; }

  using Visitor::visit;

  /// The "sink" visit function for all instructions.
  ///
  /// By default, all other visit instructions redirect to this visit function.
  /// So that you want override this visit function to handle all instructions,
  /// regardless of their polymorphism.
  bool visitInstruction(SpirvInstruction *instr) override;

  /// Lowers the given AST QualType into the corresponding SPIR-V type.
  ///
  /// The lowering is recursive; all the types that the target type depends
  /// on will be created in SpirvContext.
  const SpirvType *lowerType(QualType type, SpirvLayoutRule,
                             llvm::Optional<bool> isRowMajor, SourceLocation);

  bool useSpvArrayForHlslMat1xN() { return useArrayForMat1xN; }

private:
  /// Emits error to the diagnostic engine associated with this visitor.
  template <unsigned N>
  DiagnosticBuilder emitError(const char (&message)[N],
                              SourceLocation srcLoc = {}) {
    const auto diagId = astContext.getDiagnostics().getCustomDiagID(
        clang::DiagnosticsEngine::Error, message);
    return astContext.getDiagnostics().Report(srcLoc, diagId);
  }

  // This method sorts a field list in the following order:
  //  - fields with register annotation first, sorted by register index.
  //  - then fields without annotation, in order of declaration.
  std::vector<const HybridStructType::FieldInfo *>
  sortFields(llvm::ArrayRef<HybridStructType::FieldInfo> fields);

  /// Lowers the given Hybrid type into a SPIR-V type.
  ///
  /// Uses the above lowerType method to lower the QualType components of hybrid
  /// types.
  const SpirvType *lowerType(const SpirvType *, SpirvLayoutRule,
                             SourceLocation);

  /// Lowers the given HLSL resource type into its SPIR-V type.
  const SpirvType *lowerResourceType(QualType type, SpirvLayoutRule rule,
                                     llvm::Optional<bool> isRowMajor,
                                     SourceLocation);

  /// Lowers the fields of a RecordDecl into SPIR-V StructType field
  /// information.
  llvm::SmallVector<StructType::FieldInfo, 4>
  lowerStructFields(const RecordDecl *structType, SpirvLayoutRule rule);

  /// Creates the default AST type from a TemplateName for HLSL templates
  /// which have optional parameters (e.g. Texture2D).
  QualType createASTTypeFromTemplateName(TemplateName templateName);

  /// If the given type is an integral_constant or a Literal<integral_constant>,
  /// return the constant value as a SpirvConstant, which will be set as a
  /// literal constant if wrapped in Literal.
  bool getVkIntegralConstantValue(QualType type, SpirvConstant *&result,
                                  SourceLocation srcLoc);

  /// Lowers the given vk::SpirvType or vk::SpirvOpaqueType into its SPIR-V
  /// type.
  const SpirvType *
  lowerInlineSpirvType(llvm::StringRef name, unsigned int opcode,
                       const ClassTemplateSpecializationDecl *specDecl,
                       SpirvLayoutRule rule, llvm::Optional<bool> isRowMajor,
                       SourceLocation srcLoc);

  /// Lowers the given type defined in vk namespace into its SPIR-V type.
  const SpirvType *lowerVkTypeInVkNamespace(QualType type, llvm::StringRef name,
                                            SpirvLayoutRule rule,
                                            llvm::Optional<bool> isRowMajor,
                                            SourceLocation srcLoc);

  /// For the given sampled type, returns the corresponding image format
  /// that can be used to create an image object.
  spv::ImageFormat translateSampledTypeToImageFormat(QualType sampledType,
                                                     SourceLocation);

private:
  /// Calculates all layout information needed for the given structure fields.
  /// Returns the lowered field info vector.
  /// In other words: lowers the HybridStructType field information to
  /// StructType field information.
  llvm::SmallVector<StructType::FieldInfo, 4>
  populateLayoutInformation(llvm::ArrayRef<HybridStructType::FieldInfo> fields,
                            SpirvLayoutRule rule);

  /// Create a clang::StructType::FieldInfo from HybridStructType::FieldInfo.
  /// This function only considers the field as standalone.
  /// Offset and layout constraint from the parent struct are not considered.
  StructType::FieldInfo lowerField(const HybridStructType::FieldInfo *field,
                                   SpirvLayoutRule rule,
                                   const uint32_t fieldIndex);

  /// Get a lowered SpirvPointer from the args to a SpirvOpaqueType.
  /// The pointer will use the given layout rule. `isRowMajor` is used to
  /// lower the pointee type.
  const SpirvType *getSpirvPointerFromInlineSpirvType(
      ArrayRef<TemplateArgument> args, SpirvLayoutRule rule,
      Optional<bool> isRowMajor, SourceLocation location);

private:
  ASTContext &astContext;                /// AST context
  SpirvContext &spvContext;              /// SPIR-V context
  AlignmentSizeCalculator alignmentCalc; /// alignment calculator
  bool useArrayForMat1xN;                /// SPIR-V array for HLSL Matrix 1xN
  SpirvBuilder &spvBuilder;
  SmallVector<QualType, 4> visitedTypeStack; // for type recursion detection
};

} // end namespace spirv
} // end namespace clang

#endif // LLVM_CLANG_LIB_SPIRV_LOWERTYPEVISITOR_H
