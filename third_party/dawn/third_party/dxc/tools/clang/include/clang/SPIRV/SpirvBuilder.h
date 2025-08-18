//===-- SpirvBuilder.h - SPIR-V Builder -----------------------*- C++ -*---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_SPIRV_SPIRVBUILDER_H
#define LLVM_CLANG_SPIRV_SPIRVBUILDER_H

#include "clang/SPIRV/FeatureManager.h"
#include "clang/SPIRV/SpirvBasicBlock.h"
#include "clang/SPIRV/SpirvContext.h"
#include "clang/SPIRV/SpirvFunction.h"
#include "clang/SPIRV/SpirvInstruction.h"
#include "clang/SPIRV/SpirvModule.h"

#include "spirv/unified1/NonSemanticDebugPrintf.h"

namespace clang {
namespace spirv {

// Provides StringMapInfo for std::string so we can create a DenseMap with key
// of type std::string.
struct StringMapInfo {
  static inline std::string getEmptyKey() { return ""; }
  static inline std::string getTombstoneKey() { return ""; }
  static unsigned getHashValue(const std::string &Val) {
    return llvm::hash_combine(Val);
  }
  static bool isEqual(const std::string &LHS, const std::string &RHS) {
    // Either both are null, or both should have the same underlying type.
    return LHS == RHS;
  }
};

/// The SPIR-V in-memory representation builder class.
///
/// This class exports API for constructing SPIR-V in-memory representation
/// interactively. Under the hood, it allocates SPIR-V entity objects from
/// SpirvContext and wires them up into a connected structured representation.
///
/// At any time, there can only exist at most one function under building;
/// but there can exist multiple basic blocks under construction.
///
/// Call `getModule()` to get the SPIR-V words after finishing building the
/// module.
class SpirvBuilder {
  friend class CapabilityVisitor;

public:
  SpirvBuilder(ASTContext &ac, SpirvContext &c, const SpirvCodeGenOptions &,
               FeatureManager &featureMgr);
  ~SpirvBuilder() = default;

  // Forbid copy construction and assignment
  SpirvBuilder(const SpirvBuilder &) = delete;
  SpirvBuilder &operator=(const SpirvBuilder &) = delete;

  // Forbid move construction and assignment
  SpirvBuilder(SpirvBuilder &&) = delete;
  SpirvBuilder &operator=(SpirvBuilder &&) = delete;

  /// Returns the SPIR-V module being built.
  SpirvModule *getModule() { return mod.get(); }

  // === Function and Basic Block ===

  /// \brief Creates a SpirvFunction object with the given information and adds
  /// it to list of all discovered functions in the SpirvModule.
  SpirvFunction *createSpirvFunction(QualType returnType, SourceLocation,
                                     llvm::StringRef name, bool isPrecise,
                                     bool isNoInline = false);

  /// \brief Begins building a SPIR-V function by allocating a SpirvFunction
  /// object. Returns the pointer for the function on success. Returns nullptr
  /// on failure.
  ///
  /// At any time, there can only exist at most one function under building.
  SpirvFunction *beginFunction(QualType returnType, SourceLocation,
                               llvm::StringRef name = "",
                               bool isPrecise = false, bool isNoInline = false,
                               SpirvFunction *func = nullptr);

  /// \brief Creates and registers a function parameter of the given pointer
  /// type in the current function and returns its pointer.
  SpirvFunctionParameter *addFnParam(QualType ptrType, bool isPrecise,
                                     bool isNointerp, SourceLocation,
                                     llvm::StringRef name = "");

  /// \brief Creates a local variable of the given type in the current
  /// function and returns it.
  ///
  /// The corresponding pointer type of the given type will be constructed in
  /// this method for the variable itself.
  SpirvVariable *addFnVar(QualType valueType, SourceLocation,
                          llvm::StringRef name = "", bool isPrecise = false,
                          bool isNointerp = false,
                          SpirvInstruction *init = nullptr);

  /// \brief Ends building of the current function. All basic blocks constructed
  /// from the beginning or after ending the previous function will be collected
  /// into this function.
  void endFunction();

  /// \brief Creates a SPIR-V basic block. On success, returns the <label-id>
  /// for the basic block. On failure, returns zero.
  SpirvBasicBlock *createBasicBlock(llvm::StringRef name = "");

  /// \brief Creates a SPIR-V rich DebugInfo DebugScope instruction.
  /// On success, returns the <id> of DebugScope. On failure, returns nullptr.
  SpirvDebugScope *createDebugScope(SpirvDebugInstruction *scope);

  /// \brief Adds the basic block with the given label as a successor to the
  /// current basic block.
  void addSuccessor(SpirvBasicBlock *successorBB);

  /// \brief Sets the merge target to the given basic block.
  /// The caller must make sure the current basic block contains an
  /// OpSelectionMerge or OpLoopMerge instruction.
  void setMergeTarget(SpirvBasicBlock *mergeLabel);

  /// \brief Sets the continue target to the given basic block.
  /// The caller must make sure the current basic block contains an
  /// OpLoopMerge instruction.
  void setContinueTarget(SpirvBasicBlock *continueLabel);

  /// \brief Returns true if the current basic block inserting into is
  /// terminated.
  inline bool isCurrentBasicBlockTerminated() const {
    return insertPoint && insertPoint->hasTerminator();
  }

  /// \brief Sets insertion point to the given basic block.
  inline void setInsertPoint(SpirvBasicBlock *bb) { insertPoint = bb; }

  /// \brief Gets insertion point.
  inline SpirvBasicBlock *getInsertPoint() { return insertPoint; }

  // === Instruction at the current Insertion Point ===

  /// \brief Creates a composite construct instruction with the given
  /// <result-type> and constituents and returns the pointer of the
  /// composite instruction.
  SpirvCompositeConstruct *
  createCompositeConstruct(QualType resultType,
                           llvm::ArrayRef<SpirvInstruction *> constituents,
                           SourceLocation loc, SourceRange range = {});

