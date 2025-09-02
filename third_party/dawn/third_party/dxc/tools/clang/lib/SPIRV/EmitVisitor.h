//===-- EmitVisitor.h - Emit Visitor ----------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_SPIRV_EMITVISITOR_H
#define LLVM_CLANG_SPIRV_EMITVISITOR_H

#include "clang/SPIRV/FeatureManager.h"
#include "clang/SPIRV/SpirvContext.h"
#include "clang/SPIRV/SpirvVisitor.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/StringMap.h"

#include <functional>

namespace clang {
namespace spirv {

class SpirvFunction;
class SpirvBasicBlock;
class SpirvType;

class EmitTypeHandler {
public:
  struct DecorationInfo {
    DecorationInfo(spv::Decoration decor, llvm::ArrayRef<uint32_t> params = {},
                   llvm::Optional<uint32_t> index = llvm::None)
        : decoration(decor), decorationParams(params.begin(), params.end()),
          memberIndex(index) {}

    bool operator==(const DecorationInfo &other) const {
      return decoration == other.decoration &&
             decorationParams == other.decorationParams &&
             memberIndex.hasValue() == other.memberIndex.hasValue() &&
             (!memberIndex.hasValue() ||
              memberIndex.getValue() == other.memberIndex.getValue());
    }

    spv::Decoration decoration;
    llvm::SmallVector<uint32_t, 4> decorationParams;
    llvm::Optional<uint32_t> memberIndex;
  };

public:
  EmitTypeHandler(ASTContext &astCtx, SpirvContext &spvContext,
                  const SpirvCodeGenOptions &opts, FeatureManager &featureMgr,
                  std::vector<uint32_t> *debugVec,
                  std::vector<uint32_t> *decVec, std::vector<uint32_t> *fwdVec,
                  std::vector<uint32_t> *typesVec,
                  const std::function<uint32_t()> &takeNextIdFn)
      : astContext(astCtx), context(spvContext), featureManager(featureMgr),
        debugVariableBinary(debugVec), annotationsBinary(decVec),
        fwdDeclBinary(fwdVec), typeConstantBinary(typesVec),
        takeNextIdFunction(takeNextIdFn), emittedConstantInts({}),
        emittedConstantFloats({}), emittedConstantComposites({}),
        emittedConstantNulls({}), emittedUndef({}), emittedConstantBools() {
    assert(decVec);
    assert(typesVec);
  }

  // Disable copy constructor/assignment.
  EmitTypeHandler(const EmitTypeHandler &) = delete;
  EmitTypeHandler &operator=(const EmitTypeHandler &) = delete;

  // Emits the instruction for the given type into the typeConstantBinary and
  // returns the result-id for the type. If the type has already been emitted,
  // it only returns its result-id.
  //
  // If any names are associated with the type (or its members in case of
  // structs), the OpName/OpMemberNames will also be emitted.
  //
  // If any decorations apply to the type, it also emits the decoration
  // instructions into the annotationsBinary.
  uint32_t emitType(const SpirvType *);

  // Emits OpDecorate (or OpMemberDecorate if memberIndex is non-zero)
  // targeting the given type. Uses the given decoration kind and its
  // parameters.
  void emitDecoration(uint32_t typeResultId, spv::Decoration,
                      llvm::ArrayRef<uint32_t> decorationParams,
                      llvm::Optional<uint32_t> memberIndex = llvm::None,
                      bool usesIdParams = false);

  uint32_t getOrCreateConstant(SpirvConstant *);

  // Emits an OpConstant instruction and returns its result-id.
  // For non-specialization constants, if an identical constant has already been
  // emitted, returns the existing constant's result-id.
  //
  // Note1: This method modifies the curTypeInst. Do not call in the middle of
  // construction of another instruction.
  //
  // Note 2: Integer constants may need to be generated for cases where there is
  // no SpirvConstantInteger instruction in the module. For example, we need to
  // emit an integer in order to create an array type. Therefore,
  // 'getOrCreateConstantInt' has a different signature than others. If a
  // constant instruction is provided, and it already has a result-id assigned,
  // it will be used. Otherwise a new result-id will be allocated for the
  // instruction.
  uint32_t
  getOrCreateConstantInt(llvm::APInt value, const SpirvType *type,
                         bool isSpecConst,
                         SpirvInstruction *constantInstruction = nullptr);
  uint32_t getOrCreateConstantFloat(SpirvConstantFloat *);
  uint32_t getOrCreateConstantComposite(SpirvConstantComposite *);
  uint32_t getOrCreateConstantNull(SpirvConstantNull *);
  uint32_t getOrCreateUndef(SpirvUndef *);
  uint32_t getOrCreateConstantBool(SpirvConstantBoolean *);
  uint32_t getOrCreateConstantString(SpirvConstantString *);
  template <typename vecType>
  void emitLiteral(const SpirvConstant *, vecType &outInst);
  template <typename vecType>
  void emitFloatLiteral(const SpirvConstantFloat *, vecType &outInst);
  template <typename vecType>
  void emitIntLiteral(const SpirvConstantInteger *, vecType &outInst);
  template <typename vecType>
  void emitIntLiteral(const llvm::APInt &literalVal, vecType &outInst);

private:
  void initTypeInstruction(spv::Op op);
  void finalizeTypeInstruction(bool isFwdDecl = false);

