

#pragma once

#include "dxc/DXIL/DxilCBuffer.h"
#include "dxc/DXIL/DxilNodeProps.h"
#include "dxc/Support/HLSLVersion.h"
#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include <memory>
#include <vector>

namespace clang {
class HLSLPatchConstantFuncAttr;
namespace CodeGen {
class CodeGenModule;
class CodeGenFunction;
} // namespace CodeGen
} // namespace clang

namespace llvm {
class Function;
class Module;
class Value;
class DebugLoc;
class Constant;
class GlobalVariable;
class CallInst;
class Instruction;
template <typename T, unsigned N> class SmallVector;
} // namespace llvm

namespace hlsl {
class HLModule;
struct DxilResourceProperties;
struct DxilFunctionProps;
class DxilFieldAnnotation;
enum class IntrinsicOp;
namespace dxilutil {
class ExportMap;
}
} // namespace hlsl

namespace CGHLSLMSHelper {

struct EntryFunctionInfo {
  clang::SourceLocation SL = clang::SourceLocation();
  llvm::Function *Func = nullptr;
};

// Map to save patch constant functions
struct PatchConstantInfo {
  clang::SourceLocation SL = clang::SourceLocation();
  llvm::Function *Func = nullptr;
  std::uint32_t NumOverloads = 0;
};

/// Use this class to represent HLSL cbuffer in high-level DXIL.
class HLCBuffer : public hlsl::DxilCBuffer {
public:
  HLCBuffer(bool bIsView, bool bIsTBuf)
      : bIsView(bIsView), bIsTBuf(bIsTBuf), bIsArray(false), ResultTy(nullptr) {
  }
  virtual ~HLCBuffer() = default;

  void AddConst(std::unique_ptr<DxilResourceBase> &pItem) {
    pItem->SetID(constants.size());
    constants.push_back(std::move(pItem));
  }

  std::vector<std::unique_ptr<DxilResourceBase>> &GetConstants() {
    return constants;
  }

  bool IsView() { return bIsView; }
  bool IsTBuf() { return bIsTBuf; }
  bool IsArray() { return bIsArray; }
  void SetIsArray() { bIsArray = true; }
  llvm::Type *GetResultType() { return ResultTy; }
  void SetResultType(llvm::Type *Ty) { ResultTy = Ty; }

private:
  std::vector<std::unique_ptr<DxilResourceBase>>
      constants; // constants inside const buffer
  bool bIsView;
  bool bIsTBuf;
  bool bIsArray;
  llvm::Type *ResultTy;
};
// Scope to help transform multiple returns.
struct Scope {
  enum class ScopeKind {
    IfScope,
    SwitchScope,
    LoopScope,
    ReturnScope,
    FunctionScope,
  };
  ScopeKind kind;
  llvm::BasicBlock *EndScopeBB;
  // Save loopContinueBB to create dxBreak.
  llvm::BasicBlock *loopContinueBB;
  // For case like
  // if () {
  //   ...
  //   return;
  // } else {
  //   ...
  //   return;
  // }
  //
  // both path is returned.
  // When whole scope is returned, go to parent scope directly.
  // Anything after it is unreachable.
  bool bWholeScopeReturned;
  unsigned parentScopeIndex;
  void dump();
};

class ScopeInfo {
public:
  ScopeInfo() {}
  ScopeInfo(llvm::Function *F, clang::SourceLocation loc);
  void AddIf(llvm::BasicBlock *endIfBB);
  void AddSwitch(llvm::BasicBlock *endSwitchBB);
  void AddLoop(llvm::BasicBlock *loopContinue, llvm::BasicBlock *endLoopBB);
  void AddRet(llvm::BasicBlock *bbWithRet);
  void AddCleanupBB(llvm::BasicBlock *cleanupBB) { hasCleanupBlocks = true; }
  Scope &EndScope(bool bScopeFinishedWithRet);
  Scope &GetScope(unsigned i);
  const llvm::SmallVector<unsigned, 2> &GetRetScopes() { return rets; }
  void LegalizeWholeReturnedScope();
  llvm::SmallVector<Scope, 16> &GetScopes() { return scopes; }
  bool CanSkipStructurize();
  void dump();
  bool HasCleanupBlocks() const { return hasCleanupBlocks; }
  clang::SourceLocation GetSourceLocation() const { return sourceLoc; }

private:
  void AddScope(Scope::ScopeKind k, llvm::BasicBlock *endScopeBB);
  bool hasCleanupBlocks = false;
  llvm::SmallVector<unsigned, 2> rets;
  unsigned maxRetLevel;
  bool bAllReturnsInIf;
  llvm::SmallVector<unsigned, 8> scopeStack;
  // save all scopes.
  llvm::SmallVector<Scope, 16> scopes;
  clang::SourceLocation sourceLoc;
};

// Map from value to resource/wave matrix properties.
// This only collect object variables(global/local/parameter), not object fields
// inside struct. Object fields inside struct is saved by TypeAnnotation.
struct DxilObjectProperties {
  bool AddResource(llvm::Value *V, const hlsl::DxilResourceProperties &RP);
  bool IsResource(llvm::Value *V);
  hlsl::DxilResourceProperties GetResource(llvm::Value *V);
  void updateCoherence(llvm::Value *V, bool updateGloballyCoherent,
                       bool updateReorderCoherent);