  /// \brief Creates a composite extract instruction. The given composite is
  /// indexed using the given literal indexes to obtain the resulting element.
  /// Returns the instruction pointer for the extracted element.
  SpirvCompositeExtract *
  createCompositeExtract(QualType resultType, SpirvInstruction *composite,
                         llvm::ArrayRef<uint32_t> indexes, SourceLocation loc,
                         SourceRange range = {});

  /// \brief Creates a composite insert instruction. The given object will
  /// replace the component in the composite at the given indices. Returns the
  /// instruction pointer for the new composite.
  SpirvCompositeInsert *createCompositeInsert(QualType resultType,
                                              SpirvInstruction *composite,
                                              llvm::ArrayRef<uint32_t> indices,
                                              SpirvInstruction *object,
                                              SourceLocation loc,
                                              SourceRange range = {});

  /// \brief Creates a vector shuffle instruction of selecting from the two
  /// vectors using selectors and returns the instruction pointer of the result
  /// vector.
  SpirvVectorShuffle *createVectorShuffle(QualType resultType,
                                          SpirvInstruction *vector1,
                                          SpirvInstruction *vector2,
                                          llvm::ArrayRef<uint32_t> selectors,
                                          SourceLocation loc,
                                          SourceRange range = {});

  /// \brief Creates a load sequence loading the value of the given
  /// <result-type> from the given pointer (load + optional extraction,
  /// ex:bitfield). Returns the instruction pointer for the loaded value.
  SpirvInstruction *createLoad(QualType resultType, SpirvInstruction *pointer,
                               SourceLocation loc, SourceRange range = {});
  SpirvLoad *createLoad(const SpirvType *resultType, SpirvInstruction *pointer,
                        SourceLocation loc, SourceRange range = {});

  /// \brief Creates an OpCopyObject instruction from the given pointer.
  SpirvCopyObject *createCopyObject(QualType resultType,
                                    SpirvInstruction *pointer, SourceLocation);

  /// \brief Creates a store sequence storing the given value into the given
  /// address. Returns the instruction pointer for the store instruction.
  /// This function handles storing to bitfields.
  SpirvStore *createStore(SpirvInstruction *address, SpirvInstruction *value,
                          SourceLocation loc, SourceRange range = {});

  /// \brief Creates a function call instruction and returns the instruction
  /// pointer for the return value.
  SpirvFunctionCall *
  createFunctionCall(QualType returnType, SpirvFunction *func,
                     llvm::ArrayRef<SpirvInstruction *> params,
                     SourceLocation loc, SourceRange range = {});

  /// \brief Creates an access chain instruction to retrieve the element from
  /// the given base by walking through the given indexes. Returns the
  /// instruction pointer for the pointer to the element.
  /// Note: The given 'resultType' should be the underlying value type, not the
  /// pointer type. The type lowering pass automatically adds pointerness and
  /// proper storage class (based on the access base) to the result type.
  SpirvAccessChain *
  createAccessChain(QualType resultType, SpirvInstruction *base,
                    llvm::ArrayRef<SpirvInstruction *> indexes,
                    SourceLocation loc, SourceRange range = {});
  SpirvAccessChain *
  createAccessChain(const SpirvType *resultType, SpirvInstruction *base,
                    llvm::ArrayRef<SpirvInstruction *> indexes,
                    SourceLocation loc);

  /// \brief Creates a unary operation with the given SPIR-V opcode. Returns
  /// the instruction pointer for the result.
  SpirvUnaryOp *createUnaryOp(spv::Op op, QualType resultType,
                              SpirvInstruction *operand, SourceLocation loc,
                              SourceRange range = {});
  SpirvUnaryOp *createUnaryOp(spv::Op op, const SpirvType *resultType,
                              SpirvInstruction *operand, SourceLocation loc);

  /// \brief Creates a binary operation with the given SPIR-V opcode. Returns
  /// the instruction pointer for the result.
  SpirvBinaryOp *createBinaryOp(spv::Op op, QualType resultType,
                                SpirvInstruction *lhs, SpirvInstruction *rhs,
                                SourceLocation loc, SourceRange range = {});

  SpirvSpecConstantBinaryOp *createSpecConstantBinaryOp(spv::Op op,
                                                        QualType resultType,
                                                        SpirvInstruction *lhs,
                                                        SpirvInstruction *rhs,
                                                        SourceLocation loc);

  /// \brief Creates an operation with the given OpGroupNonUniform* SPIR-V
  /// opcode.
  SpirvGroupNonUniformOp *createGroupNonUniformOp(
      spv::Op op, QualType resultType, llvm::Optional<spv::Scope> execScope,
      llvm::ArrayRef<SpirvInstruction *> operands, SourceLocation,
      llvm::Optional<spv::GroupOperation> groupOp = llvm::None);

  /// \brief Creates an atomic instruction with the given parameters and returns
  /// its pointer.
  SpirvAtomic *createAtomicOp(spv::Op opcode, QualType resultType,
                              SpirvInstruction *orignalValuePtr,
                              spv::Scope scope,
                              spv::MemorySemanticsMask memorySemantics,
                              SpirvInstruction *valueToOp, SourceLocation,
                              SourceRange range = {});
  SpirvAtomic *createAtomicCompareExchange(
      QualType resultType, SpirvInstruction *orignalValuePtr, spv::Scope scope,
      spv::MemorySemanticsMask equalMemorySemantics,
      spv::MemorySemanticsMask unequalMemorySemantics,
      SpirvInstruction *valueToOp, SpirvInstruction *comparator, SourceLocation,
      SourceRange range = {});

  /// \brief Creates an OpSampledImage SPIR-V instruction with proper
  /// decorations for the given parameters.
  SpirvSampledImage *createSampledImage(QualType, SpirvInstruction *image,
                                        SpirvInstruction *sampler,
                                        SourceLocation loc,
                                        SourceRange range = {});

  /// \brief Creates an OpImageTexelPointer SPIR-V instruction with the given
  /// parameters.
  SpirvImageTexelPointer *createImageTexelPointer(QualType resultType,
                                                  SpirvInstruction *image,
                                                  SpirvInstruction *coordinate,
                                                  SpirvInstruction *sample,
                                                  SourceLocation);

