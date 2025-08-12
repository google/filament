//===-- SpirvInstruction.h - SPIR-V Instruction -----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_SPIRV_SPIRVINSTRUCTION_H
#define LLVM_CLANG_SPIRV_SPIRVINSTRUCTION_H

#include "dxc/Support/SPIRVOptions.h"
#include "spirv/unified1/GLSL.std.450.h"
#include "spirv/unified1/spirv.hpp11"
#include "clang/AST/APValue.h"
#include "clang/AST/Type.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/SPIRV/SpirvType.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"

namespace clang {
namespace spirv {

class BoolType;
class FloatType;
class IntegerType;
class SpirvBasicBlock;
class SpirvFunction;
class SpirvType;
class SpirvVariable;
class SpirvString;
class Visitor;
class DebugTypeComposite;
class SpirvDebugDeclare;
class FunctionType;

#define DEFINE_RELEASE_MEMORY_FOR_CLASS(cls)                                   \
  void releaseMemory() override { this->~cls(); }

/// \brief The base class for representing SPIR-V instructions.
class SpirvInstruction {
public:
  enum Kind {
    // "Metadata" kinds
    // In the order of logical layout

    IK_Capability,      // OpCapability
    IK_Extension,       // OpExtension
    IK_ExtInstImport,   // OpExtInstImport
    IK_MemoryModel,     // OpMemoryModel
    IK_EntryPoint,      // OpEntryPoint
    IK_ExecutionMode,   // OpExecutionMode
    IK_ExecutionModeId, // OpExecutionModeId
    IK_String,          // OpString (debug)
    IK_Source,          // OpSource (debug)
    IK_ModuleProcessed, // OpModuleProcessed (debug)
    IK_Decoration,      // Op*Decorate
    IK_Type,            // OpType*
    IK_Variable,        // OpVariable

    // Different kind of constants. Order matters.
    IK_ConstantBoolean,
    IK_ConstantInteger,
    IK_ConstantFloat,
    IK_ConstantComposite,
    IK_ConstantString,
    IK_ConstantNull,

    // Pointer <-> uint conversions.
    IK_ConvertPtrToU,
    IK_ConvertUToPtr,

    // OpUndef
    IK_Undef,

    // Function structure kinds

    IK_FunctionParameter, // OpFunctionParameter

    // The following section is for merge instructions.
    // Used by LLVM-style RTTI; order matters.
    IK_LoopMerge,      // OpLoopMerge
    IK_SelectionMerge, // OpSelectionMerge

    // The following section is for termination instructions.
    // Used by LLVM-style RTTI; order matters.
    IK_Branch,              // OpBranch
    IK_BranchConditional,   // OpBranchConditional
    IK_Kill,                // OpKill
    IK_Return,              // OpReturn*
    IK_Switch,              // OpSwitch
    IK_Unreachable,         // OpUnreachable
    IK_RayTracingTerminate, // OpIgnoreIntersectionKHR/OpTerminateRayKHR
    IK_EmitMeshTasksEXT,    // OpEmitMeshTasksEXT

    // Normal instruction kinds
    // In alphabetical order

    IK_AccessChain,              // OpAccessChain
    IK_ArrayLength,              // OpArrayLength
    IK_Atomic,                   // OpAtomic*
    IK_Barrier,                  // Op*Barrier
    IK_BinaryOp,                 // Binary operations
    IK_BitFieldExtract,          // OpBitFieldExtract
    IK_BitFieldInsert,           // OpBitFieldInsert
    IK_CompositeConstruct,       // OpCompositeConstruct
    IK_CompositeExtract,         // OpCompositeExtract
    IK_CompositeInsert,          // OpCompositeInsert
    IK_CopyObject,               // OpCopyObject
    IK_DemoteToHelperInvocation, // OpDemoteToHelperInvocation
    IK_IsHelperInvocationEXT,    // OpIsHelperInvocationEXT
    IK_ExtInst,                  // OpExtInst
    IK_FunctionCall,             // OpFunctionCall

    IK_EndPrimitive, // OpEndPrimitive
    IK_EmitVertex,   // OpEmitVertex

    IK_SetMeshOutputsEXT, // OpSetMeshOutputsEXT

    IK_GroupNonUniformOp, // Group non-uniform operations

    IK_ImageOp,                   // OpImage*
    IK_ImageQuery,                // OpImageQuery*
    IK_ImageSparseTexelsResident, // OpImageSparseTexelsResident
    IK_ImageTexelPointer,         // OpImageTexelPointer
    IK_Load,                      // OpLoad
    IK_RayQueryOpKHR,             // KHR rayquery ops
    IK_RayTracingOpNV,            // NV raytracing ops
    IK_ReadClock,                 // OpReadClock
    IK_SampledImage,              // OpSampledImage
    IK_Select,                    // OpSelect
    IK_SpecConstantBinaryOp,      // SpecConstant binary operations
    IK_SpecConstantUnaryOp,       // SpecConstant unary operations
    IK_Store,                     // OpStore
    IK_UnaryOp,                   // Unary operations
    IK_NullaryOp,                 // Nullary operations
    IK_VectorShuffle,             // OpVectorShuffle
    IK_SpirvIntrinsicInstruction, // Spirv Intrinsic Instructions

    // For DebugInfo instructions defined in
    // OpenCL.DebugInfo.100 and NonSemantic.Shader.DebugInfo.100
    IK_DebugInfoNone,
    IK_DebugCompilationUnit,
    IK_DebugSource,
    IK_DebugFunctionDecl,
    IK_DebugFunction,
    IK_DebugFunctionDef,
    IK_DebugEntryPoint,
    IK_DebugLocalVariable,
    IK_DebugGlobalVariable,
    IK_DebugOperation,
    IK_DebugExpression,
    IK_DebugDeclare,
    IK_DebugLexicalBlock,
    IK_DebugScope,
    IK_DebugTypeBasic,
    IK_DebugTypeArray,
    IK_DebugTypeVector,
    IK_DebugTypeMatrix,
    IK_DebugTypeFunction,
    IK_DebugTypeComposite,
    IK_DebugTypeMember,
    IK_DebugTypeTemplate,
    IK_DebugTypeTemplateParameter,

    // For workgraph instructions
    IK_IsNodePayloadValid,
    IK_NodePayloadArrayLength,
    IK_AllocateNodePayloads,
    IK_EnqueueNodePayloads,
    IK_FinishWritingNodePayload,
  };

  // All instruction classes should include a releaseMemory method.
  // This is needed in order to avoid leaking memory for classes that include
  // members that are not trivially destructible.
  virtual void releaseMemory() = 0;

  virtual ~SpirvInstruction() = default;

  // Invokes SPIR-V visitor on this instruction.
  virtual bool invokeVisitor(Visitor *) = 0;

  // Replace operands with a function callable reference, and if needed,
  // refresh instrunctions result type. If current visitor is in entry
  // function wrapper, avoid refresh its AST result type.
  virtual void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper){};

  Kind getKind() const { return kind; }
  spv::Op getopcode() const { return opcode; }
  QualType getAstResultType() const { return astResultType; }
  void setAstResultType(QualType type) { astResultType = type; }
  bool hasAstResultType() const { return astResultType != QualType(); }

  uint32_t getResultTypeId() const { return resultTypeId; }
  void setResultTypeId(uint32_t id) { resultTypeId = id; }

  bool hasResultType() const { return resultType != nullptr; }
  const SpirvType *getResultType() const { return resultType; }
  void setResultType(const SpirvType *type) { resultType = type; }

  // TODO: The responsibility of assigning the result-id of an instruction
  // shouldn't be on the instruction itself.
  uint32_t getResultId() const { return resultId; }
  void setResultId(uint32_t id) { resultId = id; }

  clang::SourceLocation getSourceLocation() const { return srcLoc; }
  clang::SourceRange getSourceRange() const { return srcRange; }

  void setDebugName(llvm::StringRef name) { debugName = name; }
  llvm::StringRef getDebugName() const { return debugName; }

  bool isArithmeticInstruction() const;

  SpirvLayoutRule getLayoutRule() const { return layoutRule; }
  void setLayoutRule(SpirvLayoutRule rule) { layoutRule = rule; }

  void setStorageClass(spv::StorageClass sc) { storageClass = sc; }
  spv::StorageClass getStorageClass() const { return storageClass; }

  void setRValue(bool rvalue = true) { isRValue_ = rvalue; }
  bool isRValue() const { return isRValue_; }
  bool isLValue() const { return !isRValue_; }

  void setRelaxedPrecision() { isRelaxedPrecision_ = true; }
  bool isRelaxedPrecision() const { return isRelaxedPrecision_; }

  void setNonUniform(bool nu = true) { isNonUniform_ = nu; }
  bool isNonUniform() const { return isNonUniform_; }

  void setPrecise(bool p = true) { isPrecise_ = p; }
  bool isPrecise() const { return isPrecise_; }

  void setNoninterpolated(bool ni = true) { isNoninterpolated_ = ni; }
  bool isNoninterpolated() const { return isNoninterpolated_; }

  void setBitfieldInfo(const BitfieldInfo &info) { bitfieldInfo = info; }
  llvm::Optional<BitfieldInfo> getBitfieldInfo() const { return bitfieldInfo; }

  void setRasterizerOrdered(bool ro = true) { isRasterizerOrdered_ = ro; }
  bool isRasterizerOrdered() const { return isRasterizerOrdered_; }

  /// Legalization-specific code
  ///
  /// Note: the following two functions are currently needed in order to support
  /// aliasing.
  ///
  /// TODO: Clean up aliasing and try to move it to a separate pass.
  void setContainsAliasComponent(bool contains) { containsAlias = contains; }
  bool containsAliasComponent() const { return containsAlias; }

protected:
  // Forbid creating SpirvInstruction directly
  SpirvInstruction(Kind kind, spv::Op opcode, QualType astResultType,
                   SourceLocation loc, SourceRange range = {});

protected:
  const Kind kind;

  spv::Op opcode;
  QualType astResultType;
  uint32_t resultId;
  SourceLocation srcLoc;
  SourceRange srcRange;
  std::string debugName;
  const SpirvType *resultType;
  uint32_t resultTypeId;
  SpirvLayoutRule layoutRule;

  /// Indicates whether this evaluation result contains alias variables
  ///
  /// This field should only be true for stand-alone alias variables, which is
  /// of pointer-to-pointer type, or struct variables containing alias fields.
  /// After dereferencing the alias variable, this should be set to false to let
  /// CodeGen fall back to normal handling path.
  ///
  /// Note: legalization specific code
  bool containsAlias;

  spv::StorageClass storageClass;
  bool isRValue_;
  bool isRelaxedPrecision_;
  bool isNonUniform_;
  bool isPrecise_;
  bool isNoninterpolated_;
  llvm::Optional<BitfieldInfo> bitfieldInfo;
  bool isRasterizerOrdered_;
};

/// \brief OpCapability instruction
class SpirvCapability : public SpirvInstruction {
public:
  SpirvCapability(SourceLocation loc, spv::Capability cap);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvCapability)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_Capability;
  }

  bool invokeVisitor(Visitor *v) override;

  bool operator==(const SpirvCapability &that) const;

  spv::Capability getCapability() const { return capability; }

private:
  spv::Capability capability;
};

/// \brief Extension instruction
class SpirvExtension : public SpirvInstruction {
public:
  SpirvExtension(SourceLocation loc, llvm::StringRef extensionName);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvExtension)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_Extension;
  }

  bool invokeVisitor(Visitor *v) override;

  bool operator==(const SpirvExtension &that) const;

  llvm::StringRef getExtensionName() const { return extName; }

private:
  std::string extName;
};

/// \brief ExtInstImport instruction
class SpirvExtInstImport : public SpirvInstruction {
public:
  SpirvExtInstImport(SourceLocation loc, llvm::StringRef extensionName);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvExtInstImport)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ExtInstImport;
  }

  bool invokeVisitor(Visitor *v) override;

  llvm::StringRef getExtendedInstSetName() const { return extName; }

private:
  std::string extName;
};

