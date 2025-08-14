//===-- SpirvVisitor.h - SPIR-V Visitor -------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_SPIRV_SPIRVVISITOR_H
#define LLVM_CLANG_SPIRV_SPIRVVISITOR_H

#include "dxc/Support/SPIRVOptions.h"
#include "clang/SPIRV/SpirvInstruction.h"

namespace clang {
namespace spirv {

class SpirvContext;
class SpirvModule;
class SpirvFunction;
class SpirvBasicBlock;

/// \brief The base class for different SPIR-V visitor classes.
/// Each Visitor class serves a specific purpose and should override the
/// suitable visit methods accordingly in order to achieve its purpose.
class Visitor {
public:
  enum Phase {
    Init, //< Before starting the visit of the given construct
    Done, //< After finishing the visit of the given construct
  };

  // Virtual destructor
  virtual ~Visitor() = default;

  // Forbid copy construction and assignment
  Visitor(const Visitor &) = delete;
  Visitor &operator=(const Visitor &) = delete;

  // Forbid move construction and assignment
  Visitor(Visitor &&) = delete;
  Visitor &operator=(Visitor &&) = delete;

  // Visiting different SPIR-V constructs.
  virtual bool visit(SpirvModule *, Phase) { return true; }
  virtual bool visit(SpirvFunction *, Phase) { return true; }
  virtual bool visit(SpirvBasicBlock *, Phase) { return true; }

  /// The "sink" visit function for all instructions.
  ///
  /// By default, all other visit instructions redirect to this visit function.
  /// So that you want override this visit function to handle all instructions,
  /// regardless of their polymorphism.
  virtual bool visitInstruction(SpirvInstruction *) { return true; }

#define DEFINE_VISIT_METHOD(cls)                                               \
  virtual bool visit(cls *i) { return visitInstruction(i); }

  DEFINE_VISIT_METHOD(SpirvCapability)
  DEFINE_VISIT_METHOD(SpirvExtension)
  DEFINE_VISIT_METHOD(SpirvExtInstImport)
  DEFINE_VISIT_METHOD(SpirvMemoryModel)
  DEFINE_VISIT_METHOD(SpirvEntryPoint)
  DEFINE_VISIT_METHOD(SpirvExecutionModeBase)
  DEFINE_VISIT_METHOD(SpirvString)
  DEFINE_VISIT_METHOD(SpirvSource)
  DEFINE_VISIT_METHOD(SpirvModuleProcessed)
  DEFINE_VISIT_METHOD(SpirvDecoration)
  DEFINE_VISIT_METHOD(SpirvVariable)

  DEFINE_VISIT_METHOD(SpirvFunctionParameter)
  DEFINE_VISIT_METHOD(SpirvLoopMerge)
  DEFINE_VISIT_METHOD(SpirvSelectionMerge)
  DEFINE_VISIT_METHOD(SpirvBranching)
  DEFINE_VISIT_METHOD(SpirvBranch)
  DEFINE_VISIT_METHOD(SpirvBranchConditional)
  DEFINE_VISIT_METHOD(SpirvKill)
  DEFINE_VISIT_METHOD(SpirvReturn)
  DEFINE_VISIT_METHOD(SpirvSwitch)
  DEFINE_VISIT_METHOD(SpirvUnreachable)