  /// \brief Creates an OpConverPtrToU SPIR-V instruction with the given
  /// parameters.
  SpirvConvertPtrToU *createConvertPtrToU(SpirvInstruction *ptr, QualType type);

  /// \brief Creates an OpConverUToPtr SPIR-V instruction with the given
  /// parameters.
  SpirvConvertUToPtr *createConvertUToPtr(SpirvInstruction *val, QualType type);

  /// \brief Creates SPIR-V instructions for sampling the given image.
  ///
  /// If compareVal is given a non-zero value, *Dref* variants of OpImageSample*
  /// will be generated.
  ///
  /// If lod or grad is given a non-zero value, *ExplicitLod variants of
  /// OpImageSample* will be generated; otherwise, *ImplicitLod variant will
  /// be generated.
  ///
  /// If bias, lod, grad, or minLod is given a non-zero value, an additional
  /// image operands, Bias, Lod, Grad, or MinLod will be attached to the current
  /// instruction, respectively. Panics if both lod and minLod are non-zero.
  ///
  /// If residencyCodeId is not zero, the sparse version of the instructions
  /// will be used, and the SPIR-V instruction for storing the resulting
  /// residency code will also be emitted.
  ///
  /// If isNonUniform is true, the sampled image will be decorated with
  /// NonUniformEXT.
  SpirvInstruction *
  createImageSample(QualType texelType, QualType imageType,
                    SpirvInstruction *image, SpirvInstruction *sampler,
                    SpirvInstruction *coordinate, SpirvInstruction *compareVal,
                    SpirvInstruction *bias, SpirvInstruction *lod,
                    std::pair<SpirvInstruction *, SpirvInstruction *> grad,
                    SpirvInstruction *constOffset, SpirvInstruction *varOffset,
                    SpirvInstruction *constOffsets, SpirvInstruction *sample,
                    SpirvInstruction *minLod, SpirvInstruction *residencyCodeId,
                    SourceLocation loc, SourceRange range = {});

  /// \brief Creates SPIR-V instructions for reading a texel from an image. If
  /// doImageFetch is true, OpImageFetch is used. OpImageRead is used otherwise.
  /// OpImageFetch should be used for sampled images. OpImageRead should be used
  /// for images without a sampler.
  ///
  /// If residencyCodeId is not zero, the sparse version of the instructions
  /// will be used, and the SPIR-V instruction for storing the resulting
  /// residency code will also be emitted.
  SpirvInstruction *createImageFetchOrRead(
      bool doImageFetch, QualType texelType, QualType imageType,
      SpirvInstruction *image, SpirvInstruction *coordinate,
      SpirvInstruction *lod, SpirvInstruction *constOffset,
      SpirvInstruction *constOffsets, SpirvInstruction *sample,
      SpirvInstruction *residencyCode, SourceLocation loc,
      SourceRange range = {});

  /// \brief Creates SPIR-V instructions for writing to the given image.
  void createImageWrite(QualType imageType, SpirvInstruction *image,
                        SpirvInstruction *coord, SpirvInstruction *texel,
                        SourceLocation loc, SourceRange range = {});

  /// \brief Creates SPIR-V instructions for gathering the given image.
  ///
  /// If compareVal is given a non-null value, OpImageDrefGather or
  /// OpImageSparseDrefGather will be generated; otherwise, OpImageGather or
  /// OpImageSparseGather will be generated.
  /// If residencyCode is not null, the sparse version of the instructions
  /// will be used, and the SPIR-V instruction for storing the resulting
  /// residency code will also be emitted.
  /// If isNonUniform is true, the sampled image will be decorated with
  /// NonUniformEXT.
  SpirvInstruction *
  createImageGather(QualType texelType, QualType imageType,
                    SpirvInstruction *image, SpirvInstruction *sampler,
                    SpirvInstruction *coordinate, SpirvInstruction *component,
                    SpirvInstruction *compareVal, SpirvInstruction *constOffset,
                    SpirvInstruction *varOffset, SpirvInstruction *constOffsets,
                    SpirvInstruction *sample, SpirvInstruction *residencyCode,
                    SourceLocation loc, SourceRange range = {});

  /// \brief Creates an OpImageSparseTexelsResident SPIR-V instruction for the
  /// given Resident Code and returns the instruction pointer.
  SpirvImageSparseTexelsResident *
  createImageSparseTexelsResident(SpirvInstruction *resident_code,
                                  SourceLocation, SourceRange range = {});

  /// \brief Creates an image query instruction.
  /// The given 'lod' is used as the Lod argument in the case of
  /// OpImageQuerySizeLod, and it is used as the 'coordinate' parameter in the
  /// case of OpImageQueryLod.
  SpirvImageQuery *createImageQuery(spv::Op opcode, QualType resultType,
                                    SourceLocation loc, SpirvInstruction *image,
                                    SpirvInstruction *lod = nullptr,
                                    SourceRange range = {});

  /// \brief Creates a select operation with the given values for true and false
  /// cases and returns the instruction pointer.
  SpirvSelect *createSelect(QualType resultType, SpirvInstruction *condition,
                            SpirvInstruction *trueValue,
                            SpirvInstruction *falseValue, SourceLocation,
                            SourceRange range = {});

  /// \brief Creates a switch statement for the given selector, default, and
  /// branches. Results in OpSelectionMerge followed by OpSwitch.
  void
  createSwitch(SpirvBasicBlock *mergeLabel, SpirvInstruction *selector,
               SpirvBasicBlock *defaultLabel,
               llvm::ArrayRef<std::pair<llvm::APInt, SpirvBasicBlock *>> target,
               SourceLocation, SourceRange);

  /// \brief Creates a fragment-shader discard via by emitting OpKill.
  void createKill(SourceLocation, SourceRange range = {});

  /// \brief Creates an unconditional branch to the given target label.
  /// If mergeBB and continueBB are non-null, it creates an OpLoopMerge
  /// instruction followed by an unconditional branch to the given target label.
  void createBranch(
      SpirvBasicBlock *targetLabel, SourceLocation loc,
      SpirvBasicBlock *mergeBB = nullptr, SpirvBasicBlock *continueBB = nullptr,
      spv::LoopControlMask loopControl = spv::LoopControlMask::MaskNone,
      SourceRange range = {});