/// \brief OpMemoryModel instruction
class SpirvMemoryModel : public SpirvInstruction {
public:
  SpirvMemoryModel(spv::AddressingModel addrModel, spv::MemoryModel memModel);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvMemoryModel)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_MemoryModel;
  }

  bool invokeVisitor(Visitor *v) override;

  spv::AddressingModel getAddressingModel() const { return addressModel; }
  spv::MemoryModel getMemoryModel() const { return memoryModel; }
  void setAddressingModel(spv::AddressingModel addrModel) {
    addressModel = addrModel;
  }

private:
  spv::AddressingModel addressModel;
  spv::MemoryModel memoryModel;
};

/// \brief OpEntryPoint instruction
class SpirvEntryPoint : public SpirvInstruction {
public:
  SpirvEntryPoint(SourceLocation loc, spv::ExecutionModel executionModel,
                  SpirvFunction *entryPoint, llvm::StringRef nameStr,
                  llvm::ArrayRef<SpirvVariable *> iface);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvEntryPoint)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_EntryPoint;
  }

  bool invokeVisitor(Visitor *v) override;

  spv::ExecutionModel getExecModel() const { return execModel; }
  SpirvFunction *getEntryPoint() const { return entryPoint; }
  llvm::StringRef getEntryPointName() const { return name; }
  llvm::ArrayRef<SpirvVariable *> getInterface() const { return interfaceVec; }

private:
  spv::ExecutionModel execModel;
  SpirvFunction *entryPoint;
  std::string name;
  llvm::SmallVector<SpirvVariable *, 8> interfaceVec;
};

class SpirvExecutionModeBase : public SpirvInstruction {
public:
  SpirvExecutionModeBase(Kind kind, spv::Op opcode, SourceLocation loc,
                         SpirvFunction *entryPointFunction,
                         spv::ExecutionMode executionMode)
      : SpirvInstruction(kind, opcode, QualType(), loc),
        entryPoint(entryPointFunction), execMode(executionMode) {}

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvExecutionModeBase)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) { return false; }

  bool invokeVisitor(Visitor *v) override;

  SpirvFunction *getEntryPoint() const { return entryPoint; }
  spv::ExecutionMode getExecutionMode() const { return execMode; }

private:
  SpirvFunction *entryPoint;
  spv::ExecutionMode execMode;
};

/// \brief OpExecutionMode and OpExecutionModeId instructions
class SpirvExecutionMode : public SpirvExecutionModeBase {
public:
  SpirvExecutionMode(SourceLocation loc, SpirvFunction *entryPointFunction,
                     spv::ExecutionMode, llvm::ArrayRef<uint32_t> params);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvExecutionMode)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ExecutionMode;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvFunction *getEntryPoint() const { return entryPoint; }
  spv::ExecutionMode getExecutionMode() const { return execMode; }
  llvm::ArrayRef<uint32_t> getParams() const { return params; }

private:
  SpirvFunction *entryPoint;
  spv::ExecutionMode execMode;
  llvm::SmallVector<uint32_t, 4> params;
};

/// \brief OpExecutionModeId
class SpirvExecutionModeId : public SpirvExecutionModeBase {
public:
  SpirvExecutionModeId(SourceLocation loc, SpirvFunction *entryPointFunction,
                       spv::ExecutionMode em,
                       llvm::ArrayRef<SpirvInstruction *> params);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvExecutionModeId)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ExecutionModeId;
  }

  bool invokeVisitor(Visitor *v) override;

  llvm::ArrayRef<SpirvInstruction *> getParams() const { return params; }

private:
  llvm::SmallVector<SpirvInstruction *, 4> params;
};

/// \brief OpString instruction
class SpirvString : public SpirvInstruction {
public:
  SpirvString(SourceLocation loc, llvm::StringRef stringLiteral);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvString)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_String;
  }

  bool invokeVisitor(Visitor *v) override;

  llvm::StringRef getString() const { return str; }

private:
  std::string str;
};

/// \brief OpSource and OpSourceContinued instruction
class SpirvSource : public SpirvInstruction {
public:
  SpirvSource(SourceLocation loc, spv::SourceLanguage language, uint32_t ver,
              SpirvString *file, llvm::StringRef src);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvSource)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_Source;
  }

  bool invokeVisitor(Visitor *v) override;

  spv::SourceLanguage getSourceLanguage() const { return lang; }
  uint32_t getVersion() const { return version; }
  bool hasFile() const { return file != nullptr; }
  SpirvString *getFile() const { return file; }
  llvm::StringRef getSource() const { return source; }

private:
  spv::SourceLanguage lang;
  uint32_t version;
  SpirvString *file;
  std::string source;
};

/// \brief OpModuleProcessed instruction
class SpirvModuleProcessed : public SpirvInstruction {
public:
  SpirvModuleProcessed(SourceLocation loc, llvm::StringRef processStr);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvModuleProcessed)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ModuleProcessed;
  }

  bool invokeVisitor(Visitor *v) override;

  llvm::StringRef getProcess() const { return process; }

private:
  std::string process;
};

/// \brief OpDecorate(Id) and OpMemberDecorate instructions
class SpirvDecoration : public SpirvInstruction {
public:
  // OpDecorate/OpMemberDecorate
  SpirvDecoration(SourceLocation loc, SpirvInstruction *target,
                  spv::Decoration decor, llvm::ArrayRef<uint32_t> params = {},
                  llvm::Optional<uint32_t> index = llvm::None);

  // OpDecorateString/OpMemberDecorateString
  SpirvDecoration(SourceLocation loc, SpirvInstruction *target,
                  spv::Decoration decor,
                  llvm::ArrayRef<llvm::StringRef> stringParam,
                  llvm::Optional<uint32_t> index = llvm::None);

  // Used for creating OpDecorateId instructions
  SpirvDecoration(SourceLocation loc, SpirvInstruction *target,
                  spv::Decoration decor,
                  llvm::ArrayRef<SpirvInstruction *> params);

  SpirvDecoration(SourceLocation loc, SpirvFunction *targetFunc,
                  spv::Decoration decor, llvm::ArrayRef<uint32_t> params);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDecoration)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_Decoration;
  }

  bool invokeVisitor(Visitor *v) override;

  bool operator==(const SpirvDecoration &that) const;

  // Returns the instruction that is the target of the decoration.
  SpirvInstruction *getTarget() const { return target; }
  SpirvFunction *getTargetFunc() const { return targetFunction; }
  spv::Decoration getDecoration() const { return decoration; }
  llvm::ArrayRef<uint32_t> getParams() const { return params; }
  llvm::ArrayRef<SpirvInstruction *> getIdParams() const { return idParams; }
  bool isMemberDecoration() const { return index.hasValue(); }
  uint32_t getMemberIndex() const { return index.getValue(); }

private:
  spv::Op getDecorateOpcode(spv::Decoration,
                            const llvm::Optional<uint32_t> &memberIndex);
  spv::Op getDecorateStringOpcode(bool isMemberDecoration);

private:
  SpirvInstruction *target;
  SpirvFunction *targetFunction;
  spv::Decoration decoration;
  llvm::Optional<uint32_t> index;
  llvm::SmallVector<uint32_t, 4> params;
  llvm::SmallVector<SpirvInstruction *, 4> idParams;
};

/// \brief OpVariable instruction
class SpirvVariable : public SpirvInstruction {
public:
  SpirvVariable(QualType resultType, SourceLocation loc, spv::StorageClass sc,
                bool isPrecise, bool isNointerp,
                SpirvInstruction *initializerId = 0);

  SpirvVariable(const SpirvType *spvType, SourceLocation loc,
                spv::StorageClass sc, bool isPrecise, bool isNointerp,
                SpirvInstruction *initializerId = 0);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvVariable)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_Variable;
  }

  bool invokeVisitor(Visitor *v) override;

  bool hasInitializer() const { return initializer != nullptr; }
  SpirvInstruction *getInitializer() const { return initializer; }
  bool hasBinding() const { return descriptorSet >= 0 || binding >= 0; }
  llvm::StringRef getHlslUserType() const { return hlslUserType; }

  void setDescriptorSetNo(int32_t dset) { descriptorSet = dset; }
  void setBindingNo(int32_t b) { binding = b; }
  void setHlslUserType(llvm::StringRef userType) { hlslUserType = userType; }

private:
  SpirvInstruction *initializer;
  int32_t descriptorSet;
  int32_t binding;
  std::string hlslUserType;
};

class SpirvFunctionParameter : public SpirvInstruction {
public:
  SpirvFunctionParameter(QualType resultType, bool isPrecise, bool isNointerp,
                         SourceLocation loc);

  SpirvFunctionParameter(const SpirvType *spvType, bool isPrecise,
                         bool isNointerp, SourceLocation loc);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvFunctionParameter)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_FunctionParameter;
  }

  bool invokeVisitor(Visitor *v) override;

  void setDebugDeclare(SpirvDebugDeclare *decl) { debugDecl = decl; }
  SpirvDebugDeclare *getDebugDeclare() const { return debugDecl; }

private:
  // When we turn on the rich debug info generation option, we want
  // to keep the function parameter information (like a local
  // variable). Since DebugDeclare instruction maps a
  // DebugLocalVariable instruction to OpFunctionParameter instruction,
  // we keep a pointer to SpirvDebugDeclare in SpirvFunctionParameter.
  SpirvDebugDeclare *debugDecl;
};

/// \brief Merge instructions include OpLoopMerge and OpSelectionMerge
class SpirvMerge : public SpirvInstruction {
public:
  SpirvBasicBlock *getMergeBlock() const { return mergeBlock; }

protected:
  SpirvMerge(Kind kind, spv::Op opcode, SourceLocation loc,
             SpirvBasicBlock *mergeBlock, SourceRange range = {});

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_LoopMerge ||
           inst->getKind() == IK_SelectionMerge;
  }

private:
  SpirvBasicBlock *mergeBlock;
};

class SpirvLoopMerge : public SpirvMerge {
public:
  SpirvLoopMerge(SourceLocation loc, SpirvBasicBlock *mergeBlock,
                 SpirvBasicBlock *contTarget, spv::LoopControlMask mask,
                 SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvLoopMerge)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_LoopMerge;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvBasicBlock *getContinueTarget() const { return continueTarget; }
  spv::LoopControlMask getLoopControlMask() const { return loopControlMask; }

private:
  SpirvBasicBlock *continueTarget;
  spv::LoopControlMask loopControlMask;
};

class SpirvSelectionMerge : public SpirvMerge {
public:
  SpirvSelectionMerge(SourceLocation loc, SpirvBasicBlock *mergeBlock,
                      spv::SelectionControlMask mask, SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvSelectionMerge)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_SelectionMerge;
  }

  bool invokeVisitor(Visitor *v) override;

  spv::SelectionControlMask getSelectionControlMask() const {
    return selControlMask;
  }

private:
  spv::SelectionControlMask selControlMask;
};

/// \brief Termination instructions are instructions that end a basic block.
///
/// These instructions include:
///
/// * OpBranch, OpBranchConditional, OpSwitch
/// * OpReturn, OpReturnValue, OpKill, OpUnreachable
/// * OpIgnoreIntersectionKHR, OpTerminateIntersectionKHR
///
/// The first group (branching instructions) also include information on
/// possible branches that will be taken next.
class SpirvTerminator : public SpirvInstruction {
public:
  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() >= IK_Branch &&
           inst->getKind() <= IK_EmitMeshTasksEXT;
  }

protected:
  SpirvTerminator(Kind kind, spv::Op opcode, SourceLocation loc,
                  SourceRange range = {});
};

/// \brief Base class for branching instructions
class SpirvBranching : public SpirvTerminator {
public:
  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() >= IK_Branch &&
           inst->getKind() <= IK_BranchConditional;
  }

  virtual llvm::ArrayRef<SpirvBasicBlock *> getTargetBranches() const = 0;

protected:
  SpirvBranching(Kind kind, spv::Op opcode, SourceLocation loc,
                 SourceRange range = {});
};

/// \brief OpBranch instruction
class SpirvBranch : public SpirvBranching {
public:
  SpirvBranch(SourceLocation loc, SpirvBasicBlock *target,
              SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvBranch)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_Branch;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvBasicBlock *getTargetLabel() const { return targetLabel; }