  // Returns the result-id for the given type and decorations. If a type with
  // the same decorations have already been used, it returns the existing
  // result-id. If not, creates a new result-id for such type and returns it.
  uint32_t getResultIdForType(const SpirvType *, bool *alreadyExists);

  // Emits an OpName (if memberIndex is not provided) or OpMemberName (if
  // memberIndex is provided) for the given target result-id.
  void emitNameForType(llvm::StringRef name, uint32_t targetTypeId,
                       llvm::Optional<uint32_t> memberIndex = llvm::None);

  void
  emitDecorationsForNodePayloadArrayTypes(const NodePayloadArrayType *npaType,
                                          uint32_t id);

  // There is no guarantee that an instruction or a function or a basic block
  // has been assigned result-id. This method returns the result-id for the
  // given object. If a result-id has not been assigned yet, it'll assign
  // one and return it.
  template <class T> uint32_t getOrAssignResultId(T *obj) {
    if (!obj->getResultId()) {
      obj->setResultId(takeNextIdFunction());
    }
    return obj->getResultId();
  }

private:
  /// Emits error to the diagnostic engine associated with this visitor.
  template <unsigned N>
  DiagnosticBuilder emitError(const char (&message)[N],
                              SourceLocation loc = {}) {
    const auto diagId = astContext.getDiagnostics().getCustomDiagID(
        clang::DiagnosticsEngine::Error, message);
    return astContext.getDiagnostics().Report(loc, diagId);
  }

private:
  ASTContext &astContext;
  SpirvContext &context;
  FeatureManager featureManager;
  std::vector<uint32_t> curTypeInst;
  std::vector<uint32_t> curDecorationInst;
  std::vector<uint32_t> *debugVariableBinary;
  std::vector<uint32_t> *annotationsBinary;
  std::vector<uint32_t> *fwdDeclBinary;
  std::vector<uint32_t> *typeConstantBinary;
  std::function<uint32_t()> takeNextIdFunction;

  // The array type requires the result-id of an OpConstant for its length. In
  // order to avoid duplicate OpConstant instructions, we keep a map of constant
  // uint value to the result-id of the OpConstant for that value.
  llvm::DenseMap<std::pair<uint64_t, const SpirvType *>, uint32_t>
      emittedConstantInts;
  llvm::DenseMap<std::pair<uint64_t, const SpirvType *>, uint32_t>
      emittedConstantFloats;
  llvm::DenseMap<StringRef, const SpirvConstantString *> emittedConstantStrings;
  llvm::SmallVector<SpirvConstantComposite *, 8> emittedConstantComposites;
  llvm::SmallVector<SpirvConstantNull *, 8> emittedConstantNulls;
  llvm::SmallVector<SpirvUndef *, 8> emittedUndef;
  SpirvConstantBoolean *emittedConstantBools[2];
  llvm::DenseSet<const SpirvInstruction *> emittedSpecConstantInstructions;

  // emittedTypes is a map that caches the result-id of types in order to avoid
  // emitting an identical type multiple times.
  llvm::DenseMap<const SpirvType *, uint32_t> emittedTypes;
};

/// \breif The visitor class that emits the SPIR-V words from the in-memory
/// representation.
class EmitVisitor : public Visitor {
public:
  /// \brief The struct representing a SPIR-V module header.
  struct Header {
    /// \brief Default constructs a SPIR-V module header with id bound 0.
    Header(uint32_t bound, uint32_t version);

    /// \brief Feeds the consumer with all the SPIR-V words for this header.
    std::vector<uint32_t> takeBinary();