  /// \brief Creates a conditional branch. An OpSelectionMerge instruction
  /// will be created if mergeLabel is not null and continueLabel is null.
  /// An OpLoopMerge instruction will also be created if both continueLabel
  /// and mergeLabel are not null. For other cases, mergeLabel and continueLabel
  /// will be ignored. If selection control mask and/or loop control mask are
  /// provided, they will be applied to the corresponding SPIR-V instruction.
  /// Otherwise, MaskNone will be used.
  void createConditionalBranch(
      SpirvInstruction *condition, SpirvBasicBlock *trueLabel,
      SpirvBasicBlock *falseLabel, SourceLocation loc,
      SpirvBasicBlock *mergeLabel = nullptr,
      SpirvBasicBlock *continueLabel = nullptr,
      spv::SelectionControlMask selectionControl =
          spv::SelectionControlMask::MaskNone,
      spv::LoopControlMask loopControl = spv::LoopControlMask::MaskNone,
      SourceRange range = {});

  /// \brief Creates a return instruction.
  void createReturn(SourceLocation, SourceRange range = {});
  /// \brief Creates a return value instruction.
  void createReturnValue(SpirvInstruction *value, SourceLocation,
                         SourceRange range = {});

  /// \brief Creates an OpExtInst instruction for the GLSL extended instruction
  /// set, with the given instruction number, and operands. Returns the
  /// resulting instruction pointer.
  SpirvInstruction *
  createGLSLExtInst(QualType resultType, GLSLstd450 instId,
                    llvm::ArrayRef<SpirvInstruction *> operands, SourceLocation,
                    SourceRange range = {});
  SpirvInstruction *
  createGLSLExtInst(const SpirvType *resultType, GLSLstd450 instId,
                    llvm::ArrayRef<SpirvInstruction *> operands, SourceLocation,
                    SourceRange range = {});

  /// \brief Creates an OpExtInst instruction for the NonSemantic.DebugPrintf
  /// extension set. Returns the resulting instruction pointer.
  SpirvInstruction *createNonSemanticDebugPrintfExtInst(
      QualType resultType, NonSemanticDebugPrintfInstructions instId,
      llvm::ArrayRef<SpirvInstruction *> operands, SourceLocation);

  SpirvInstruction *createIsNodePayloadValid(SpirvInstruction *payloadArray,
                                             SpirvInstruction *nodeIndex,
                                             SourceLocation);

  SpirvInstruction *createNodePayloadArrayLength(SpirvInstruction *payloadArray,
                                                 SourceLocation);

  SpirvInstruction *createAllocateNodePayloads(QualType resultType,
                                               spv::Scope allocationScope,
                                               SpirvInstruction *shaderIndex,
                                               SpirvInstruction *recordCount,
                                               SourceLocation);

  void createEnqueueOutputNodePayloads(SpirvInstruction *payload,
                                       SourceLocation);

  SpirvInstruction *createFinishWritingNodePayload(SpirvInstruction *payload,
                                                   SourceLocation);

  /// \brief Creates an OpMemoryBarrier or OpControlBarrier instruction with the
  /// given flags. If execution scope (exec) is provided, an OpControlBarrier
  /// is created; otherwise an OpMemoryBarrier is created.
  void createBarrier(spv::Scope memoryScope,
                     spv::MemorySemanticsMask memorySemantics,
                     llvm::Optional<spv::Scope> exec, SourceLocation,
                     SourceRange range = {});

  /// \brief Creates an OpBitFieldInsert SPIR-V instruction for the given
  /// arguments.
  SpirvInstruction *createBitFieldInsert(QualType resultType,
                                         SpirvInstruction *base,
                                         SpirvInstruction *insert,
                                         unsigned bitOffset, unsigned bitCount,
                                         SourceLocation, SourceRange);

  /// \brief Creates an OpBitFieldUExtract or OpBitFieldSExtract SPIR-V
  /// instruction for the given arguments.
  SpirvInstruction *createBitFieldExtract(QualType resultType,
                                          SpirvInstruction *base,
                                          unsigned bitOffset, unsigned bitCount,
                                          SourceLocation, SourceRange);

  /// \brief Creates an OpEmitVertex instruction.
  void createEmitVertex(SourceLocation, SourceRange range = {});

  /// \brief Creates an OpEndPrimitive instruction.
  void createEndPrimitive(SourceLocation, SourceRange range = {});

  /// \brief Creates an OpEmitMeshTasksEXT instruction.
  void createEmitMeshTasksEXT(SpirvInstruction *xDim, SpirvInstruction *yDim,
                              SpirvInstruction *zDim, SourceLocation loc,
                              SpirvInstruction *payload = nullptr,
                              SourceRange range = {});

  /// \brief Creates an OpSetMeshOutputsEXT instruction.
  void createSetMeshOutputsEXT(SpirvInstruction *vertCount,
                               SpirvInstruction *primCount, SourceLocation loc,
                               SourceRange range = {});

  /// \brief Creates an OpArrayLength instruction.
  SpirvArrayLength *createArrayLength(QualType resultType, SourceLocation loc,
                                      SpirvInstruction *structure,
                                      uint32_t arrayMember,
                                      SourceRange range = {});

  /// \brief Creates SPIR-V instructions for NV raytracing ops.
  SpirvInstruction *
  createRayTracingOpsNV(spv::Op opcode, QualType resultType,
                        llvm::ArrayRef<SpirvInstruction *> operands,
                        SourceLocation loc);

  /// \brief Creates an OpDemoteToHelperInvocation instruction.
  SpirvInstruction *createDemoteToHelperInvocation(SourceLocation);

  /// \brief Creates an OpIsHelperInvocationEXT instruction.
  SpirvInstruction *createIsHelperInvocationEXT(QualType type,
                                                SourceLocation loc);

  // === SPIR-V Rich Debug Info Creation ===
  SpirvDebugSource *createDebugSource(llvm::StringRef file,
                                      llvm::StringRef text = "");