  // Returns all possible basic blocks that could be taken by the branching
  // instruction.
  llvm::ArrayRef<SpirvBasicBlock *> getTargetBranches() const override {
    return {targetLabel};
  }

private:
  SpirvBasicBlock *targetLabel;
};

/// \brief OpBranchConditional instruction
class SpirvBranchConditional : public SpirvBranching {
public:
  SpirvBranchConditional(SourceLocation loc, SpirvInstruction *cond,
                         SpirvBasicBlock *trueLabel,
                         SpirvBasicBlock *falseLabel);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvBranchConditional)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_BranchConditional;
  }

  bool invokeVisitor(Visitor *v) override;

  llvm::ArrayRef<SpirvBasicBlock *> getTargetBranches() const override {
    return {trueLabel, falseLabel};
  }

  SpirvInstruction *getCondition() const { return condition; }
  SpirvBasicBlock *getTrueLabel() const { return trueLabel; }
  SpirvBasicBlock *getFalseLabel() const { return falseLabel; }
  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    condition = remapOp(condition);
  }

private:
  SpirvInstruction *condition;
  SpirvBasicBlock *trueLabel;
  SpirvBasicBlock *falseLabel;
};

/// \brief OpKill instruction
class SpirvKill : public SpirvTerminator {
public:
  SpirvKill(SourceLocation loc, SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvKill)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_Kill;
  }

  bool invokeVisitor(Visitor *v) override;
};

/// \brief OpReturn and OpReturnValue instructions
class SpirvReturn : public SpirvTerminator {
public:
  SpirvReturn(SourceLocation loc, SpirvInstruction *retVal = 0,
              SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvReturn)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_Return;
  }

  bool invokeVisitor(Visitor *v) override;

  bool hasReturnValue() const { return returnValue != 0; }
  SpirvInstruction *getReturnValue() const { return returnValue; }
  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    returnValue = remapOp(returnValue);
  }

private:
  SpirvInstruction *returnValue;
};

/// \brief Switch instruction
class SpirvSwitch : public SpirvBranching {
public:
  SpirvSwitch(
      SourceLocation loc, SpirvInstruction *selector,
      SpirvBasicBlock *defaultLabel,
      llvm::ArrayRef<std::pair<llvm::APInt, SpirvBasicBlock *>> &targetsVec);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvSwitch)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_Switch;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getSelector() const { return selector; }
  SpirvBasicBlock *getDefaultLabel() const { return defaultLabel; }
  llvm::MutableArrayRef<std::pair<llvm::APInt, SpirvBasicBlock *>>
  getTargets() {
    return targets;
  }
  // Returns the branch label that will be taken for the given literal.
  SpirvBasicBlock *getTargetLabelForLiteral(uint32_t) const;
  // Returns all possible branches that could be taken by the switch statement.
  llvm::ArrayRef<SpirvBasicBlock *> getTargetBranches() const override;
  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    selector = remapOp(selector);
  }

private:
  SpirvInstruction *selector;
  SpirvBasicBlock *defaultLabel;
  llvm::SmallVector<std::pair<llvm::APInt, SpirvBasicBlock *>, 4> targets;
};

/// \brief OpUnreachable instruction
class SpirvUnreachable : public SpirvTerminator {
public:
  SpirvUnreachable(SourceLocation loc);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvUnreachable)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_Unreachable;
  }

  bool invokeVisitor(Visitor *v) override;
};

/// \brief Access Chain instruction representation (OpAccessChain)
///
/// Note: If needed, this class can be extended to cover Ptr access chains,
/// and InBounds access chains. These are currently not used by CodeGen.
class SpirvAccessChain : public SpirvInstruction {
public:
  SpirvAccessChain(QualType resultType, SourceLocation loc,
                   SpirvInstruction *base,
                   llvm::ArrayRef<SpirvInstruction *> indexVec,
                   SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvAccessChain)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_AccessChain;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getBase() const { return base; }
  llvm::ArrayRef<SpirvInstruction *> getIndexes() const { return indices; }
  void insertIndex(SpirvInstruction *i, uint32_t index) {
    if (index < indices.size())
      indices.insert(&indices[index], i);
    else if (index == indices.size())
      indices.push_back(i);
  }

private:
  SpirvInstruction *base;
  llvm::SmallVector<SpirvInstruction *, 4> indices;
};

/// \brief Atomic instructions.
///
/// This class includes:
/// OpAtomicLoad           (pointer, scope, memorysemantics)
/// OpAtomicIncrement      (pointer, scope, memorysemantics)
/// OpAtomicDecrement      (pointer, scope, memorysemantics)
/// OpAtomicFlagClear      (pointer, scope, memorysemantics)
/// OpAtomicFlagTestAndSet (pointer, scope, memorysemantics)
/// OpAtomicStore          (pointer, scope, memorysemantics, value)
/// OpAtomicAnd            (pointer, scope, memorysemantics, value)
/// OpAtomicOr             (pointer, scope, memorysemantics, value)
/// OpAtomicXor            (pointer, scope, memorysemantics, value)
/// OpAtomicIAdd           (pointer, scope, memorysemantics, value)
/// OpAtomicISub           (pointer, scope, memorysemantics, value)
/// OpAtomicSMin           (pointer, scope, memorysemantics, value)
/// OpAtomicUMin           (pointer, scope, memorysemantics, value)
/// OpAtomicSMax           (pointer, scope, memorysemantics, value)
/// OpAtomicUMax           (pointer, scope, memorysemantics, value)
/// OpAtomicExchange       (pointer, scope, memorysemantics, value)
/// OpAtomicCompareExchange(pointer, scope, semaequal, semaunequal,
///                         value, comparator)
class SpirvAtomic : public SpirvInstruction {
public:
  SpirvAtomic(spv::Op opcode, QualType resultType, SourceLocation loc,
              SpirvInstruction *pointer, spv::Scope, spv::MemorySemanticsMask,
              SpirvInstruction *value = nullptr, SourceRange range = {});
  SpirvAtomic(spv::Op opcode, QualType resultType, SourceLocation loc,
              SpirvInstruction *pointer, spv::Scope,
              spv::MemorySemanticsMask semanticsEqual,
              spv::MemorySemanticsMask semanticsUnequal,
              SpirvInstruction *value, SpirvInstruction *comparator,
              SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvAtomic)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_Atomic;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getPointer() const { return pointer; }
  spv::Scope getScope() const { return scope; }
  spv::MemorySemanticsMask getMemorySemantics() const { return memorySemantic; }
  bool hasValue() const { return value != nullptr; }
  SpirvInstruction *getValue() const { return value; }
  bool hasComparator() const { return comparator != nullptr; }
  SpirvInstruction *getComparator() const { return comparator; }
  spv::MemorySemanticsMask getMemorySemanticsEqual() const {
    return memorySemantic;
  }
  spv::MemorySemanticsMask getMemorySemanticsUnequal() const {
    return memorySemanticUnequal;
  }

  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    pointer = remapOp(pointer);
    value = remapOp(value);
    comparator = remapOp(comparator);
  }

private:
  SpirvInstruction *pointer;
  spv::Scope scope;
  spv::MemorySemanticsMask memorySemantic;
  spv::MemorySemanticsMask memorySemanticUnequal;
  SpirvInstruction *value;
  SpirvInstruction *comparator;
};

/// \brief OpMemoryBarrier and OpControlBarrier instructions
class SpirvBarrier : public SpirvInstruction {
public:
  SpirvBarrier(SourceLocation loc, spv::Scope memoryScope,
               spv::MemorySemanticsMask memorySemantics,
               llvm::Optional<spv::Scope> executionScope = llvm::None,
               SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvBarrier)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_Barrier;
  }

  bool invokeVisitor(Visitor *v) override;

  spv::Scope getMemoryScope() const { return memoryScope; }
  spv::MemorySemanticsMask getMemorySemantics() const {
    return memorySemantics;
  }
  bool isControlBarrier() const { return hasExecutionScope(); }
  bool hasExecutionScope() const { return executionScope.hasValue(); }
  spv::Scope getExecutionScope() const { return executionScope.getValue(); }

private:
  spv::Scope memoryScope;
  spv::MemorySemanticsMask memorySemantics;
  llvm::Optional<spv::Scope> executionScope;
};

/// \brief OpIsNodePayloadValidAMDX instruction
class SpirvIsNodePayloadValid : public SpirvInstruction {
public:
  SpirvIsNodePayloadValid(QualType resultType, SourceLocation loc,
                          SpirvInstruction *payloadArray,
                          SpirvInstruction *nodeIndex);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvIsNodePayloadValid)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_IsNodePayloadValid;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getPayloadArray() { return payloadArray; }
  SpirvInstruction *getNodeIndex() { return nodeIndex; }

private:
  SpirvInstruction *payloadArray;
  SpirvInstruction *nodeIndex;
};

/// \brief OpNodePayloadArrayLengthAMDX instruction
class SpirvNodePayloadArrayLength : public SpirvInstruction {
public:
  SpirvNodePayloadArrayLength(QualType resultType, SourceLocation loc,
                              SpirvInstruction *payloadArray);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvNodePayloadArrayLength)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_NodePayloadArrayLength;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getPayloadArray() { return payloadArray; }

private:
  SpirvInstruction *payloadArray;
};

/// \brief OpAllocateNodePayloadsAMDX instruction
class SpirvAllocateNodePayloads : public SpirvInstruction {
public:
  SpirvAllocateNodePayloads(QualType resultType, SourceLocation loc,
                            spv::Scope allocationScope,
                            SpirvInstruction *shaderIndex,
                            SpirvInstruction *recordCount);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvAllocateNodePayloads)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_AllocateNodePayloads;
  }

  bool invokeVisitor(Visitor *v) override;

  spv::Scope getAllocationScope() { return allocationScope; }
  SpirvInstruction *getShaderIndex() { return shaderIndex; }
  SpirvInstruction *getRecordCount() { return recordCount; }

private:
  spv::Scope allocationScope;
  SpirvInstruction *shaderIndex;
  SpirvInstruction *recordCount;
};

/// \brief OpReleaseOutputNodePayloadAMDX instruction
class SpirvEnqueueNodePayloads : public SpirvInstruction {
public:
  SpirvEnqueueNodePayloads(SourceLocation loc, SpirvInstruction *payload);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvEnqueueNodePayloads)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_EnqueueNodePayloads;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getPayload() { return payload; }

private:
  SpirvInstruction *payload;
};

/// \brief OpFinishWritingNodePayloadAMDX instruction
class SpirvFinishWritingNodePayload : public SpirvInstruction {
public:
  SpirvFinishWritingNodePayload(QualType resultType, SourceLocation loc,
                                SpirvInstruction *payload);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvFinishWritingNodePayload)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_FinishWritingNodePayload;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getPayload() { return payload; }

private:
  SpirvInstruction *payload;
};

/// \brief Represents SPIR-V binary operation instructions.
///
/// This class includes:
/// -------------------------- Arithmetic operations ---------------------------
/// OpIAdd
/// OpFAdd
/// OpISub
/// OpFSub
/// OpIMul
/// OpFMul
/// OpUDiv
/// OpSDiv
/// OpFDiv
/// OpUMod
/// OpSRem
/// OpSMod
/// OpFRem
/// OpFMod
/// OpVectorTimesScalar
/// OpMatrixTimesScalar
/// OpVectorTimesMatrix
/// OpMatrixTimesVector
/// OpMatrixTimesMatrix
/// OpOuterProduct
/// OpDot
/// -------------------------- Shift operations --------------------------------
/// OpShiftRightLogical
/// OpShiftRightArithmetic
/// OpShiftLeftLogical
/// -------------------------- Bitwise logical operations ----------------------
/// OpBitwiseOr
/// OpBitwiseXor
/// OpBitwiseAnd
/// -------------------------- Logical operations ------------------------------
/// OpLogicalEqual
/// OpLogicalNotEqual
/// OpLogicalOr
/// OpLogicalAnd
/// OpIEqual
/// OpINotEqual
/// OpUGreaterThan
/// OpSGreaterThan
/// OpUGreaterThanEqual
/// OpSGreaterThanEqual
/// OpULessThan
/// OpSLessThan
/// OpULessThanEqual
/// OpSLessThanEqual
/// OpFOrdEqual
/// OpFUnordEqual
/// OpFOrdNotEqual
/// OpFUnordNotEqual
/// OpFOrdLessThan
/// OpFUnordLessThan
/// OpFOrdGreaterThan
/// OpFUnordGreaterThan
/// OpFOrdLessThanEqual
/// OpFUnordLessThanEqual
/// OpFOrdGreaterThanEqual
/// OpFUnordGreaterThanEqual
/// ----------------------------------------------------------------------------
class SpirvBinaryOp : public SpirvInstruction {
public:
  SpirvBinaryOp(spv::Op opcode, QualType resultType, SourceLocation loc,
                SpirvInstruction *op1, SpirvInstruction *op2,
                SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvBinaryOp)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_BinaryOp;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getOperand1() const { return operand1; }
  SpirvInstruction *getOperand2() const { return operand2; }
  bool isSpecConstantOp() const {
    return getopcode() == spv::Op::OpSpecConstantOp;
  }
  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    operand1 = remapOp(operand1);
    operand2 = remapOp(operand2);
  }