  // MapVector for deterministic iteration order.
  llvm::MapVector<llvm::Value *, hlsl::DxilResourceProperties> resMap;
};

void CopyAndAnnotateResourceArgument(llvm::Value *Src, llvm::Value *Dest,
                                     hlsl::DxilResourceProperties &RP,
                                     hlsl::HLModule &HLM,
                                     clang::CodeGen::CodeGenFunction &CGF);

// Align cbuffer offset in legacy mode (16 bytes per row).
unsigned AlignBufferOffsetInLegacy(unsigned offset, unsigned size,
                                   unsigned scalarSizeInBytes,
                                   bool bNeedNewRow);

void FinishEntries(hlsl::HLModule &HLM, const EntryFunctionInfo &Entry,
                   clang::CodeGen::CodeGenModule &CGM,
                   llvm::StringMap<EntryFunctionInfo> &entryFunctionMap,
                   std::unordered_map<llvm::Function *,
                                      const clang::HLSLPatchConstantFuncAttr *>
                       &HSEntryPatchConstantFuncAttr,
                   llvm::StringMap<PatchConstantInfo> &patchConstantFunctionMap,
                   std::unordered_map<llvm::Function *,
                                      std::unique_ptr<hlsl::DxilFunctionProps>>
                       &patchConstantFunctionPropsMap);

void FinishIntrinsics(
    hlsl::HLModule &HLM,
    std::vector<std::pair<llvm::Function *, unsigned>> &intrinsicMap,
    DxilObjectProperties &valToResPropertiesMap);

void AddDxBreak(llvm::Module &M,
                const llvm::SmallVector<llvm::BranchInst *, 16> &DxBreaks);

void ReplaceConstStaticGlobals(
    std::unordered_map<llvm::GlobalVariable *, std::vector<llvm::Constant *>>
        &staticConstGlobalInitListMap,
    std::unordered_map<llvm::GlobalVariable *, llvm::Function *>
        &staticConstGlobalCtorMap);

void FinishClipPlane(
    hlsl::HLModule &HLM, std::vector<llvm::Function *> &clipPlaneFuncList,
    std::unordered_map<llvm::Value *, llvm::DebugLoc> &debugInfoMap,
    clang::CodeGen::CodeGenModule &CGM);

void AddRegBindingsForResourceInConstantBuffer(
    hlsl::HLModule &HLM,
    llvm::DenseMap<
        llvm::Constant *,
        llvm::SmallVector<std::pair<hlsl::DXIL::ResourceClass, unsigned>, 1>>
        &constantRegBindingMap);

void FinishCBuffer(
    hlsl::HLModule &HLM, llvm::Type *CBufferType,
    std::unordered_map<llvm::Constant *, hlsl::DxilFieldAnnotation>
        &AnnotationMap);

void ProcessCtorFunctions(llvm::Module &M,
                          llvm::SmallVector<llvm::Function *, 2> &Ctors,
                          llvm::Function *Entry,
                          llvm::Function *PatchConstantFn);

void CollectCtorFunctions(llvm::Module &M, llvm::StringRef globalName,
                          llvm::SmallVector<llvm::Function *, 2> &Ctors,
                          clang::CodeGen::CodeGenModule &CGM);

void TranslateRayQueryConstructor(hlsl::HLModule &HLM);
void TranslateInputNodeRecordArgToHandle(
    hlsl::HLModule &HLM,
    llvm::MapVector<llvm::Argument *, hlsl::NodeInputRecordProps> &NodeParams);
void TranslateNodeOutputParamToHandle(
    hlsl::HLModule &HLM,
    llvm::MapVector<llvm::Argument *, hlsl::NodeProps> &NodeParams);
void UpdateLinkage(
    hlsl::HLModule &HLM, clang::CodeGen::CodeGenModule &CGM,
    hlsl::dxilutil::ExportMap &exportMap,
    llvm::StringMap<EntryFunctionInfo> &entryFunctionMap,
    llvm::StringMap<PatchConstantInfo> &patchConstantFunctionMap);

void StructurizeMultiRet(llvm::Module &M, clang::CodeGen::CodeGenModule &CGM,
                         llvm::DenseMap<llvm::Function *, ScopeInfo> &ScopeMap,
                         bool bWaveEnabledStage,
                         llvm::SmallVector<llvm::BranchInst *, 16> &DxBreaks);

llvm::Value *TryEvalIntrinsic(llvm::CallInst *CI, hlsl::IntrinsicOp intriOp,
                              hlsl::LangStd hlslVersion);
void SimpleTransformForHLDXIR(llvm::Module *pM);
void ExtensionCodeGen(hlsl::HLModule &HLM, clang::CodeGen::CodeGenModule &CGM);
} // namespace CGHLSLMSHelper