  SpirvDebugCompilationUnit *createDebugCompilationUnit(SpirvDebugSource *);

  void createDebugEntryPoint(SpirvDebugFunction *ep,
                             SpirvDebugCompilationUnit *cu,
                             llvm::StringRef signature, llvm::StringRef args);

  SpirvDebugLexicalBlock *
  createDebugLexicalBlock(SpirvDebugSource *, uint32_t line, uint32_t column,
                          SpirvDebugInstruction *parent);

  SpirvDebugLocalVariable *createDebugLocalVariable(
      QualType debugType, llvm::StringRef varName, SpirvDebugSource *src,
      uint32_t line, uint32_t column, SpirvDebugInstruction *parentScope,
      uint32_t flags, llvm::Optional<uint32_t> argNumber = llvm::None);

  SpirvDebugGlobalVariable *createDebugGlobalVariable(
      QualType debugType, llvm::StringRef varName, SpirvDebugSource *src,
      uint32_t line, uint32_t column, SpirvDebugInstruction *parentScope,
      llvm::StringRef linkageName, SpirvVariable *var, uint32_t flags,
      llvm::Optional<SpirvInstruction *> staticMemberDebugType = llvm::None);

  // Get a DebugInfoNone if exists. Otherwise, create one and return it.
  SpirvDebugInfoNone *getOrCreateDebugInfoNone();

  // Get a null DebugExpression if exists. Otherwise, create one and return it.
  SpirvDebugExpression *getOrCreateNullDebugExpression();

  SpirvDebugDeclare *createDebugDeclare(
      SpirvDebugLocalVariable *dbgVar, SpirvInstruction *var,
      SourceLocation loc = {}, SourceRange range = {},
      llvm::Optional<SpirvDebugExpression *> dbgExpr = llvm::None);

  SpirvDebugFunction *
  createDebugFunction(const FunctionDecl *decl, llvm::StringRef name,
                      SpirvDebugSource *src, uint32_t fnLine, uint32_t fnColumn,
                      SpirvDebugInstruction *parentScope,
                      llvm::StringRef linkageName, uint32_t flags,
                      uint32_t scopeLine, SpirvFunction *fn);

  SpirvDebugFunctionDefinition *
  createDebugFunctionDef(SpirvDebugFunction *function, SpirvFunction *fn);

  /// \brief Create SPIR-V instructions for KHR RayQuery ops
  SpirvInstruction *
  createRayQueryOpsKHR(spv::Op opcode, QualType resultType,
                       llvm::ArrayRef<SpirvInstruction *> operands,
                       bool cullFlags, SourceLocation loc,
                       SourceRange range = {});
  /// \brief Creates an OpReadClockKHR instruction.
  SpirvInstruction *createReadClock(SpirvInstruction *scope, SourceLocation);

  /// \brief Create Raytracing terminate Ops
  /// OpIgnoreIntersectionKHR/OpTerminateIntersectionKHR
  void createRaytracingTerminateKHR(spv::Op opcode, SourceLocation loc);

  /// \brief Create spirv intrinsic instructions
  SpirvInstruction *createSpirvIntrInstExt(
      uint32_t opcode, QualType retType,
      llvm::ArrayRef<SpirvInstruction *> operands,
      llvm::ArrayRef<llvm::StringRef> extensions, llvm::StringRef instSet,
      llvm::ArrayRef<uint32_t> capablities, SourceLocation loc);

  /// \brief Creates an OpBeginInvocationInterlockEXT instruction.
  void createBeginInvocationInterlockEXT(SourceLocation loc, SourceRange range);

  /// \brief Creates an OpEndInvocationInterlockEXT instruction.
  void createEndInvocationInterlockEXT(SourceLocation loc, SourceRange range);

  /// \brief Returns a clone SPIR-V variable for CTBuffer with FXC memory layout
  /// and creates copy instructions from the CTBuffer to the clone variable in
  /// module.init if it contains HLSL matrix 1xN. Otherwise, returns nullptr.
  ///
  /// Motivation for this clone variable:
  /// We translate a matrix type1xN as a vector typeN in all code generation,
  /// but type1xN in CTBuffer with FXC memory layout rule must have a stride 16
  /// bytes between elements. Since we cannot set a stride for a SPIR-V vector,
  /// we must use a SPIR-V array type[N] with stride 16 bytes for it. Since we
  /// translate it into a vector typeN for all places, it has side effects. We
  /// use a clone variable to fix this issue i.e.,
  ///   1. Use the CTBuffer to receive the data from CPU
  ///   2. Copy it to the clone variable
  ///   3. Use the clone variable in all the places
  SpirvInstruction *initializeCloneVarForFxcCTBuffer(SpirvInstruction *instr);

  /// \brief Adds a module variable with the Private storage class for a
  /// stage variable with [[vk::builtin(HelperInvocation)]] attribute and
  /// initializes it as the result of OpIsHelperInvocationEXT instruction.
  ///
  /// Note that we must not use it for Vulkan 1.3 or above. Vulkan 1.3 or
  /// above allows us to use HelperInvocation Builtin decoration for stage
  /// variables.
  SpirvVariable *addVarForHelperInvocation(QualType type, bool isPrecise,
                                           SourceLocation loc);

  // === SPIR-V Module Structure ===
  inline void setMemoryModel(spv::AddressingModel, spv::MemoryModel);

  /// \brief Adds an entry point for the module under construction. We only
  /// support a single entry point per module for now.
  inline void addEntryPoint(spv::ExecutionModel em, SpirvFunction *target,
                            llvm::StringRef targetName,
                            llvm::ArrayRef<SpirvVariable *> interfaces);

  /// \brief Sets the shader model version, source file name, and source file
  /// content. Returns the SpirvString instruction of the file name.
  inline SpirvString *setDebugSource(uint32_t major, uint32_t minor,
                                     const std::vector<llvm::StringRef> &name,
                                     llvm::StringRef content = "");