private:
  SpirvInstruction *operand1;
  SpirvInstruction *operand2;
};

/// \brief BitField instructions
///
/// This class includes OpBitFieldInsert, OpBitFieldSExtract,
//  and OpBitFieldUExtract.
class SpirvBitField : public SpirvInstruction {
public:
  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_BitFieldExtract ||
           inst->getKind() == IK_BitFieldInsert;
  }

  virtual SpirvInstruction *getBase() const { return base; }
  virtual SpirvInstruction *getOffset() const { return offset; }
  virtual SpirvInstruction *getCount() const { return count; }
  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    count = remapOp(count);
    offset = remapOp(offset);
    base = remapOp(base);
  }

protected:
  SpirvBitField(Kind kind, spv::Op opcode, QualType resultType,
                SourceLocation loc, SpirvInstruction *base,
                SpirvInstruction *offset, SpirvInstruction *count);

private:
  SpirvInstruction *base;
  SpirvInstruction *offset;
  SpirvInstruction *count;
};

class SpirvBitFieldExtract : public SpirvBitField {
public:
  SpirvBitFieldExtract(QualType resultType, SourceLocation loc,
                       SpirvInstruction *base, SpirvInstruction *offset,
                       SpirvInstruction *count);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvBitFieldExtract)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_BitFieldExtract;
  }

  bool invokeVisitor(Visitor *v) override;
};

class SpirvBitFieldInsert : public SpirvBitField {
public:
  SpirvBitFieldInsert(QualType resultType, SourceLocation loc,
                      SpirvInstruction *base, SpirvInstruction *insert,
                      SpirvInstruction *offset, SpirvInstruction *count);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvBitFieldInsert)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_BitFieldInsert;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getInsert() const { return insert; }

  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    SpirvBitField::replaceOperand(remapOp, inEntryFunctionWrapper);
    insert = remapOp(insert);
  }

private:
  SpirvInstruction *insert;
};

class SpirvConstant : public SpirvInstruction {
public:
  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() >= IK_ConstantBoolean &&
           inst->getKind() <= IK_ConstantNull;
  }

  bool operator==(const SpirvConstant &that) const;

  bool isSpecConstant() const;
  void setLiteral(bool literal = true) { literalConstant = literal; }
  bool isLiteral() { return literalConstant; }

protected:
  SpirvConstant(Kind, spv::Op, const SpirvType *, bool literal = false);
  SpirvConstant(Kind, spv::Op, QualType, bool literal = false);
  bool literalConstant;
};

class SpirvConstantBoolean : public SpirvConstant {
public:
  SpirvConstantBoolean(QualType type, bool value, bool isSpecConst = false);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvConstantBoolean)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ConstantBoolean;
  }

  bool operator==(const SpirvConstantBoolean &that) const;

  bool invokeVisitor(Visitor *v) override;

  bool getValue() const { return value; }

private:
  bool value;
};

/// \brief Represent OpConstant for integer values.
class SpirvConstantInteger : public SpirvConstant {
public:
  SpirvConstantInteger(QualType type, llvm::APInt value,
                       bool isSpecConst = false);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvConstantInteger)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ConstantInteger;
  }

  bool operator==(const SpirvConstantInteger &that) const;

  bool invokeVisitor(Visitor *v) override;

  llvm::APInt getValue() const { return value; }

private:
  llvm::APInt value;
};

class SpirvConstantFloat : public SpirvConstant {
public:
  SpirvConstantFloat(QualType type, llvm::APFloat value,
                     bool isSpecConst = false);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvConstantFloat)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ConstantFloat;
  }

  bool operator==(const SpirvConstantFloat &that) const;

  bool invokeVisitor(Visitor *v) override;

  llvm::APFloat getValue() const { return value; }

private:
  llvm::APFloat value;
};

class SpirvConstantComposite : public SpirvConstant {
public:
  SpirvConstantComposite(QualType type,
                         llvm::ArrayRef<SpirvConstant *> constituents,
                         bool isSpecConst = false);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvConstantComposite)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ConstantComposite;
  }

  bool invokeVisitor(Visitor *v) override;

  llvm::ArrayRef<SpirvConstant *> getConstituents() const {
    return constituents;
  }

private:
  llvm::SmallVector<SpirvConstant *, 4> constituents;
};

class SpirvConstantNull : public SpirvConstant {
public:
  SpirvConstantNull(QualType type);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvConstantNull)

  bool invokeVisitor(Visitor *v) override;

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ConstantNull;
  }

  bool operator==(const SpirvConstantNull &that) const;
};

class SpirvConstantString : public SpirvConstant {
public:
  SpirvConstantString(llvm::StringRef stringLiteral, bool isSpecConst = false);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvConstantString)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ConstantString;
  }

  bool invokeVisitor(Visitor *v) override;

  bool operator==(const SpirvConstantString &that) const;

  llvm::StringRef getString() const { return str; }

private:
  std::string str;
};

class SpirvConvertPtrToU : public SpirvInstruction {
public:
  SpirvConvertPtrToU(SpirvInstruction *ptr, QualType type,
                     SourceLocation loc = {}, SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvConvertPtrToU)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ConvertPtrToU;
  }

  bool operator==(const SpirvConvertPtrToU &that) const;

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getPtr() const { return ptr; }

private:
  SpirvInstruction *ptr;
};

class SpirvConvertUToPtr : public SpirvInstruction {
public:
  SpirvConvertUToPtr(SpirvInstruction *intValue, QualType type,
                     SourceLocation loc = {}, SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvConvertUToPtr)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ConvertUToPtr;
  }

  bool operator==(const SpirvConvertUToPtr &that) const;

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getVal() const { return val; }

private:
  SpirvInstruction *val;
};

class SpirvUndef : public SpirvInstruction {
public:
  SpirvUndef(QualType type);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvUndef)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_Undef;
  }

  bool operator==(const SpirvUndef &that) const;

  bool invokeVisitor(Visitor *v) override;
};

/// \brief OpCompositeConstruct instruction
class SpirvCompositeConstruct : public SpirvInstruction {
public:
  SpirvCompositeConstruct(QualType resultType, SourceLocation loc,
                          llvm::ArrayRef<SpirvInstruction *> constituentsVec,
                          SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvCompositeConstruct)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_CompositeConstruct;
  }

  bool invokeVisitor(Visitor *v) override;

  llvm::ArrayRef<SpirvInstruction *> getConstituents() const {
    return consituents;
  }

  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    for (size_t idx = 0; idx < consituents.size(); idx++)
      consituents[idx] = remapOp(consituents[idx]);
  }

private:
  llvm::SmallVector<SpirvInstruction *, 4> consituents;
};

/// \brief Extraction instruction (OpCompositeExtract)
class SpirvCompositeExtract : public SpirvInstruction {
public:
  SpirvCompositeExtract(QualType resultType, SourceLocation loc,
                        SpirvInstruction *composite,
                        llvm::ArrayRef<uint32_t> indices,
                        SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvCompositeExtract)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_CompositeExtract;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getComposite() const { return composite; }
  llvm::ArrayRef<uint32_t> getIndexes() const { return indices; }
  void insertIndex(uint32_t i, uint32_t index) {
    if (index < indices.size())
      indices.insert(&indices[index], i);
    else if (index == indices.size())
      indices.push_back(i);
  }

private:
  SpirvInstruction *composite;
  llvm::SmallVector<uint32_t, 4> indices;
};

/// \brief Composite insertion instruction (OpCompositeInsert)
class SpirvCompositeInsert : public SpirvInstruction {
public:
  SpirvCompositeInsert(QualType resultType, SourceLocation loc,
                       SpirvInstruction *composite, SpirvInstruction *object,
                       llvm::ArrayRef<uint32_t> indices,
                       SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvCompositeInsert)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_CompositeInsert;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getComposite() const { return composite; }
  SpirvInstruction *getObject() const { return object; }
  llvm::ArrayRef<uint32_t> getIndexes() const { return indices; }
  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    composite = remapOp(composite);
    object = remapOp(object);
  }

private:
  SpirvInstruction *composite;
  SpirvInstruction *object;
  llvm::SmallVector<uint32_t, 4> indices;
};

/// \brief EmitVertex instruction
class SpirvEmitVertex : public SpirvInstruction {
public:
  SpirvEmitVertex(SourceLocation loc, SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvEmitVertex)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_EmitVertex;
  }

  bool invokeVisitor(Visitor *v) override;
};

/// \brief EndPrimitive instruction
class SpirvEndPrimitive : public SpirvInstruction {
public:
  SpirvEndPrimitive(SourceLocation loc, SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvEndPrimitive)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_EndPrimitive;
  }

  bool invokeVisitor(Visitor *v) override;
};

/// \brief ExtInst instruction
class SpirvExtInst : public SpirvInstruction {
public:
  SpirvExtInst(QualType resultType, SourceLocation loc, SpirvExtInstImport *set,
               uint32_t inst, llvm::ArrayRef<SpirvInstruction *> operandsVec,
               SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvExtInst)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ExtInst;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvExtInstImport *getInstructionSet() const { return instructionSet; }
  uint32_t getInstruction() const { return instruction; }
  llvm::ArrayRef<SpirvInstruction *> getOperands() const { return operands; }

  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    for (size_t idx = 0; idx < operands.size(); idx++)
      operands[idx] = remapOp(operands[idx]);
  }

private:
  SpirvExtInstImport *instructionSet;
  uint32_t instruction;
  llvm::SmallVector<SpirvInstruction *, 4> operands;
};

/// \brief OpFunctionCall instruction
class SpirvFunctionCall : public SpirvInstruction {
public:
  SpirvFunctionCall(QualType resultType, SourceLocation loc,
                    SpirvFunction *function,
                    llvm::ArrayRef<SpirvInstruction *> argsVec,
                    SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvFunctionCall)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_FunctionCall;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvFunction *getFunction() const { return function; }
  llvm::ArrayRef<SpirvInstruction *> getArgs() const { return args; }
  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    for (size_t idx = 0; idx < args.size(); idx++)
      args[idx] = remapOp(args[idx]);
  }

private:
  SpirvFunction *function;
  llvm::SmallVector<SpirvInstruction *, 4> args;
};

/// \brief OpGroupNonUniform* instructions
class SpirvGroupNonUniformOp : public SpirvInstruction {
public:
  SpirvGroupNonUniformOp(spv::Op opcode, QualType resultType,
                         llvm::Optional<spv::Scope> scope,
                         llvm::ArrayRef<SpirvInstruction *> operands,
                         SourceLocation loc,
                         llvm::Optional<spv::GroupOperation> group);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvGroupNonUniformOp)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_GroupNonUniformOp;
  }

  bool invokeVisitor(Visitor *v) override;

  bool hasExecutionScope() const { return execScope.hasValue(); }
  spv::Scope getExecutionScope() const { return execScope.getValue(); }

  llvm::ArrayRef<SpirvInstruction *> getOperands() const { return operands; }

  bool hasGroupOp() const { return groupOp.hasValue(); }
  spv::GroupOperation getGroupOp() const { return groupOp.getValue(); }

  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    for (auto *operand : getOperands()) {
      operand = remapOp(operand);
    }
    if (inEntryFunctionWrapper)
      setAstResultType(getOperands()[0]->getAstResultType());
  }