    const uint32_t magicNumber;
    uint32_t version;
    const uint32_t generator;
    uint32_t bound;
    const uint32_t reserved;
  };

public:
  EmitVisitor(ASTContext &astCtx, SpirvContext &spvCtx,
              const SpirvCodeGenOptions &opts, FeatureManager &featureMgr)
      : Visitor(opts, spvCtx), astContext(astCtx), featureManager(featureMgr),
        id(0),
        typeHandler(astCtx, spvCtx, opts, featureMgr, &debugVariableBinary,
                    &annotationsBinary, &fwdDeclBinary, &typeConstantBinary,
                    [this]() -> uint32_t { return takeNextId(); }),
        debugMainFileId(0), debugInfoExtInstId(0), debugLineStart(0),
        debugLineEnd(0), debugColumnStart(0), debugColumnEnd(0),
        lastOpWasMergeInst(false), inEntryFunctionWrapper(false),
        hlslVersion(0) {}

  ~EmitVisitor();

  // Visit different SPIR-V constructs for emitting.
  bool visit(SpirvModule *, Phase phase) override;
  bool visit(SpirvFunction *, Phase phase) override;
  bool visit(SpirvBasicBlock *, Phase phase) override;

  bool visit(SpirvCapability *) override;
  bool visit(SpirvExtension *) override;
  bool visit(SpirvExtInstImport *) override;
  bool visit(SpirvMemoryModel *) override;
  bool visit(SpirvEmitVertex *) override;
  bool visit(SpirvEndPrimitive *) override;
  bool visit(SpirvEntryPoint *) override;
  bool visit(SpirvExecutionModeBase *) override;
  bool visit(SpirvString *) override;
  bool visit(SpirvSource *) override;
  bool visit(SpirvModuleProcessed *) override;
  bool visit(SpirvDecoration *) override;
  bool visit(SpirvVariable *) override;
  bool visit(SpirvFunctionParameter *) override;
  bool visit(SpirvLoopMerge *) override;
  bool visit(SpirvSelectionMerge *) override;
  bool visit(SpirvBranch *) override;
  bool visit(SpirvBranchConditional *) override;
  bool visit(SpirvKill *) override;
  bool visit(SpirvReturn *) override;
  bool visit(SpirvSwitch *) override;
  bool visit(SpirvUnreachable *) override;
  bool visit(SpirvAccessChain *) override;
  bool visit(SpirvAtomic *) override;
  bool visit(SpirvBarrier *) override;
  bool visit(SpirvIsNodePayloadValid *inst) override;
  bool visit(SpirvNodePayloadArrayLength *inst) override;
  bool visit(SpirvAllocateNodePayloads *inst) override;
  bool visit(SpirvEnqueueNodePayloads *inst) override;
  bool visit(SpirvFinishWritingNodePayload *inst) override;
  bool visit(SpirvBinaryOp *) override;
  bool visit(SpirvBitFieldExtract *) override;
  bool visit(SpirvBitFieldInsert *) override;
  bool visit(SpirvConstantBoolean *) override;
  bool visit(SpirvConstantInteger *) override;
  bool visit(SpirvConstantFloat *) override;
  bool visit(SpirvConstantComposite *) override;
  bool visit(SpirvConstantString *) override;
  bool visit(SpirvConstantNull *) override;
  bool visit(SpirvConvertPtrToU *) override;
  bool visit(SpirvConvertUToPtr *) override;
  bool visit(SpirvUndef *) override;
  bool visit(SpirvCompositeConstruct *) override;
  bool visit(SpirvCompositeExtract *) override;
  bool visit(SpirvCompositeInsert *) override;
  bool visit(SpirvExtInst *) override;
  bool visit(SpirvFunctionCall *) override;
  bool visit(SpirvGroupNonUniformOp *) override;
  bool visit(SpirvImageOp *) override;
  bool visit(SpirvImageQuery *) override;
  bool visit(SpirvImageSparseTexelsResident *) override;
  bool visit(SpirvImageTexelPointer *) override;
  bool visit(SpirvLoad *) override;
  bool visit(SpirvCopyObject *) override;
  bool visit(SpirvSampledImage *) override;
  bool visit(SpirvSelect *) override;
  bool visit(SpirvSpecConstantBinaryOp *) override;
  bool visit(SpirvSpecConstantUnaryOp *) override;
  bool visit(SpirvStore *) override;
  bool visit(SpirvNullaryOp *) override;
  bool visit(SpirvUnaryOp *) override;
  bool visit(SpirvVectorShuffle *) override;
  bool visit(SpirvArrayLength *) override;
  bool visit(SpirvRayTracingOpNV *) override;
  bool visit(SpirvDemoteToHelperInvocation *) override;
  bool visit(SpirvIsHelperInvocationEXT *) override;
  bool visit(SpirvRayQueryOpKHR *) override;
  bool visit(SpirvReadClock *) override;
  bool visit(SpirvRayTracingTerminateOpKHR *) override;
  bool visit(SpirvDebugInfoNone *) override;
  bool visit(SpirvDebugSource *) override;
  bool visit(SpirvDebugCompilationUnit *) override;
  bool visit(SpirvDebugLexicalBlock *) override;
  bool visit(SpirvDebugScope *) override;
  bool visit(SpirvDebugFunctionDeclaration *) override;
  bool visit(SpirvDebugFunction *) override;
  bool visit(SpirvDebugFunctionDefinition *) override;
  bool visit(SpirvDebugEntryPoint *) override;
  bool visit(SpirvDebugLocalVariable *) override;
  bool visit(SpirvDebugDeclare *) override;
  bool visit(SpirvDebugGlobalVariable *) override;
  bool visit(SpirvDebugExpression *) override;
  bool visit(SpirvDebugTypeBasic *) override;
  bool visit(SpirvDebugTypeVector *) override;
  bool visit(SpirvDebugTypeMatrix *) override;
  bool visit(SpirvDebugTypeArray *) override;
  bool visit(SpirvDebugTypeFunction *) override;
  bool visit(SpirvDebugTypeComposite *) override;
  bool visit(SpirvDebugTypeMember *) override;
  bool visit(SpirvDebugTypeTemplate *) override;
  bool visit(SpirvDebugTypeTemplateParameter *) override;
  bool visit(SpirvIntrinsicInstruction *) override;
  bool visit(SpirvEmitMeshTasksEXT *) override;
  bool visit(SpirvSetMeshOutputsEXT *) override;