  /// \brief Adds an execution mode to the module under construction if it does
  /// not already exist. Return the newly added instruction or the existing
  /// instruction, if one already exists.
  inline SpirvInstruction *addExecutionMode(SpirvFunction *entryPoint,
                                            spv::ExecutionMode em,
                                            llvm::ArrayRef<uint32_t> params,
                                            SourceLocation);

  /// \brief Adds an execution mode to the module under construction if it does
  /// not already exist. Return the newly added instruction or the existing
  /// instruction, if one already exists.
  inline SpirvInstruction *
  addExecutionModeId(SpirvFunction *entryPoint, spv::ExecutionMode em,
                     llvm::ArrayRef<SpirvInstruction *> params,
                     SourceLocation loc);

  /// \brief Adds an OpModuleProcessed instruction to the module under
  /// construction.
  void addModuleProcessed(llvm::StringRef process);

  /// \brief If not added already, adds an OpExtInstImport (import of extended
  /// instruction set) of the rich DebugInfo instruction set, either OpenCL or
  /// Vulkan.
  /// Returns the imported instruction set.
  SpirvExtInstImport *getDebugInfoExtInstSet(bool vulkanDebugInfo);

  /// \brief Adds a stage input/ouput variable whose value is of the given type.
  ///
  /// Note: the corresponding pointer type of the given type will not be
  /// constructed in this method.
  SpirvVariable *addStageIOVar(QualType type, spv::StorageClass storageClass,
                               llvm::StringRef name, bool isPrecise,
                               bool isNointerp, SourceLocation loc);

  /// \brief Adds a stage builtin variable whose value is of the given type.
  ///
  /// Note: The corresponding pointer type of the given type will not be
  /// constructed in this method.
  SpirvVariable *addStageBuiltinVar(QualType type,
                                    spv::StorageClass storageClass,
                                    spv::BuiltIn, bool isPrecise,
                                    SourceLocation loc);

  /// \brief Adds a module variable. This variable should not have the Function
  /// storage class.
  ///
  /// Note: The corresponding pointer type of the given type will not be
  /// constructed in this method.
  SpirvVariable *
  addModuleVar(QualType valueType, spv::StorageClass storageClass,
               bool isPrecise, bool isNointerp, llvm::StringRef name = "",
               llvm::Optional<SpirvInstruction *> init = llvm::None,
               SourceLocation loc = {});

  // Adds a variable to the module.
  SpirvVariable *
  addModuleVar(const SpirvType *valueType, spv::StorageClass storageClass,
               bool isPrecise, bool isNointerp, llvm::StringRef name = "",
               llvm::Optional<SpirvInstruction *> init = llvm::None,
               SourceLocation loc = {});

  // Adds a variable to the module. It will be placed in the variable list
  // before `pos`.
  SpirvVariable *
  addModuleVar(const SpirvType *valueType, spv::StorageClass storageClass,
               bool isPrecise, bool isNointerp, SpirvInstruction *before,
               llvm::StringRef name = "",
               llvm::Optional<SpirvInstruction *> init = llvm::None,
               SourceLocation loc = {});

  /// \brief Decorates the given target with the given location.
  void decorateLocation(SpirvInstruction *target, uint32_t location);

  /// \brief Decorates the given target with the given component.
  void decorateComponent(SpirvInstruction *target, uint32_t component);

  /// \brief Decorates the given target with the given index.
  void decorateIndex(SpirvInstruction *target, uint32_t index, SourceLocation);

  /// \brief Decorates the given target with the given descriptor set and
  /// binding number.
  void decorateDSetBinding(SpirvVariable *target, uint32_t setNumber,
                           uint32_t bindingNumber);

  /// \brief Decorates the given target with the given SpecId.
  void decorateSpecId(SpirvInstruction *target, uint32_t specId,
                      SourceLocation);

  /// \brief Decorates the given target with the given input attchment index
  /// number.
  void decorateInputAttachmentIndex(SpirvInstruction *target,
                                    uint32_t indexNumber, SourceLocation);

  /// \brief Decorates the given main buffer with the given counter buffer.
  void decorateCounterBuffer(SpirvInstruction *mainBuffer,
                             SpirvInstruction *counterBuffer, SourceLocation);

  /// \brief Decorates the given target with the given HLSL semantic string.
  void decorateHlslSemantic(SpirvInstruction *target, llvm::StringRef semantic,
                            llvm::Optional<uint32_t> memberIdx = llvm::None);

  /// \brief Decorates the given target with centroid
  void decorateCentroid(SpirvInstruction *target, SourceLocation);

  /// \brief Decorates the given target with flat
  void decorateFlat(SpirvInstruction *target, SourceLocation);

  /// \brief Decorates the given target with noperspective
  void decorateNoPerspective(SpirvInstruction *target, SourceLocation);

  /// \brief Decorates the given target with sample
  void decorateSample(SpirvInstruction *target, SourceLocation);

  /// \brief Decorates the given target with patch
  void decoratePatch(SpirvInstruction *target, SourceLocation);

  /// \brief Decorates the given target with NoContraction
  void decorateNoContraction(SpirvInstruction *target, SourceLocation);

  /// \brief Decorates the given target with PerPrimitiveNV
  void decoratePerPrimitiveNV(SpirvInstruction *target, SourceLocation);

  /// \brief Decorates the given target with PerTaskNV
  void decoratePerTaskNV(SpirvInstruction *target, uint32_t offset,
                         SourceLocation);

  /// \brief Decorates the given target with PerVertexKHR
  void decoratePerVertexKHR(SpirvInstruction *argInst, SourceLocation);

  /// \brief Decorates the given target with Coherent
  void decorateCoherent(SpirvInstruction *target, SourceLocation);

  /// \brief Decorates the given target with LinkageAttributes
  /// We have to set targetInst as nullptr when it is an imported or exported
  /// function.
  /// We have to set targetFunc as nullptr when it is an imported or
  /// exported global variable.
  void decorateLinkage(SpirvInstruction *targetInst, SpirvFunction *targetFunc,
                       llvm::StringRef name, spv::LinkageType linkageType,
                       SourceLocation);

