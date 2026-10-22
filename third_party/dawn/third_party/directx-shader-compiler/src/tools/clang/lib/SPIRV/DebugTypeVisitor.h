//===--- DebugTypeVisitor.h - Convert AST Type to Debug Type -----*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_DEBUGTYPEVISITOR_H
#define LLVM_CLANG_LIB_SPIRV_DEBUGTYPEVISITOR_H

#include "clang/AST/ASTContext.h"
#include "clang/SPIRV/SpirvContext.h"
#include "clang/SPIRV/SpirvVisitor.h"
#include "llvm/ADT/Optional.h"

namespace clang {
namespace spirv {

class SpirvBuilder;
class LowerTypeVisitor;

/// The class responsible to translate SPIR-V types into DebugType*
/// types as defined in the rich DebugInfo spec.
/// This visitor must be run after the LowerTypeVisitor pass.
class DebugTypeVisitor : public Visitor {
public:
  DebugTypeVisitor(ASTContext &astCtx, SpirvContext &spvCtx,
                   const SpirvCodeGenOptions &opts, SpirvBuilder &builder,
                   LowerTypeVisitor &lowerTypeVisitor)
      : Visitor(opts, spvCtx), astContext(astCtx), spvContext(spvCtx),
        spvBuilder(builder), spvTypeVisitor(lowerTypeVisitor),
        currentDebugInstructionLayoutRule(SpirvLayoutRule::Void) {}

  // Visiting different SPIR-V constructs.
  bool visit(SpirvModule *module, Phase);
  bool visit(SpirvBasicBlock *, Phase) { return true; }
  bool visit(SpirvFunction *, Phase) { return true; }

  /// The "sink" visit function for all instructions.
  ///
  /// By default, all other visit instructions redirect to this visit function.
  /// So that you want override this visit function to handle all instructions,
  /// regardless of their polymorphism.
  bool visitInstruction(SpirvInstruction *);

  using Visitor::visit;

private:
  /// Emits error to the diagnostic engine associated with this visitor.
  template <unsigned N>
  DiagnosticBuilder emitError(const char (&message)[N],
                              SourceLocation srcLoc = {}) {
    const auto diagId = astContext.getDiagnostics().getCustomDiagID(
        clang::DiagnosticsEngine::Error, message);
    return astContext.getDiagnostics().Report(srcLoc, diagId);
  }

  /// Lowers the type of the given instruction to the corresponding SPIR-V debug
  /// type. Adds the debug type instructions to the module.
  ///
  /// The lowering is recursive. All the debug types that the target type
  /// depends on will also be created.
  SpirvDebugType *lowerToDebugType(const SpirvType *);

  /// Lowers DebugTypeComposite.
  SpirvDebugType *lowerToDebugTypeComposite(const SpirvType *);

  /// Creates DebugTypeComposite for a struct type.
  SpirvDebugTypeComposite *createDebugTypeComposite(const SpirvType *type,
                                                    const SourceLocation &loc,
                                                    uint32_t tag);

  /// Adds DebugTypeMembers for member variables to DebugTypeComposite.
  void addDebugTypeForMemberVariables(
      SpirvDebugTypeComposite *debugTypeComposite, const StructType *type,
      llvm::function_ref<SourceLocation()> location, unsigned numBases);

  /// Lowers DebugTypeMembers of DebugTypeComposite.
  void lowerDebugTypeMembers(SpirvDebugTypeComposite *debugTypeComposite,
                             const StructType *type, const DeclContext *decl);

  /// Lowers DebugTypeTemplate for composite type.
  SpirvDebugTypeTemplate *
  lowerDebugTypeTemplate(const ClassTemplateSpecializationDecl *templateDecl,
                         SpirvDebugTypeComposite *debugTypeComposite);

  /// Set the result type of debug instructions to OpTypeVoid.
  /// According to the rich DebugInfo spec, all debug instructions are
  /// OpExtInst with result type of void.
  void setDefaultDebugInfo(SpirvDebugInstruction *instr);

  SpirvDebugInfoNone *getDebugInfoNone();

private:
  ASTContext &astContext;           /// AST context
  SpirvContext &spvContext;         /// SPIR-V context
  SpirvBuilder &spvBuilder;         ///< SPIR-V builder
  LowerTypeVisitor &spvTypeVisitor; /// QualType to SPIR-V type visitor

  SpirvLayoutRule currentDebugInstructionLayoutRule; /// SPIR-V layout rule
};

} // end namespace spirv
} // end namespace clang

#endif // LLVM_CLANG_LIB_SPIRV_DEBUGTYPEVISITOR_H