  using Visitor::visit;

  // Returns the assembled binary built up in this visitor.
  std::vector<uint32_t> takeBinary();

private:
  // Returns the next available result-id.
  uint32_t takeNextId() { return ++id; }

  // There is no guarantee that an instruction or a function or a basic block
  // has been assigned result-id. This method returns the result-id for the
  // given object. If a result-id has not been assigned yet, it'll assign
  // one and return it.
  template <class T> uint32_t getOrAssignResultId(T *obj) {
    if (!obj->getResultId()) {
      obj->setResultId(takeNextId());
    }
    return obj->getResultId();
  }

  /// If we already created OpString for str, just return the id of the created
  /// one. Otherwise, create it, keep it in stringIdMap, and return its id.
  uint32_t getOrCreateOpStringId(llvm::StringRef str);

  // Generate DebugSource for inst
  void generateDebugSource(uint32_t fileId, uint32_t textId,
                           SpirvDebugSource *inst);

  // Generate DebugSourceContinued for inst
  void generateDebugSourceContinued(uint32_t textId, SpirvDebugSource *inst);

  /// Generate DebugSource and DebugSourceContinue for inst using previously
  /// generated fileId, chopping source into pieces as needed.
  void generateChoppedSource(uint32_t fileId, SpirvDebugSource *inst);

  /// In the OpenCL.DebugInfo.100 spec some parameters are literals, where in
  /// the NonSemantic.Shader.DebugInfo.100 spec they are encoded as constant
  /// operands. This function takes care of checking which version we are
  /// emitting and either returning the literal directly or a constant.
  uint32_t getLiteralEncodedForDebugInfo(uint32_t val);

  // Emits an OpLine instruction for the given operation into the given binary
  // section.
  void emitDebugLine(spv::Op op, const SourceLocation &loc,
                     const SourceRange &range, std::vector<uint32_t> *section,
                     bool isDebugScope = false);

  // Initiates the creation of a new instruction with the given Opcode.
  void initInstruction(spv::Op, const SourceLocation &);
  // Initiates the creation of the given SPIR-V instruction.
  // If the given instruction has a return type, it will also trigger emitting
  // the necessary type (and its associated decorations) and uses its result-id
  // in the instruction.
  void initInstruction(SpirvInstruction *);

  // Finalizes the current instruction by encoding the instruction size into the
  // first word, and then appends the current instruction to the given SPIR-V
  // binary section.
  void finalizeInstruction(std::vector<uint32_t> *section);

  // Encodes the given string into the current instruction that is being built.
  void encodeString(llvm::StringRef value);

  // Emits an OpName instruction into the debugBinary for the given target.
  void emitDebugNameForInstruction(uint32_t resultId, llvm::StringRef name);

  // TODO: Add a method for adding OpMemberName instructions for struct members
  // using the type information.

  // Returns the SPIR-V result id of the OpString for the File operand of
  // OpSource instruction.
  uint32_t getSourceFileId(SpirvSource *inst) {
    uint32_t fileId = debugMainFileId;
    if (inst->hasFile()) {
      fileId = getOrCreateOpStringId(inst->getFile()->getString());
    }
    return fileId;
  }