private:
  llvm::Optional<spv::Scope> execScope;
  llvm::SmallVector<SpirvInstruction *, 4> operands;
  llvm::Optional<spv::GroupOperation> groupOp;
};

/// \brief Image instructions.
///
/// This class includes:
///
/// OpImageSampleImplicitLod          (image, coordinate, mask, args)
/// OpImageSampleExplicitLod          (image, coordinate, mask, args, lod)
/// OpImageSampleDrefImplicitLod      (image, coordinate, mask, args, dref)
/// OpImageSampleDrefExplicitLod      (image, coordinate, mask, args, dref, lod)
/// OpImageSparseSampleImplicitLod    (image, coordinate, mask, args)
/// OpImageSparseSampleExplicitLod    (image, coordinate, mask, args, lod)
/// OpImageSparseSampleDrefImplicitLod(image, coordinate, mask, args, dref)
/// OpImageSparseSampleDrefExplicitLod(image, coordinate, mask, args, dref, lod)
///
/// OpImageFetch                      (image, coordinate, mask, args)
/// OpImageSparseFetch                (image, coordinate, mask, args)
/// OpImageGather                     (image, coordinate, mask, args, component)
/// OpImageSparseGather               (image, coordinate, mask, args, component)
/// OpImageDrefGather                 (image, coordinate, mask, args, dref)
/// OpImageSparseDrefGather           (image, coordinate, mask, args, dref)
/// OpImageRead                       (image, coordinate, mask, args)
/// OpImageSparseRead                 (image, coordinate, mask, args)
/// OpImageWrite                      (image, coordinate, mask, args, texel)
///
/// Image operands can include:
/// Bias, Lod, Grad (pair), ConstOffset, Offset, ConstOffsets, Sample, MinLod.
///
class SpirvImageOp : public SpirvInstruction {
public:
  SpirvImageOp(
      spv::Op opcode, QualType resultType, SourceLocation loc,
      SpirvInstruction *image, SpirvInstruction *coordinate,
      spv::ImageOperandsMask mask, SpirvInstruction *dref = nullptr,
      SpirvInstruction *bias = nullptr, SpirvInstruction *lod = nullptr,
      SpirvInstruction *gradDx = nullptr, SpirvInstruction *gradDy = nullptr,
      SpirvInstruction *constOffset = nullptr,
      SpirvInstruction *offset = nullptr,
      SpirvInstruction *constOffsets = nullptr,
      SpirvInstruction *sample = nullptr, SpirvInstruction *minLod = nullptr,
      SpirvInstruction *component = nullptr,
      SpirvInstruction *texelToWrite = nullptr, SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvImageOp)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ImageOp;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getImage() const { return image; }
  SpirvInstruction *getCoordinate() const { return coordinate; }
  spv::ImageOperandsMask getImageOperandsMask() const { return operandsMask; }

  bool isSparse() const;
  bool hasDref() const { return dref != nullptr; }
  bool hasBias() const { return bias != nullptr; }
  bool hasLod() const { return lod != nullptr; }
  bool hasGrad() const { return gradDx != nullptr && gradDy != nullptr; }
  bool hasConstOffset() const { return constOffset != nullptr; }
  bool hasOffset() const { return offset != nullptr; }
  bool hasConstOffsets() const { return constOffsets != nullptr; }
  bool hasSample() const { return sample != nullptr; }
  bool hasMinLod() const { return minLod != nullptr; }
  bool hasComponent() const { return component != nullptr; }
  bool isImageWrite() const { return texelToWrite != nullptr; }

  SpirvInstruction *getDref() const { return dref; }
  SpirvInstruction *getBias() const { return bias; }
  SpirvInstruction *getLod() const { return lod; }
  SpirvInstruction *getGradDx() const { return gradDx; }
  SpirvInstruction *getGradDy() const { return gradDy; }
  std::pair<SpirvInstruction *, SpirvInstruction *> getGrad() const {
    return std::make_pair(gradDx, gradDy);
  }
  SpirvInstruction *getConstOffset() const { return constOffset; }
  SpirvInstruction *getOffset() const { return offset; }
  SpirvInstruction *getConstOffsets() const { return constOffsets; }
  SpirvInstruction *getSample() const { return sample; }
  SpirvInstruction *getMinLod() const { return minLod; }
  SpirvInstruction *getComponent() const { return component; }
  SpirvInstruction *getTexelToWrite() const { return texelToWrite; }
  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    coordinate = remapOp(coordinate);
    dref = remapOp(dref);
    bias = remapOp(bias);
    lod = remapOp(lod);
    gradDx = remapOp(gradDx);
    gradDy = remapOp(gradDy);
    offset = remapOp(offset);
    minLod = remapOp(minLod);
    component = remapOp(component);
  }

private:
  SpirvInstruction *image;
  SpirvInstruction *coordinate;
  SpirvInstruction *dref;
  SpirvInstruction *bias;
  SpirvInstruction *lod;
  SpirvInstruction *gradDx;
  SpirvInstruction *gradDy;
  SpirvInstruction *constOffset;
  SpirvInstruction *offset;
  SpirvInstruction *constOffsets;
  SpirvInstruction *sample;
  SpirvInstruction *minLod;
  SpirvInstruction *component;
  SpirvInstruction *texelToWrite;
  spv::ImageOperandsMask operandsMask;
};

/// \brief Image query instructions:
///
/// Covers the following instructions:
/// OpImageQueryFormat  (image)
/// OpImageQueryOrder   (image)
/// OpImageQuerySize    (image)
/// OpImageQueryLevels  (image)
/// OpImageQuerySamples (image)
/// OpImageQueryLod     (image, coordinate)
/// OpImageQuerySizeLod (image, lod)
class SpirvImageQuery : public SpirvInstruction {
public:
  SpirvImageQuery(spv::Op opcode, QualType resultType, SourceLocation loc,
                  SpirvInstruction *img, SpirvInstruction *lod = nullptr,
                  SpirvInstruction *coord = nullptr, SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvImageQuery)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ImageQuery;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getImage() const { return image; }
  bool hasLod() const { return lod != nullptr; }
  SpirvInstruction *getLod() const { return lod; }
  bool hasCoordinate() const { return coordinate != nullptr; }
  SpirvInstruction *getCoordinate() const { return coordinate; }
  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    lod = remapOp(lod);
    coordinate = remapOp(coordinate);
  }

private:
  SpirvInstruction *image;
  SpirvInstruction *lod;
  SpirvInstruction *coordinate;
};

/// \brief OpImageSparseTexelsResident instruction
class SpirvImageSparseTexelsResident : public SpirvInstruction {
public:
  SpirvImageSparseTexelsResident(QualType resultType, SourceLocation loc,
                                 SpirvInstruction *resCode,
                                 SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvImageSparseTexelsResident)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ImageSparseTexelsResident;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getResidentCode() const { return residentCode; }

private:
  SpirvInstruction *residentCode;
};

/// \brief OpImageTexelPointer instruction
/// Note: The resultType stored in objects of this class are the underlying
/// type. The real result type of OpImageTexelPointer must always be an
/// OpTypePointer whose Storage Class operand is Image.
class SpirvImageTexelPointer : public SpirvInstruction {
public:
  SpirvImageTexelPointer(QualType resultType, SourceLocation loc,
                         SpirvInstruction *image, SpirvInstruction *coordinate,
                         SpirvInstruction *sample);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvImageTexelPointer)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ImageTexelPointer;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getImage() const { return image; }
  SpirvInstruction *getCoordinate() const { return coordinate; }
  SpirvInstruction *getSample() const { return sample; }
  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    coordinate = remapOp(coordinate);
  }

private:
  SpirvInstruction *image;
  SpirvInstruction *coordinate;
  SpirvInstruction *sample;
};

/// \brief Load instruction representation
class SpirvLoad : public SpirvInstruction {
public:
  SpirvLoad(QualType resultType, SourceLocation loc, SpirvInstruction *pointer,
            SourceRange range = {},
            llvm::Optional<spv::MemoryAccessMask> mask = llvm::None);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvLoad)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_Load;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getPointer() const { return pointer; }
  bool hasMemoryAccessSemantics() const { return memoryAccess.hasValue(); }
  spv::MemoryAccessMask getMemoryAccess() const {
    return memoryAccess.getValue();
  }

  void setAlignment(uint32_t alignment);
  bool hasAlignment() const { return memoryAlignment.hasValue(); }
  uint32_t getAlignment() const { return memoryAlignment.getValue(); }
  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    pointer = remapOp(pointer);
    if (inEntryFunctionWrapper)
      setAstResultType(pointer->getAstResultType());
  }

private:
  SpirvInstruction *pointer;
  llvm::Optional<spv::MemoryAccessMask> memoryAccess;
  llvm::Optional<uint32_t> memoryAlignment;
};

/// \brief OpCopyObject instruction
class SpirvCopyObject : public SpirvInstruction {
public:
  SpirvCopyObject(QualType resultType, SourceLocation loc,
                  SpirvInstruction *pointer);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvCopyObject)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_CopyObject;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getPointer() const { return pointer; }
  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    pointer = remapOp(pointer);
    if (inEntryFunctionWrapper)
      setAstResultType(pointer->getAstResultType());
  }

private:
  SpirvInstruction *pointer;
};

/// \brief OpSampledImage instruction
/// Result Type must be the OpTypeSampledImage type whose Image Type operand is
/// the type of Image. We store the QualType for the underlying image as result
/// type.
class SpirvSampledImage : public SpirvInstruction {
public:
  SpirvSampledImage(QualType resultType, SourceLocation loc,
                    SpirvInstruction *image, SpirvInstruction *sampler,
                    SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvSampledImage)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_SampledImage;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getImage() const { return image; }
  SpirvInstruction *getSampler() const { return sampler; }

private:
  SpirvInstruction *image;
  SpirvInstruction *sampler;
};

/// \brief Select operation representation.
class SpirvSelect : public SpirvInstruction {
public:
  SpirvSelect(QualType resultType, SourceLocation loc, SpirvInstruction *cond,
              SpirvInstruction *trueId, SpirvInstruction *falseId,
              SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvSelect)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_Select;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getCondition() const { return condition; }
  SpirvInstruction *getTrueObject() const { return trueObject; }
  SpirvInstruction *getFalseObject() const { return falseObject; }
  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    condition = remapOp(condition);
    trueObject = remapOp(trueObject);
    falseObject = remapOp(falseObject);
  }

private:
  SpirvInstruction *condition;
  SpirvInstruction *trueObject;
  SpirvInstruction *falseObject;
};

/// \brief OpSpecConstantOp instruction where the operation is binary.
class SpirvSpecConstantBinaryOp : public SpirvInstruction {
public:
  SpirvSpecConstantBinaryOp(spv::Op specConstantOp, QualType resultType,
                            SourceLocation loc, SpirvInstruction *operand1,
                            SpirvInstruction *operand2);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvSpecConstantBinaryOp)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_SpecConstantBinaryOp;
  }

  bool invokeVisitor(Visitor *v) override;

  spv::Op getSpecConstantopcode() const { return specOp; }
  SpirvInstruction *getOperand1() const { return operand1; }
  SpirvInstruction *getOperand2() const { return operand2; }

private:
  spv::Op specOp;
  SpirvInstruction *operand1;
  SpirvInstruction *operand2;
};

/// \brief OpSpecConstantOp instruction where the operation is unary.
class SpirvSpecConstantUnaryOp : public SpirvInstruction {
public:
  SpirvSpecConstantUnaryOp(spv::Op specConstantOp, QualType resultType,
                           SourceLocation loc, SpirvInstruction *operand);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvSpecConstantUnaryOp)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_SpecConstantUnaryOp;
  }

  bool invokeVisitor(Visitor *v) override;

  spv::Op getSpecConstantopcode() const { return specOp; }
  SpirvInstruction *getOperand() const { return operand; }

private:
  spv::Op specOp;
  SpirvInstruction *operand;
};