  /// \brief Decorates the given target with information from VKDecorateExt
  void decorateWithLiterals(SpirvInstruction *targetInst, unsigned decorate,
                            llvm::ArrayRef<unsigned> literals, SourceLocation);

  /// \brief Decorates the given target with result ids of SPIR-V
  /// instructions.
  void decorateWithIds(SpirvInstruction *targetInst, unsigned decorate,
                       llvm::ArrayRef<SpirvInstruction *> ids, SourceLocation);

  /// \brief Decorates the given target with the given strings.
  void decorateWithStrings(SpirvInstruction *target, unsigned decorate,
                           llvm::ArrayRef<llvm::StringRef> strLiteral,
                           SourceLocation loc);

  /// --- Constants ---
  /// Each of these methods can acquire a unique constant from the SpirvContext,
  /// and add the context to the list of constants in the module.
  SpirvConstant *getConstantInt(QualType type, llvm::APInt value,
                                bool specConst = false);
  SpirvConstant *getConstantFloat(QualType type, llvm::APFloat value,
                                  bool specConst = false);
  SpirvConstant *getConstantBool(bool value, bool specConst = false);
  SpirvConstant *
  getConstantComposite(QualType compositeType,
                       llvm::ArrayRef<SpirvConstant *> constituents,
                       bool specConst = false);
  SpirvConstant *getConstantNull(QualType);
  SpirvConstant *getConstantString(llvm::StringRef str, bool specConst = false);
  SpirvUndef *getUndef(QualType);

  SpirvString *createString(llvm::StringRef str);
  SpirvString *getString(llvm::StringRef str);

  const HybridPointerType *getPhysicalStorageBufferType(QualType pointee);
  const SpirvPointerType *
  getPhysicalStorageBufferType(const SpirvType *pointee);

  void setPerVertexInterpMode(bool b);
  bool isPerVertexInterpMode();

  void addPerVertexStgInputFuncVarEntry(SpirvInstruction *k,
                                        SpirvInstruction *v);
  SpirvInstruction *getPerVertexStgInput(SpirvInstruction *k);

  std::vector<uint32_t> takeModule();

  /// \brief Adds the given capability to the module under construction due to
  /// the feature used at the given source location.
  inline void requireCapability(spv::Capability, SourceLocation loc = {});

  /// \brief Returns true if the module requires the given capability.
  inline bool hasCapability(spv::Capability cap);

  /// \brief Adds an extension to the module under construction for translating
  /// the given target at the given source location.
  inline void requireExtension(llvm::StringRef extension, SourceLocation);

private:
  /// \brief If not added already, adds an OpExtInstImport (import of extended
  /// instruction set) for the given instruction set. Returns the imported
  /// instruction set.
  SpirvExtInstImport *getExtInstSet(llvm::StringRef extensionName);

  /// \brief Returns the composed ImageOperandsMask from non-zero parameters
  /// and pushes non-zero parameters to *orderedParams in the expected order.
  spv::ImageOperandsMask composeImageOperandsMask(
      SpirvInstruction *bias, SpirvInstruction *lod,
      const std::pair<SpirvInstruction *, SpirvInstruction *> &grad,
      SpirvInstruction *constOffset, SpirvInstruction *varOffset,
      SpirvInstruction *constOffsets, SpirvInstruction *sample,
      SpirvInstruction *minLod);

  /// \brief Creates instructions to copy sub-components of CTBuffer src to its
  /// clone dst. This method assumes
  ///   1. src has a pointer type to a type with FXC memory layout rule
  ///   2. dst has a pointer type to a type with void memory layout rule
  void
  createCopyInstructionsFromFxcCTBufferToClone(SpirvInstruction *fxcCTBuffer,
                                               SpirvInstruction *clone);
  void createCopyArrayInFxcCTBufferToClone(const ArrayType *fxcCTBufferArrTy,
                                           SpirvInstruction *fxcCTBuffer,
                                           const SpirvType *cloneType,
                                           SpirvInstruction *clone,
                                           SourceLocation loc);
  void createCopyStructInFxcCTBufferToClone(
      const StructType *fxcCTBufferStructTy, SpirvInstruction *fxcCTBuffer,
      const SpirvType *cloneType, SpirvInstruction *clone, SourceLocation loc);

  /// \brief Sets moduleInitInsertPoint as insertPoint.
  void switchInsertPointToModuleInit();

  /// \brief Adds OpFunctionCall instructions for ModuleInit to all entry
  /// points.
  void addModuleInitCallToEntryPoints();

  /// \brief Ends building of the module initialization function.
  void endModuleInitFunction();

  /// \brief Creates a clone SPIR-V variable for CTBuffer.
  SpirvVariable *createCloneVarForFxcCTBuffer(QualType astType,
                                              const SpirvType *spvType,
                                              SpirvInstruction *var);

  /// \brief Emulates OpBitFieldInsert SPIR-V instruction for the given
  /// arguments.
  SpirvInstruction *
  createEmulatedBitFieldInsert(QualType resultType, uint32_t baseTypeBitwidth,
                               SpirvInstruction *base, SpirvInstruction *insert,
                               unsigned bitOffset, unsigned bitCount,
                               SourceLocation, SourceRange);

  SpirvInstruction *
  createEmulatedBitFieldExtract(QualType resultType, uint32_t baseTypeBitwidth,
                                SpirvInstruction *base, unsigned bitOffset,
                                unsigned bitCount, SourceLocation loc,
                                SourceRange range);

private:
  ASTContext &astContext;
  SpirvContext &context; ///< From which we allocate various SPIR-V object
  FeatureManager &featureManager;

  std::unique_ptr<SpirvModule> mod; ///< The current module being built
  SpirvFunction *function;          ///< The current function being built
  SpirvBasicBlock *insertPoint;     ///< The current basic block being built

  SpirvFunction *moduleInit;              ///< The module initialization
                                          ///< function
  SpirvBasicBlock *moduleInitInsertPoint; ///< The basic block of the module
                                          ///< initialization function

  const SpirvCodeGenOptions &spirvOptions; ///< Command line options.