  DEFINE_VISIT_METHOD(SpirvAccessChain)
  DEFINE_VISIT_METHOD(SpirvAtomic)
  DEFINE_VISIT_METHOD(SpirvBarrier)
  DEFINE_VISIT_METHOD(SpirvIsNodePayloadValid)
  DEFINE_VISIT_METHOD(SpirvNodePayloadArrayLength)
  DEFINE_VISIT_METHOD(SpirvAllocateNodePayloads)
  DEFINE_VISIT_METHOD(SpirvEnqueueNodePayloads)
  DEFINE_VISIT_METHOD(SpirvFinishWritingNodePayload)
  DEFINE_VISIT_METHOD(SpirvBinaryOp)
  DEFINE_VISIT_METHOD(SpirvBitFieldExtract)
  DEFINE_VISIT_METHOD(SpirvBitFieldInsert)
  DEFINE_VISIT_METHOD(SpirvConstantBoolean)
  DEFINE_VISIT_METHOD(SpirvConstantInteger)
  DEFINE_VISIT_METHOD(SpirvConstantFloat)
  DEFINE_VISIT_METHOD(SpirvConstantComposite)
  DEFINE_VISIT_METHOD(SpirvConstantString)
  DEFINE_VISIT_METHOD(SpirvConstantNull)
  DEFINE_VISIT_METHOD(SpirvConvertPtrToU)
  DEFINE_VISIT_METHOD(SpirvConvertUToPtr)
  DEFINE_VISIT_METHOD(SpirvUndef)
  DEFINE_VISIT_METHOD(SpirvCompositeConstruct)
  DEFINE_VISIT_METHOD(SpirvCompositeExtract)
  DEFINE_VISIT_METHOD(SpirvCompositeInsert)
  DEFINE_VISIT_METHOD(SpirvEmitVertex)
  DEFINE_VISIT_METHOD(SpirvEndPrimitive)
  DEFINE_VISIT_METHOD(SpirvExtInst)
  DEFINE_VISIT_METHOD(SpirvFunctionCall)
  DEFINE_VISIT_METHOD(SpirvGroupNonUniformOp)
  DEFINE_VISIT_METHOD(SpirvImageOp)
  DEFINE_VISIT_METHOD(SpirvImageQuery)
  DEFINE_VISIT_METHOD(SpirvImageSparseTexelsResident)
  DEFINE_VISIT_METHOD(SpirvImageTexelPointer)
  DEFINE_VISIT_METHOD(SpirvLoad)
  DEFINE_VISIT_METHOD(SpirvCopyObject)
  DEFINE_VISIT_METHOD(SpirvSampledImage)
  DEFINE_VISIT_METHOD(SpirvSelect)
  DEFINE_VISIT_METHOD(SpirvSpecConstantBinaryOp)
  DEFINE_VISIT_METHOD(SpirvSpecConstantUnaryOp)
  DEFINE_VISIT_METHOD(SpirvStore)
  DEFINE_VISIT_METHOD(SpirvNullaryOp)
  DEFINE_VISIT_METHOD(SpirvUnaryOp)
  DEFINE_VISIT_METHOD(SpirvVectorShuffle)
  DEFINE_VISIT_METHOD(SpirvArrayLength)
  DEFINE_VISIT_METHOD(SpirvRayTracingOpNV)
  DEFINE_VISIT_METHOD(SpirvDemoteToHelperInvocation)
  DEFINE_VISIT_METHOD(SpirvIsHelperInvocationEXT)
  DEFINE_VISIT_METHOD(SpirvDebugInfoNone)
  DEFINE_VISIT_METHOD(SpirvDebugSource)
  DEFINE_VISIT_METHOD(SpirvDebugCompilationUnit)
  DEFINE_VISIT_METHOD(SpirvDebugFunctionDeclaration)
  DEFINE_VISIT_METHOD(SpirvDebugFunction)
  DEFINE_VISIT_METHOD(SpirvDebugFunctionDefinition)
  DEFINE_VISIT_METHOD(SpirvDebugEntryPoint)
  DEFINE_VISIT_METHOD(SpirvDebugLocalVariable)
  DEFINE_VISIT_METHOD(SpirvDebugGlobalVariable)
  DEFINE_VISIT_METHOD(SpirvDebugOperation)
  DEFINE_VISIT_METHOD(SpirvDebugExpression)
  DEFINE_VISIT_METHOD(SpirvDebugDeclare)
  DEFINE_VISIT_METHOD(SpirvDebugLexicalBlock)
  DEFINE_VISIT_METHOD(SpirvDebugScope)
  DEFINE_VISIT_METHOD(SpirvDebugTypeBasic)
  DEFINE_VISIT_METHOD(SpirvDebugTypeArray)
  DEFINE_VISIT_METHOD(SpirvDebugTypeVector)
  DEFINE_VISIT_METHOD(SpirvDebugTypeMatrix)
  DEFINE_VISIT_METHOD(SpirvDebugTypeFunction)
  DEFINE_VISIT_METHOD(SpirvDebugTypeComposite)
  DEFINE_VISIT_METHOD(SpirvDebugTypeMember)
  DEFINE_VISIT_METHOD(SpirvDebugTypeTemplate)
  DEFINE_VISIT_METHOD(SpirvDebugTypeTemplateParameter)

  DEFINE_VISIT_METHOD(SpirvRayQueryOpKHR)
  DEFINE_VISIT_METHOD(SpirvReadClock)
  DEFINE_VISIT_METHOD(SpirvRayTracingTerminateOpKHR)
  DEFINE_VISIT_METHOD(SpirvIntrinsicInstruction)

  DEFINE_VISIT_METHOD(SpirvEmitMeshTasksEXT)
  DEFINE_VISIT_METHOD(SpirvSetMeshOutputsEXT)
#undef DEFINE_VISIT_METHOD

  const SpirvCodeGenOptions &getCodeGenOptions() const { return spvOptions; }

protected:
  explicit Visitor(const SpirvCodeGenOptions &opts, SpirvContext &ctx)
      : spvOptions(opts), context(ctx) {}

protected:
  const SpirvCodeGenOptions &spvOptions;
  SpirvContext &context;
};

} // namespace spirv
} // namespace clang

#endif // LLVM_CLANG_SPIRV_SPIRVVISITOR_H