/// \brief Store instruction representation
class SpirvStore : public SpirvInstruction {
public:
  SpirvStore(SourceLocation loc, SpirvInstruction *pointer,
             SpirvInstruction *object,
             llvm::Optional<spv::MemoryAccessMask> mask = llvm::None,
             SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvStore)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_Store;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getPointer() const { return pointer; }
  SpirvInstruction *getObject() const { return object; }
  bool hasMemoryAccessSemantics() const { return memoryAccess.hasValue(); }
  spv::MemoryAccessMask getMemoryAccess() const {
    return memoryAccess.getValue();
  }

  void setAlignment(uint32_t alignment);
  bool hasAlignment() const { return memoryAlignment.hasValue(); }
  uint32_t getAlignment() const { return memoryAlignment.getValue(); }
  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    pointer = remapOp(pointer);
    object = remapOp(object);
    if (inEntryFunctionWrapper &&
        object->getAstResultType() != pointer->getAstResultType() &&
        isa<SpirvVariable>(pointer) &&
        pointer->getStorageClass() == spv::StorageClass::Output)
      pointer->setAstResultType(object->getAstResultType());
  }

private:
  SpirvInstruction *pointer;
  SpirvInstruction *object;
  llvm::Optional<spv::MemoryAccessMask> memoryAccess;
  llvm::Optional<uint32_t> memoryAlignment;
};

/// \brief Represents SPIR-V nullary operation instructions.
///
/// This class includes:
/// ----------------------------------------------------------------------------
/// OpBeginInvocationInterlockEXT // FragmentShader*InterlockEXT capability
/// OpEndInvocationInterlockEXT // FragmentShader*InterlockEXT capability
/// ----------------------------------------------------------------------------
class SpirvNullaryOp : public SpirvInstruction {
public:
  SpirvNullaryOp(spv::Op opcode, SourceLocation loc, SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvNullaryOp)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_NullaryOp;
  }

  bool invokeVisitor(Visitor *v) override;
};

/// \brief Represents SPIR-V unary operation instructions.
///
/// This class includes:
/// ----------------------------------------------------------------------------
/// opTranspose    // Matrix capability
/// opDPdx
/// opDPdy
/// opFwidth
/// opDPdxFine     // DerivativeControl capability
/// opDPdyFine     // DerivativeControl capability
/// opFwidthFine   // DerivativeControl capability
/// opDPdxCoarse   // DerivativeControl capability
/// opDPdyCoarse   // DerivativeControl capability
/// opFwidthCoarse // DerivativeControl capability
/// ------------------------- Conversion operations ----------------------------
/// OpConvertFToU
/// OpConvertFToS
/// OpConvertSToF
/// OpConvertUToF
/// OpUConvert
/// OpSConvert
/// OpFConvert
/// OpBitcast
/// ----------------------------------------------------------------------------
/// OpSNegate
/// OpFNegate
/// ----------------------------------------------------------------------------
/// opBitReverse
/// opBitCount
/// OpNot
/// ----------------------------- Logical operations ---------------------------
/// OpLogicalNot
/// OpAny
/// OpAll
/// OpIsNan
/// OpIsInf
/// OpIsFinite        // Kernel capability
/// ----------------------------------------------------------------------------
class SpirvUnaryOp : public SpirvInstruction {
public:
  SpirvUnaryOp(spv::Op opcode, QualType resultType, SourceLocation loc,
               SpirvInstruction *op, SourceRange range = {});

  SpirvUnaryOp(spv::Op opcode, const SpirvType *resultType, SourceLocation loc,
               SpirvInstruction *op);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvUnaryOp)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_UnaryOp;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getOperand() const { return operand; }
  bool isConversionOp() const;
  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    operand = remapOp(operand);
  }

private:
  SpirvInstruction *operand;
};

/// \brief OpVectorShuffle instruction
class SpirvVectorShuffle : public SpirvInstruction {
public:
  SpirvVectorShuffle(QualType resultType, SourceLocation loc,
                     SpirvInstruction *vec1, SpirvInstruction *vec2,
                     llvm::ArrayRef<uint32_t> componentsVec,
                     SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvVectorShuffle)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_VectorShuffle;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getVec1() const { return vec1; }
  SpirvInstruction *getVec2() const { return vec2; }
  llvm::ArrayRef<uint32_t> getComponents() const { return components; }
  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    vec1 = remapOp(vec1);
    vec2 = remapOp(vec2);
  }

private:
  SpirvInstruction *vec1;
  SpirvInstruction *vec2;
  llvm::SmallVector<uint32_t, 4> components;
};

class SpirvArrayLength : public SpirvInstruction {
public:
  SpirvArrayLength(QualType resultType, SourceLocation loc,
                   SpirvInstruction *structure, uint32_t arrayMember,
                   SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvArrayLength)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ArrayLength;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getStructure() const { return structure; }
  uint32_t getArrayMember() const { return arrayMember; }

private:
  SpirvInstruction *structure;
  uint32_t arrayMember;
};

/// \brief Base class for all NV raytracing instructions.
/// These include following SPIR-V opcodes:
/// OpTraceNV, OpReportIntersectionNV, OpIgnoreIntersectionNV, OpTerminateRayNV,
/// OpExecuteCallableNV
class SpirvRayTracingOpNV : public SpirvInstruction {
public:
  SpirvRayTracingOpNV(QualType resultType, spv::Op opcode,
                      llvm::ArrayRef<SpirvInstruction *> vecOperands,
                      SourceLocation loc);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvRayTracingOpNV)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_RayTracingOpNV;
  }

  bool invokeVisitor(Visitor *v) override;

  llvm::ArrayRef<SpirvInstruction *> getOperands() const { return operands; }

private:
  llvm::SmallVector<SpirvInstruction *, 4> operands;
};

class SpirvRayQueryOpKHR : public SpirvInstruction {
public:
  SpirvRayQueryOpKHR(QualType resultType, spv::Op opcode,
                     llvm::ArrayRef<SpirvInstruction *> vecOperands, bool flags,
                     SourceLocation loc, SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvRayQueryOpKHR)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_RayQueryOpKHR;
  }

  bool invokeVisitor(Visitor *v) override;

  llvm::ArrayRef<SpirvInstruction *> getOperands() const { return operands; }

  bool hasCullFlags() const { return cullFlags; }

private:
  llvm::SmallVector<SpirvInstruction *, 4> operands;
  bool cullFlags;
};

class SpirvRayTracingTerminateOpKHR : public SpirvTerminator {
public:
  SpirvRayTracingTerminateOpKHR(spv::Op opcode, SourceLocation loc);
  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvRayTracingTerminateOpKHR)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_RayTracingTerminate;
  }

  bool invokeVisitor(Visitor *v) override;
};

/// \brief OpDemoteToHelperInvocation instruction.
/// Demote fragment shader invocation to a helper invocation. Any stores to
/// memory after this instruction are suppressed and the fragment does not write
/// outputs to the framebuffer. Unlike the OpKill instruction, this does not
/// necessarily terminate the invocation. It is not considered a flow control
/// instruction (flow control does not become non-uniform) and does not
/// terminate the block.
class SpirvDemoteToHelperInvocation : public SpirvInstruction {
public:
  SpirvDemoteToHelperInvocation(SourceLocation);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDemoteToHelperInvocation)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DemoteToHelperInvocation;
  }

  bool invokeVisitor(Visitor *v) override;
};

/// \brief OpIsHelperInvocationEXT instruction.
/// Result is true if the invocation is currently a helper invocation, otherwise
/// result is false. An invocation is currently a helper invocation if it was
/// originally invoked as a helper invocation or if it has been demoted to a
/// helper invocation by OpDemoteToHelperInvocationEXT.
class SpirvIsHelperInvocationEXT : public SpirvInstruction {
public:
  SpirvIsHelperInvocationEXT(QualType, SourceLocation);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvIsHelperInvocationEXT)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_IsHelperInvocationEXT;
  }

  bool invokeVisitor(Visitor *v) override;
};

// A class keeping information of [[vk::ext_instruction(uint opcode,
// string extended_instruction_set)]] attribute. The attribute allows users to
// emit an arbitrary SPIR-V instruction by adding it to a function declaration.
// Note that this class does not represent an actual specific SPIR-V
// instruction. It is used to keep the information of the arbitrary SPIR-V
// instruction.
class SpirvIntrinsicInstruction : public SpirvInstruction {
public:
  SpirvIntrinsicInstruction(QualType resultType, uint32_t opcode,
                            llvm::ArrayRef<SpirvInstruction *> operands,
                            llvm::ArrayRef<llvm::StringRef> extensions,
                            SpirvExtInstImport *set,
                            llvm::ArrayRef<uint32_t> capabilities,
                            SourceLocation loc);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvIntrinsicInstruction)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_SpirvIntrinsicInstruction;
  }

  bool invokeVisitor(Visitor *v) override;

  llvm::ArrayRef<SpirvInstruction *> getOperands() const { return operands; }
  llvm::ArrayRef<uint32_t> getCapabilities() const { return capabilities; }
  llvm::ArrayRef<std::string> getExtensions() const { return extensions; }
  SpirvExtInstImport *getInstructionSet() const { return instructionSet; }
  uint32_t getInstruction() const { return instruction; }
  void replaceOperand(
      llvm::function_ref<SpirvInstruction *(SpirvInstruction *)> remapOp,
      bool inEntryFunctionWrapper) override {
    for (size_t idx = 0; idx < operands.size(); idx++)
      operands[idx] = remapOp(operands[idx]);
  }

private:
  uint32_t instruction;
  llvm::SmallVector<SpirvInstruction *, 4> operands;
  llvm::SmallVector<uint32_t, 4> capabilities;
  llvm::SmallVector<std::string, 4> extensions;
  SpirvExtInstImport *instructionSet;
};

/// \brief Base class for all rich DebugInfo extension instructions.
/// Note that all of these instructions should be added to the SPIR-V module as
/// an OpExtInst instructions. So, all of these instructions must:
/// 1) contain the result-id of the extended instruction set
/// 2) have OpTypeVoid as their Result Type.
/// 3) contain additional QualType and SpirvType for the debug type.
class SpirvDebugInstruction : public SpirvInstruction {
public:
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() >= IK_DebugInfoNone &&
           inst->getKind() <= IK_DebugTypeTemplateParameter;
  }

  void setDebugType(SpirvDebugInstruction *type) { debugType = type; }
  void setInstructionSet(SpirvExtInstImport *set) { instructionSet = set; }
  SpirvExtInstImport *getInstructionSet() const { return instructionSet; }
  uint32_t getDebugOpcode() const { return debugOpcode; }
  QualType getDebugQualType() const { return debugQualType; }
  const SpirvType *getDebugSpirvType() const { return debugSpirvType; }
  SpirvDebugInstruction *getDebugType() const { return debugType; }
  void setDebugQualType(QualType type) { debugQualType = type; }
  void setDebugSpirvType(const SpirvType *type) { debugSpirvType = type; }

  virtual SpirvDebugInstruction *getParentScope() const { return nullptr; }

protected:
  // TODO: Replace opcode type with an enum, when it is available in
  // SPIRV-Headers.
  SpirvDebugInstruction(Kind kind, uint32_t opcode);

private:
  // TODO: Replace this with an enum, when it is available in SPIRV-Headers.
  uint32_t debugOpcode;

  QualType debugQualType;
  const SpirvType *debugSpirvType;

  // The constructor for SpirvDebugInstruction sets the debug type to nullptr.
  // A type lowering IMR pass will set debug types for all debug instructions
  // that do contain a debug type.
  SpirvDebugInstruction *debugType;

  // The pointer to the debug info extended instruction set.
  // This is not required by the constructor, and can be set via any IMR pass.
  SpirvExtInstImport *instructionSet;
};

/// \brief OpEmitMeshTasksEXT instruction.
class SpirvEmitMeshTasksEXT : public SpirvInstruction {
public:
  SpirvEmitMeshTasksEXT(SpirvInstruction *xDim, SpirvInstruction *yDim,
                        SpirvInstruction *zDim, SpirvInstruction *payload,
                        SourceLocation loc, SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvEmitMeshTasksEXT)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_EmitMeshTasksEXT;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getXDimension() const { return xDim; }
  SpirvInstruction *getYDimension() const { return yDim; }
  SpirvInstruction *getZDimension() const { return zDim; }
  SpirvInstruction *getPayload() const { return payload; }

private:
  SpirvInstruction *xDim;
  SpirvInstruction *yDim;
  SpirvInstruction *zDim;
  SpirvInstruction *payload;
};