  /// A struct containing information regarding a builtin variable.
  struct BuiltInVarInfo {
    BuiltInVarInfo(spv::StorageClass s, spv::BuiltIn b, SpirvVariable *v)
        : sc(s), builtIn(b), variable(v) {}
    spv::StorageClass sc;
    spv::BuiltIn builtIn;
    SpirvVariable *variable;
  };
  /// Used as caches for all created builtin variables to avoid duplication.
  llvm::SmallVector<BuiltInVarInfo, 16> builtinVars;

  SpirvDebugInfoNone *debugNone;

  /// DebugExpression that does not reference any DebugOperation
  SpirvDebugExpression *nullDebugExpr;

  // To avoid generating multiple OpStrings for the same string literal
  // the SpirvBuilder will generate and reuse them. The empty string is
  // kept track of separately. This is because the empty string is used
  // as the EmptyKey and TombstoneKey for the map, prohibiting insertion
  // of the empty string as a contained value.
  llvm::DenseMap<std::string, SpirvString *, StringMapInfo> stringLiterals;
  SpirvString *emptyString;

  /// Mapping of CTBuffers including matrix 1xN with FXC memory layout to their
  /// clone variables. We need it to avoid multiple clone variables for the same
  /// CTBuffer.
  llvm::DenseMap<SpirvVariable *, SpirvVariable *> fxcCTBufferToClone;

  /// Mapping of a temporary stage parameter variable to real stage input
  /// variables, only when the declaration has attribute `nointerpolation`
  llvm::DenseMap<SpirvInstruction *, SpirvInstruction *> perVertexInputVarMap;
};

void SpirvBuilder::requireCapability(spv::Capability cap, SourceLocation loc) {
  auto *capability = new (context) SpirvCapability(loc, cap);
  if (mod->addCapability(capability)) {
    if (cap == spv::Capability::PhysicalStorageBufferAddresses) {
      mod->promoteAddressingModel(
          spv::AddressingModel::PhysicalStorageBuffer64);
    }
  } else {
    capability->releaseMemory();
  }
}

bool SpirvBuilder::hasCapability(spv::Capability cap) {
  SpirvCapability capability({}, cap);
  return mod->hasCapability(capability);
}

void SpirvBuilder::requireExtension(llvm::StringRef ext, SourceLocation loc) {
  auto *extension = new (context) SpirvExtension(loc, ext);
  if (!mod->addExtension(extension))
    extension->releaseMemory();
}

void SpirvBuilder::setMemoryModel(spv::AddressingModel addrModel,
                                  spv::MemoryModel memModel) {
  mod->setMemoryModel(new (context) SpirvMemoryModel(addrModel, memModel));
}

void SpirvBuilder::addEntryPoint(spv::ExecutionModel em, SpirvFunction *target,
                                 llvm::StringRef targetName,
                                 llvm::ArrayRef<SpirvVariable *> interfaces) {
  mod->addEntryPoint(new (context) SpirvEntryPoint(
      target->getSourceLocation(), em, target, targetName, interfaces));
}

SpirvString *
SpirvBuilder::setDebugSource(uint32_t major, uint32_t minor,
                             const std::vector<llvm::StringRef> &fileNames,
                             llvm::StringRef content) {
  uint32_t version = 100 * major + 10 * minor;
  SpirvSource *mainSource = nullptr;
  for (const auto &name : fileNames) {
    SpirvString *fileString = name.empty() ? nullptr : getString(name);
    SpirvSource *debugSource = new (context)
        SpirvSource(/*SourceLocation*/ {}, spv::SourceLanguage::HLSL, version,
                    fileString, content);
    mod->addSource(debugSource);
    if (!mainSource)
      mainSource = debugSource;
  }

  // If mainSource is nullptr, fileNames is empty and no input file is
  // specified. We must create a SpirvSource for OpSource HLSL <version>.
  if (!mainSource) {
    mainSource = new (context)
        SpirvSource(/*SourceLocation*/ {}, spv::SourceLanguage::HLSL, version,
                    nullptr, content);
    mod->addSource(mainSource);
  }
  return mainSource->getFile();
}

SpirvInstruction *
SpirvBuilder::addExecutionMode(SpirvFunction *entryPoint, spv::ExecutionMode em,
                               llvm::ArrayRef<uint32_t> params,
                               SourceLocation loc) {
  SpirvExecutionMode *mode = nullptr;
  SpirvExecutionModeBase *existingInstruction =
      mod->findExecutionMode(entryPoint, em);

  if (!existingInstruction) {
    mode = new (context) SpirvExecutionMode(loc, entryPoint, em, params);
    mod->addExecutionMode(mode);
  } else {
    // No execution mode can be used with both OpExecutionMode and
    // OpExecutionModeId. If this assert is triggered, then either this
    // `addExecutionModeId` should have been called with `em` or the existing
    // instruction is wrong.
    assert(existingInstruction->getKind() ==
           SpirvInstruction::IK_ExecutionMode);
    mode = cast<SpirvExecutionMode>(existingInstruction);
  }

  return mode;
}

SpirvInstruction *SpirvBuilder::addExecutionModeId(
    SpirvFunction *entryPoint, spv::ExecutionMode em,
    llvm::ArrayRef<SpirvInstruction *> params, SourceLocation loc) {
  SpirvExecutionModeId *mode = nullptr;
  SpirvExecutionModeBase *existingInstruction =
      mod->findExecutionMode(entryPoint, em);
  if (!existingInstruction) {
    mode = new (context) SpirvExecutionModeId(loc, entryPoint, em, params);
    mod->addExecutionMode(mode);
  } else {
    // No execution mode can be used with both OpExecutionMode and
    // OpExecutionModeId. If this assert is triggered, then either this
    // `addExecutionMode` should have been called with `em` or the existing
    // instruction is wrong.
    assert(existingInstruction->getKind() ==
           SpirvInstruction::IK_ExecutionModeId);
    mode = cast<SpirvExecutionModeId>(existingInstruction);
  }

  return mode;
}

} // end namespace spirv
} // end namespace clang

#endif // LLVM_CLANG_SPIRV_SPIRVBUILDER_H