  // Returns true if we already emitted the OpSource instruction whose File
  // operand is |fileId|.
  bool isSourceWithFileEmitted(uint32_t fileId) {
    return emittedSource[fileId] != 0;
  }

  // Inserts the file id of OpSource instruction to the id of its
  // corresponding DebugSource instruction.
  void setFileOfSourceToDebugSourceId(uint32_t fileId, uint32_t dbg_src_id) {
    emittedSource[fileId] = dbg_src_id;
  }

  // Emits an OpCooperativeMatrixLength instruction into the main binary
  // section. It will replace the operand with the id of the type of the
  // operand.
  bool emitCooperativeMatrixLength(SpirvUnaryOp *inst);

private:
  /// Emits error to the diagnostic engine associated with this visitor.
  template <unsigned N>
  DiagnosticBuilder emitError(const char (&message)[N],
                              SourceLocation loc = {}) {
    const auto diagId = astContext.getDiagnostics().getCustomDiagID(
        clang::DiagnosticsEngine::Error, message);
    return astContext.getDiagnostics().Report(loc, diagId);
  }

private:
  // Object that holds Clang AST nodes.
  ASTContext &astContext;
  // Feature manager.
  FeatureManager featureManager;
  // The last result-id that's been used so far.
  uint32_t id;
  // Handler for emitting types and their related instructions.
  EmitTypeHandler typeHandler;
  // Current instruction being built
  SmallVector<uint32_t, 16> curInst;
  // All preamble instructions in the following order:
  // OpCapability, OpExtension, OpExtInstImport, OpMemoryModel, OpEntryPoint,
  // OpExecutionMode(Id)
  std::vector<uint32_t> preambleBinary;
  // Debug instructions related to file. Includes:
  // OpString, OpSourceExtension, OpSource, OpSourceContinued
  std::vector<uint32_t> debugFileBinary;
  // All debug instructions related to variable name. Includes:
  // OpName, OpMemberName, OpModuleProcessed
  std::vector<uint32_t> debugVariableBinary;
  // All annotation instructions: OpDecorate, OpMemberDecorate, OpGroupDecorate,
  // OpGroupMemberDecorate, and OpDecorationGroup.
  std::vector<uint32_t> annotationsBinary;
  // All forward pointer type declaration instructions
  std::vector<uint32_t> fwdDeclBinary;
  // All other type and constant instructions
  std::vector<uint32_t> typeConstantBinary;
  // All global variable declarations (all OpVariable instructions whose Storage
  // Class is not Function)
  std::vector<uint32_t> globalVarsBinary;
  // All Rich Debug Info instructions
  std::vector<uint32_t> richDebugInfo;
  // All other instructions
  std::vector<uint32_t> mainBinary;
  // String literals to SpirvString objects
  llvm::StringMap<uint32_t> stringIdMap;
  // String literals to SpirvConstantString objects
  llvm::StringMap<uint32_t> stringConstantIdMap;
  // String spec constants
  llvm::DenseSet<const SpirvInstruction *> stringSpecConstantInstructions;
  // Main file information for debugging that will be used by OpLine.
  uint32_t debugMainFileId;
  // Id for Vulkan DebugInfo extended instruction set. Used when generating
  // Debug[No]Line
  uint32_t debugInfoExtInstId;
  // One HLSL source line may result in several SPIR-V instructions. In order
  // to avoid emitting debug line instructions with identical line and column
  // numbers, we record the last line and column numbers that were used in a
  // debug line op, and only emit a new debug line op when a new line/column
  // in the source is discovered.
  uint32_t debugLineStart;
  uint32_t debugLineEnd;
  uint32_t debugColumnStart;
  uint32_t debugColumnEnd;
  // True if the last emitted instruction was OpSelectionMerge or OpLoopMerge.
  bool lastOpWasMergeInst;
  // True if currently it enters an entry function wrapper.
  bool inEntryFunctionWrapper;
  // Map of filename string id to the id of its DebugSource instruction. When
  // generating OpSource instruction without a result id, use 1 to remember it
  // was generated.
  llvm::DenseMap<uint32_t, uint32_t> emittedSource;
  uint32_t hlslVersion;
  // Vector to contain SpirvInstruction objects created by this class. The
  // destructor of this class will release them.
  std::vector<SpirvInstruction *> spvInstructions;
};

} // namespace spirv
} // namespace clang

#endif // LLVM_CLANG_SPIRV_EMITVISITOR_H