/// \brief OpSetMeshOutputsEXT instruction.
class SpirvSetMeshOutputsEXT : public SpirvInstruction {
public:
  SpirvSetMeshOutputsEXT(SpirvInstruction *vertCount,
                         SpirvInstruction *primCount, SourceLocation loc,
                         SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvSetMeshOutputsEXT)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_SetMeshOutputsEXT;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getVertexCount() const { return vertCount; }
  SpirvInstruction *getPrimitiveCount() const { return primCount; }

private:
  SpirvInstruction *vertCount;
  SpirvInstruction *primCount;
};

class SpirvDebugInfoNone : public SpirvDebugInstruction {
public:
  SpirvDebugInfoNone();

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugInfoNone)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugInfoNone;
  }

  bool invokeVisitor(Visitor *v) override;
};

class SpirvDebugSource : public SpirvDebugInstruction {
public:
  SpirvDebugSource(llvm::StringRef file, llvm::StringRef text);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugSource)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugSource;
  }

  bool invokeVisitor(Visitor *v) override;

  llvm::StringRef getFile() const { return file; }
  llvm::StringRef getContent() const { return text; }

private:
  std::string file;
  std::string text;
};

class SpirvDebugCompilationUnit : public SpirvDebugInstruction {
public:
  SpirvDebugCompilationUnit(uint32_t spirvVersion, uint32_t dwarfVersion,
                            SpirvDebugSource *src);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugCompilationUnit)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugCompilationUnit;
  }

  bool invokeVisitor(Visitor *v) override;

  uint32_t getSpirvVersion() const { return spirvVersion; }
  uint32_t getDwarfVersion() const { return dwarfVersion; }
  SpirvDebugSource *getDebugSource() const { return source; }
  spv::SourceLanguage getLanguage() const { return lang; }

private:
  uint32_t spirvVersion;
  uint32_t dwarfVersion;
  SpirvDebugSource *source;
  spv::SourceLanguage lang;
};

// This class is not actually used. It can be cleaned up.
class SpirvDebugFunctionDeclaration : public SpirvDebugInstruction {
public:
  SpirvDebugFunctionDeclaration(llvm::StringRef name, SpirvDebugSource *src,
                                uint32_t fnLine, uint32_t fnColumn,
                                SpirvDebugInstruction *parentScope,
                                llvm::StringRef linkageName, uint32_t flags);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugFunctionDeclaration)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugFunctionDecl;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvDebugSource *getSource() const { return source; }
  uint32_t getLine() const { return fnLine; }
  uint32_t getColumn() const { return fnColumn; }
  void setParent(SpirvDebugInstruction *scope) { parentScope = scope; }
  SpirvDebugInstruction *getParentScope() const override { return parentScope; }
  llvm::StringRef getLinkageName() const { return linkageName; }
  uint32_t getFlags() const { return flags; }

private:
  SpirvDebugSource *source;
  // Source line number at which the function appears
  uint32_t fnLine;
  // Source column number at which the function appears
  uint32_t fnColumn;
  // Debug instruction which represents the parent lexical scope
  SpirvDebugInstruction *parentScope;
  std::string linkageName;
  // TODO: Replace this with an enum, when it is available in SPIRV-Headers
  uint32_t flags;
};

class SpirvDebugFunction : public SpirvDebugInstruction {
public:
  SpirvDebugFunction(llvm::StringRef name, SpirvDebugSource *src,
                     uint32_t fnLine, uint32_t fnColumn,
                     SpirvDebugInstruction *parentScope,
                     llvm::StringRef linkageName, uint32_t flags,
                     uint32_t scopeLine, SpirvFunction *fn);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugFunction)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugFunction;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvDebugSource *getSource() const { return source; }
  uint32_t getLine() const { return fnLine; }
  uint32_t getColumn() const { return fnColumn; }
  void setParent(SpirvDebugInstruction *scope) { parentScope = scope; }
  SpirvDebugInstruction *getParentScope() const override { return parentScope; }
  llvm::StringRef getLinkageName() const { return linkageName; }
  uint32_t getFlags() const { return flags; }
  uint32_t getScopeLine() const { return scopeLine; }
  SpirvFunction *getSpirvFunction() const { return fn; }

  void setFunctionType(clang::spirv::FunctionType *t) { fnType = t; }
  clang::spirv::FunctionType *getFunctionType() const { return fnType; }

  void setDebugInfoNone(SpirvDebugInfoNone *none) { debugNone = none; }
  SpirvDebugInfoNone *getDebugInfoNone() const { return debugNone; }

private:
  SpirvDebugSource *source;
  // Source line number at which the function appears
  uint32_t fnLine;
  // Source column number at which the function appears
  uint32_t fnColumn;
  // Debug instruction which represents the parent lexical scope
  SpirvDebugInstruction *parentScope;
  std::string linkageName;
  // TODO: Replace this with an enum, when it is available in SPIRV-Headers
  uint32_t flags;
  // Line number in the source program at which the function scope begins
  uint32_t scopeLine;
  // The function to which this debug instruction belongs
  SpirvFunction *fn;

  // When fn is nullptr, we want to put DebugInfoNone for Function operand
  // of DebugFunction. Note that the spec must allow it (which will be
  // discussed). We can consider this debugNone is a fallback of fn.
  SpirvDebugInfoNone *debugNone;

  // When the function debug info is generated by LowerTypeVisitor (not
  // SpirvEmitter), we do not generate SpirvFunction. We only generate
  // SpirvDebugFunction. Similar to fnType of SpirvFunction, we want to
  // keep the function type info in this fnType.
  clang::spirv::FunctionType *fnType;
};

class SpirvDebugEntryPoint : public SpirvDebugInstruction {
public:
  SpirvDebugEntryPoint(SpirvDebugFunction *ep, SpirvDebugCompilationUnit *cu,
                       llvm::StringRef signature, llvm::StringRef args);
  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugEntryPoint)
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugEntryPoint;
  }
  bool invokeVisitor(Visitor *v) override;
  SpirvDebugFunction *getEntryPoint() const { return ep; }
  SpirvDebugCompilationUnit *getCompilationUnit() const { return cu; }
  llvm::StringRef getSignature() const { return signature; }
  llvm::StringRef getArgs() const { return args; }

private:
  SpirvDebugFunction *ep;
  SpirvDebugCompilationUnit *cu;
  std::string signature;
  std::string args;
};

class SpirvDebugFunctionDefinition : public SpirvDebugInstruction {
public:
  SpirvDebugFunctionDefinition(SpirvDebugFunction *function, SpirvFunction *fn);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugFunctionDefinition)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugFunctionDef;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvDebugFunction *getDebugFunction() const { return function; }
  SpirvFunction *getFunction() const { return fn; }

private:
  SpirvDebugFunction *function;
  SpirvFunction *fn;
};

class SpirvDebugLocalVariable : public SpirvDebugInstruction {
public:
  SpirvDebugLocalVariable(QualType debugQualType, llvm::StringRef varName,
                          SpirvDebugSource *src, uint32_t line, uint32_t column,
                          SpirvDebugInstruction *parentScope, uint32_t flags,
                          llvm::Optional<uint32_t> argNumber = llvm::None);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugLocalVariable)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugLocalVariable;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvDebugSource *getSource() const { return source; }
  uint32_t getLine() const { return line; }
  uint32_t getColumn() const { return column; }
  SpirvDebugInstruction *getParentScope() const override { return parentScope; }
  uint32_t getFlags() const { return flags; }
  llvm::Optional<uint32_t> getArgNumber() const { return argNumber; }

private:
  SpirvDebugSource *source;
  uint32_t line;
  uint32_t column;
  SpirvDebugInstruction *parentScope;
  // TODO: Replace this with an enum, when it is available in SPIRV-Headers
  uint32_t flags;
  llvm::Optional<uint32_t> argNumber;
};

class SpirvDebugGlobalVariable : public SpirvDebugInstruction {
public:
  SpirvDebugGlobalVariable(
      QualType debugQualType, llvm::StringRef varName, SpirvDebugSource *src,
      uint32_t line, uint32_t column, SpirvDebugInstruction *parentScope,
      llvm::StringRef linkageName, SpirvVariable *var, uint32_t flags,
      llvm::Optional<SpirvInstruction *> staticMemberDebugType = llvm::None);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugGlobalVariable)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugGlobalVariable;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvDebugSource *getSource() const { return source; }
  uint32_t getLine() const { return line; }
  uint32_t getColumn() const { return column; }
  SpirvDebugInstruction *getParentScope() const override { return parentScope; }
  llvm::StringRef getLinkageName() const { return linkageName; }
  uint32_t getFlags() const { return flags; }
  SpirvInstruction *getVariable() const { return var; }
  llvm::Optional<SpirvInstruction *> getStaticMemberDebugDecl() const {
    return staticMemberDebugDecl;
  }

private:
  SpirvDebugSource *source;
  uint32_t line;
  uint32_t column;
  SpirvDebugInstruction *parentScope;
  std::string linkageName;
  SpirvVariable *var;
  // TODO: Replace this with an enum, when it is available in SPIRV-Headers
  uint32_t flags;
  llvm::Optional<SpirvInstruction *> staticMemberDebugDecl;
};

class SpirvDebugOperation : public SpirvDebugInstruction {
public:
  SpirvDebugOperation(uint32_t operationOpCode,
                      llvm::ArrayRef<int32_t> operands = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugOperation)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugOperation;
  }

  bool invokeVisitor(Visitor *v) override;

  uint32_t getOperationOpcode() { return operationOpcode; }

private:
  uint32_t operationOpcode;
  llvm::SmallVector<int32_t, 2> operands;
};

class SpirvDebugExpression : public SpirvDebugInstruction {
public:
  SpirvDebugExpression(llvm::ArrayRef<SpirvDebugOperation *> operations = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugExpression)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugExpression;
  }

  bool invokeVisitor(Visitor *v) override;

  llvm::ArrayRef<SpirvDebugOperation *> getOperations() const {
    return operations;
  }

private:
  llvm::SmallVector<SpirvDebugOperation *, 4> operations;
};

class SpirvDebugDeclare : public SpirvDebugInstruction {
public:
  SpirvDebugDeclare(SpirvDebugLocalVariable *, SpirvInstruction *,
                    SpirvDebugExpression *, SourceLocation loc = {},
                    SourceRange range = {});

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugDeclare)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugDeclare;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvDebugLocalVariable *getDebugLocalVariable() const { return debugVar; }
  SpirvInstruction *getVariable() const { return var; }
  SpirvDebugExpression *getDebugExpression() const { return expression; }

private:
  SpirvDebugLocalVariable *debugVar;
  SpirvInstruction *var;
  SpirvDebugExpression *expression;
};

class SpirvDebugLexicalBlock : public SpirvDebugInstruction {
public:
  SpirvDebugLexicalBlock(SpirvDebugSource *source, uint32_t line,
                         uint32_t column, SpirvDebugInstruction *parent);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugLexicalBlock)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugLexicalBlock;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvDebugSource *getSource() const { return source; }
  uint32_t getLine() const { return line; }
  uint32_t getColumn() const { return column; }
  SpirvDebugInstruction *getParentScope() const override { return parent; }

private:
  SpirvDebugSource *source;
  uint32_t line;
  uint32_t column;
  SpirvDebugInstruction *parent;
};

/// Represent DebugScope. DebugScope has two operands: a lexical scope
/// and DebugInlinedAt. The DebugInlinedAt is an optional argument
/// and it is only used when we inline a function. Since DXC does not
/// conduct the inlining, we do not add DebugInlinedAt operand.
class SpirvDebugScope : public SpirvDebugInstruction {
public:
  SpirvDebugScope(SpirvDebugInstruction *);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugScope)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugScope;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvDebugInstruction *getScope() const { return scope; }

private:
  SpirvDebugInstruction *scope;
};

/// The following classes represent debug types defined in the rich DebugInfo
/// spec.
///
/// Note: While debug type and SPIR-V type are very similar, they are not quite
/// identical. For example: the debug type contains the HLL string name of the
/// type. Another example: two structs with similar definitions in different
/// lexical scopes translate into 1 SPIR-V type, but translate into 2 different
/// debug type instructions.
///
/// Note: DebugTypeComposite contains a vector of its members, which can be
/// DebugTypeMember or DebugFunction. The former is a type, and the latter is a
/// SPIR-V instruction. This somewhat tips the sclae in favor of using the
/// SpirvInstruction base class for representing debug types.
///
/// TODO: Add support for the following debug types:
///   DebugTypePointer,
///   DebugTypeQualifier,
///   DebugTypedef,
///   DebugTypeEnum,
///   DebugTypeInheritance,
///   DebugTypePtrToMember,
///   DebugTypeTemplate,
///   DebugTypeTemplateParameter,
///   DebugTypeTemplateTemplateParameter,
///   DebugTypeTemplateParameterPack,

class SpirvDebugType : public SpirvDebugInstruction {
public:
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() >= IK_DebugTypeBasic &&
           inst->getKind() <= IK_DebugTypeTemplateParameter;
  }

  virtual uint32_t getSizeInBits() const { return 0u; }

protected:
  SpirvDebugType(Kind kind, uint32_t opcode)
      : SpirvDebugInstruction(kind, opcode) {}
};

/// Represents basic debug types, including boolean, float, integer.
class SpirvDebugTypeBasic : public SpirvDebugType {
public:
  SpirvDebugTypeBasic(llvm::StringRef name, SpirvConstant *size,
                      uint32_t encoding);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugTypeBasic)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugTypeBasic;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvConstant *getSize() const { return size; }
  uint32_t getEncoding() const { return encoding; }
  uint32_t getSizeInBits() const override;

private:
  SpirvConstant *size;
  // TODO: Replace uint32_t with enum from SPIRV-Headers once available.
  // 0, Unspecified
  // 1, Address
  // 2, Boolean
  // 3, Float
  // 4, Signed
  // 5, SignedChar
  // 6, Unsigned
  // 7, UnsignedChar
  uint32_t encoding;
};

/// Represents array debug types
class SpirvDebugTypeArray : public SpirvDebugType {
public:
  SpirvDebugTypeArray(SpirvDebugType *elemType,
                      llvm::ArrayRef<uint32_t> elemCount);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugTypeArray)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugTypeArray;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvDebugType *getElementType() const { return elementType; }
  llvm::SmallVector<uint32_t, 2> &getElementCount() { return elementCount; }

  uint32_t getSizeInBits() const override {
    // TODO: avoid integer overflow
    uint32_t nElem = elementType->getSizeInBits();
    for (auto k : elementCount)
      nElem *= k;
    return nElem;
  }

private:
  SpirvDebugType *elementType;
  llvm::SmallVector<uint32_t, 2> elementCount;
};

/// Represents vector debug types
class SpirvDebugTypeVector : public SpirvDebugType {
public:
  SpirvDebugTypeVector(SpirvDebugType *elemType, uint32_t elemCount);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugTypeVector)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugTypeVector;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvDebugType *getElementType() const { return elementType; }
  uint32_t getElementCount() const { return elementCount; }

  uint32_t getSizeInBits() const override {
    return elementCount * elementType->getSizeInBits();
  }

private:
  SpirvDebugType *elementType;
  uint32_t elementCount;
};

/// Represents matrix debug types
class SpirvDebugTypeMatrix : public SpirvDebugType {
public:
  SpirvDebugTypeMatrix(SpirvDebugTypeVector *vectorType, uint32_t vectorCount);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugTypeMatrix)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugTypeMatrix;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvDebugTypeVector *getVectorType() const { return vectorType; }
  uint32_t getVectorCount() const { return vectorCount; }

  uint32_t getSizeInBits() const override {
    return vectorCount * vectorType->getSizeInBits();
  }

private:
  SpirvDebugTypeVector *vectorType;
  uint32_t vectorCount;
};

/// Represents a function debug type. Includes the function return type and
/// parameter types.
class SpirvDebugTypeFunction : public SpirvDebugType {
public:
  SpirvDebugTypeFunction(uint32_t flags, SpirvDebugType *ret,
                         llvm::ArrayRef<SpirvDebugType *> params);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugTypeFunction)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugTypeFunction;
  }

  bool invokeVisitor(Visitor *v) override;

  uint32_t getDebugFlags() const { return debugFlags; }
  SpirvDebugType *getReturnType() const { return returnType; }
  llvm::ArrayRef<SpirvDebugType *> getParamTypes() const { return paramTypes; }

private:
  // TODO: Replace uint32_t with enum in the SPIRV-Headers once it is available.
  uint32_t debugFlags;
  // Return Type is a debug instruction which represents type of return value of
  // the function. If the function has no return value, this operand is
  // OpTypeVoid, in which case we will use nullptr for this member.
  SpirvDebugType *returnType;
  llvm::SmallVector<SpirvDebugType *, 4> paramTypes;
};

/// Represents debug information for a template type parameter.
class SpirvDebugTypeTemplateParameter : public SpirvDebugType {
public:
  SpirvDebugTypeTemplateParameter(llvm::StringRef name, SpirvDebugType *type,
                                  SpirvInstruction *value,
                                  SpirvDebugSource *source, uint32_t line,
                                  uint32_t column);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugTypeTemplateParameter)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugTypeTemplateParameter;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvDebugType *getActualType() const { return actualType; }
  void setValue(SpirvInstruction *c) { value = c; }
  SpirvInstruction *getValue() const { return value; }
  SpirvDebugSource *getSource() const { return source; }
  uint32_t getLine() const { return line; }
  uint32_t getColumn() const { return column; }

private:
  SpirvDebugType *actualType; //< Type for type param
  SpirvInstruction *value;    //< Value. It must be null for type.
  SpirvDebugSource *source;   //< DebugSource containing this type
  uint32_t line;              //< Line number
  uint32_t column;            //< Column number
};

/// Represents debug information for a template type.
class SpirvDebugTypeTemplate : public SpirvDebugType {
public:
  SpirvDebugTypeTemplate(
      SpirvDebugInstruction *target,
      const llvm::SmallVector<SpirvDebugTypeTemplateParameter *, 2> &params);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugTypeTemplate)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugTypeTemplate;
  }

  bool invokeVisitor(Visitor *v) override;

  llvm::SmallVector<SpirvDebugTypeTemplateParameter *, 2> getParams() {
    return params;
  }
  SpirvDebugInstruction *getTarget() const { return target; }

private:
  // A debug instruction representing class, struct or function which has
  // template parameter(s).
  SpirvDebugInstruction *target;

  // Debug instructions representing the template parameters for this
  // particular instantiation. It must be DebugTypeTemplateParameter
  // or DebugTypeTemplateTemplateParameter.
  // TODO: change the type to SpirvDebugType * when we support
  // DebugTypeTemplateTemplateParameter.
  llvm::SmallVector<SpirvDebugTypeTemplateParameter *, 2> params;
};

/// Represents the debug type of a struct/union/class member.
/// Note: We have intentionally left out the pointer to the parent composite
/// type.
class SpirvDebugTypeMember : public SpirvDebugType {
public:
  SpirvDebugTypeMember(llvm::StringRef name, SpirvDebugType *type,
                       SpirvDebugSource *source, uint32_t line_,
                       uint32_t column_, SpirvDebugInstruction *parent,
                       uint32_t flags, uint32_t offsetInBits,
                       uint32_t sizeInBits, const APValue *value = nullptr);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugTypeMember)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugTypeMember;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvDebugInstruction *getParentScope() const override { return parent; }

  SpirvDebugSource *getSource() const { return source; }
  uint32_t getLine() const { return line; }
  uint32_t getColumn() const { return column; }
  uint32_t getOffsetInBits() const { return offsetInBits; }
  uint32_t getDebugFlags() const { return debugFlags; }
  uint32_t getSizeInBits() const override { return sizeInBits; }
  const APValue *getValue() const { return value; }

private:
  SpirvDebugSource *source; //< DebugSource
  uint32_t line;            //< Line number
  uint32_t column;          //< Column number

  SpirvDebugInstruction *parent; //< The parent DebugTypeComposite

  uint32_t offsetInBits; //< Offset (in bits) of this member in the struct
  uint32_t sizeInBits;   //< Size (in bits) of this member in the struct
  // TODO: Replace uint32_t with enum in the SPIRV-Headers once it is
  // available.
  uint32_t debugFlags;
  const APValue *value; //< Value (if static member)
};

class SpirvDebugTypeComposite : public SpirvDebugType {
public:
  SpirvDebugTypeComposite(llvm::StringRef name, SpirvDebugSource *source,
                          uint32_t line, uint32_t column,
                          SpirvDebugInstruction *parent,
                          llvm::StringRef linkageName, uint32_t flags,
                          uint32_t tag);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvDebugTypeComposite)

  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_DebugTypeComposite;
  }

  bool invokeVisitor(Visitor *v) override;

  llvm::SmallVector<SpirvDebugInstruction *, 4> getMembers() { return members; }
  void appendMember(SpirvDebugInstruction *member) {
    members.push_back(member);
  }
  void
  setMembers(const llvm::SmallVector<SpirvDebugInstruction *, 4> &memberTypes) {
    members = memberTypes;
  }

  SpirvDebugInstruction *getParentScope() const override { return parent; }
  uint32_t getTag() const { return tag; }
  SpirvDebugSource *getSource() const { return source; }
  uint32_t getLine() const { return line; }
  uint32_t getColumn() const { return column; }
  llvm::StringRef getLinkageName() const { return linkageName; }
  uint32_t getDebugFlags() const { return debugFlags; }

  void setSizeInBits(uint32_t size_) { sizeInBits = size_; }
  uint32_t getSizeInBits() const override { return sizeInBits; }

  void markAsOpaqueType(SpirvDebugInfoNone *none) {
    // If it was already marked as a opaque type, just return. For example,
    // `debugName` can be "@@Texture2D" if we call this method twice.
    if (debugNone == none && !debugName.empty() && debugName[0] == '@')
      return;
    debugName = std::string("@") + debugName;
    debugNone = none;
  }
  SpirvDebugInfoNone *getDebugInfoNone() const { return debugNone; }

private:
  SpirvDebugSource *source; //< DebugSource containing this type
  uint32_t line;            //< Line number
  uint32_t column;          //< Column number

  // The parent lexical scope. Must be one of the following:
  // DebugCompilationUnit, DebugFunction, DebugLexicalBlock or other
  // DebugTypeComposite
  SpirvDebugInstruction *parent; //< The parent lexical scope

  std::string linkageName; //< Linkage name
  uint32_t sizeInBits;     //< Size (in bits) of an instance of composite

  // TODO: Replace uint32_t with enum in the SPIRV-Headers once it is
  // available.
  uint32_t debugFlags;
  // TODO: Replace uint32_t with enum in the SPIRV-Headers once it is
  // available.
  uint32_t tag;

  // Pointer to members inside this composite type.
  // Note: Members must be ids of DebugTypeMember, DebugFunction or
  // DebugTypeInheritance. Since DebugFunction may be a member, we cannot use a
  // vector of SpirvDebugType.
  llvm::SmallVector<SpirvDebugInstruction *, 4> members;

  // When it is DebugTypeComposite for HLSL resource type i.e., opaque
  // type, we must put DebugInfoNone for Size operand.
  SpirvDebugInfoNone *debugNone;
};

class SpirvReadClock : public SpirvInstruction {
public:
  SpirvReadClock(QualType resultType, SpirvInstruction *scope, SourceLocation);

  DEFINE_RELEASE_MEMORY_FOR_CLASS(SpirvReadClock)

  // For LLVM-style RTTI
  static bool classof(const SpirvInstruction *inst) {
    return inst->getKind() == IK_ReadClock;
  }

  bool invokeVisitor(Visitor *v) override;

  SpirvInstruction *getScope() const { return scope; }

private:
  SpirvInstruction *scope;
};

#undef DECLARE_INVOKE_VISITOR_FOR_CLASS

} // namespace spirv
} // namespace clang

#endif // LLVM_CLANG_SPIRV_SPIRVINSTRUCTION_H
