//===-- AddressSanitizer.cpp - memory error detector ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of AddressSanitizer, an address sanity checker.
// Details of the algorithm:
//  http://code.google.com/p/address-sanitizer/wiki/AddressSanitizerAlgorithm
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Instrumentation.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/SwapByteOrder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/ASanStackFrameLayout.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include <algorithm>
#include <string>
#include <system_error>

using namespace llvm;

#define DEBUG_TYPE "asan"

static const uint64_t kDefaultShadowScale = 3;
static const uint64_t kDefaultShadowOffset32 = 1ULL << 29;
static const uint64_t kIOSShadowOffset32 = 1ULL << 30;
static const uint64_t kDefaultShadowOffset64 = 1ULL << 44;
static const uint64_t kSmallX86_64ShadowOffset = 0x7FFF8000;  // < 2G.
static const uint64_t kLinuxKasan_ShadowOffset64 = 0xdffffc0000000000;
static const uint64_t kPPC64_ShadowOffset64 = 1ULL << 41;
static const uint64_t kMIPS32_ShadowOffset32 = 0x0aaa0000;
static const uint64_t kMIPS64_ShadowOffset64 = 1ULL << 37;
static const uint64_t kAArch64_ShadowOffset64 = 1ULL << 36;
static const uint64_t kFreeBSD_ShadowOffset32 = 1ULL << 30;
static const uint64_t kFreeBSD_ShadowOffset64 = 1ULL << 46;
static const uint64_t kWindowsShadowOffset32 = 3ULL << 28;

static const size_t kMinStackMallocSize = 1 << 6;   // 64B
static const size_t kMaxStackMallocSize = 1 << 16;  // 64K
static const uintptr_t kCurrentStackFrameMagic = 0x41B58AB3;
static const uintptr_t kRetiredStackFrameMagic = 0x45E0360E;

static const char *const kAsanModuleCtorName = "asan.module_ctor";
static const char *const kAsanModuleDtorName = "asan.module_dtor";
static const uint64_t kAsanCtorAndDtorPriority = 1;
static const char *const kAsanReportErrorTemplate = "__asan_report_";
static const char *const kAsanRegisterGlobalsName = "__asan_register_globals";
static const char *const kAsanUnregisterGlobalsName =
    "__asan_unregister_globals";
static const char *const kAsanPoisonGlobalsName = "__asan_before_dynamic_init";
static const char *const kAsanUnpoisonGlobalsName = "__asan_after_dynamic_init";
static const char *const kAsanInitName = "__asan_init_v5";
static const char *const kAsanPtrCmp = "__sanitizer_ptr_cmp";
static const char *const kAsanPtrSub = "__sanitizer_ptr_sub";
static const char *const kAsanHandleNoReturnName = "__asan_handle_no_return";
static const int kMaxAsanStackMallocSizeClass = 10;
static const char *const kAsanStackMallocNameTemplate = "__asan_stack_malloc_";
static const char *const kAsanStackFreeNameTemplate = "__asan_stack_free_";
static const char *const kAsanGenPrefix = "__asan_gen_";
static const char *const kSanCovGenPrefix = "__sancov_gen_";
static const char *const kAsanPoisonStackMemoryName =
    "__asan_poison_stack_memory";
static const char *const kAsanUnpoisonStackMemoryName =
    "__asan_unpoison_stack_memory";

static const char *const kAsanOptionDetectUAR =
    "__asan_option_detect_stack_use_after_return";

static const char *const kAsanAllocaPoison = "__asan_alloca_poison";
static const char *const kAsanAllocasUnpoison = "__asan_allocas_unpoison";

// Accesses sizes are powers of two: 1, 2, 4, 8, 16.
static const size_t kNumberOfAccessSizes = 5;

static const unsigned kAllocaRzSize = 32;

// Command-line flags.
static cl::opt<bool> ClEnableKasan(
    "asan-kernel", cl::desc("Enable KernelAddressSanitizer instrumentation"),
    cl::Hidden, cl::init(false));

// This flag may need to be replaced with -f[no-]asan-reads.
static cl::opt<bool> ClInstrumentReads("asan-instrument-reads",
                                       cl::desc("instrument read instructions"),
                                       cl::Hidden, cl::init(true));
static cl::opt<bool> ClInstrumentWrites(
    "asan-instrument-writes", cl::desc("instrument write instructions"),
    cl::Hidden, cl::init(true));
static cl::opt<bool> ClInstrumentAtomics(
    "asan-instrument-atomics",
    cl::desc("instrument atomic instructions (rmw, cmpxchg)"), cl::Hidden,
    cl::init(true));
static cl::opt<bool> ClAlwaysSlowPath(
    "asan-always-slow-path",
    cl::desc("use instrumentation with slow path for all accesses"), cl::Hidden,
    cl::init(false));
// This flag limits the number of instructions to be instrumented
// in any given BB. Normally, this should be set to unlimited (INT_MAX),
// but due to http://llvm.org/bugs/show_bug.cgi?id=12652 we temporary
// set it to 10000.
static cl::opt<int> ClMaxInsnsToInstrumentPerBB(
    "asan-max-ins-per-bb", cl::init(10000),
    cl::desc("maximal number of instructions to instrument in any given BB"),
    cl::Hidden);
// This flag may need to be replaced with -f[no]asan-stack.
static cl::opt<bool> ClStack("asan-stack", cl::desc("Handle stack memory"),
                             cl::Hidden, cl::init(true));
static cl::opt<bool> ClUseAfterReturn("asan-use-after-return",
                                      cl::desc("Check return-after-free"),
                                      cl::Hidden, cl::init(true));
// This flag may need to be replaced with -f[no]asan-globals.
static cl::opt<bool> ClGlobals("asan-globals",
                               cl::desc("Handle global objects"), cl::Hidden,
                               cl::init(true));
static cl::opt<bool> ClInitializers("asan-initialization-order",
                                    cl::desc("Handle C++ initializer order"),
                                    cl::Hidden, cl::init(true));
static cl::opt<bool> ClInvalidPointerPairs(
    "asan-detect-invalid-pointer-pair",
    cl::desc("Instrument <, <=, >, >=, - with pointer operands"), cl::Hidden,
    cl::init(false));
static cl::opt<unsigned> ClRealignStack(
    "asan-realign-stack",
    cl::desc("Realign stack to the value of this flag (power of two)"),
    cl::Hidden, cl::init(32));
static cl::opt<int> ClInstrumentationWithCallsThreshold(
    "asan-instrumentation-with-call-threshold",
    cl::desc(
        "If the function being instrumented contains more than "
        "this number of memory accesses, use callbacks instead of "
        "inline checks (-1 means never use callbacks)."),
    cl::Hidden, cl::init(7000));
static cl::opt<std::string> ClMemoryAccessCallbackPrefix(
    "asan-memory-access-callback-prefix",
    cl::desc("Prefix for memory access callbacks"), cl::Hidden,
    cl::init("__asan_"));
static cl::opt<bool> ClInstrumentAllocas("asan-instrument-allocas",
                                         cl::desc("instrument dynamic allocas"),
                                         cl::Hidden, cl::init(false));
static cl::opt<bool> ClSkipPromotableAllocas(
    "asan-skip-promotable-allocas",
    cl::desc("Do not instrument promotable allocas"), cl::Hidden,
    cl::init(true));

// These flags allow to change the shadow mapping.
// The shadow mapping looks like
//    Shadow = (Mem >> scale) + (1 << offset_log)
static cl::opt<int> ClMappingScale("asan-mapping-scale",
                                   cl::desc("scale of asan shadow mapping"),
                                   cl::Hidden, cl::init(0));

// Optimization flags. Not user visible, used mostly for testing
// and benchmarking the tool.
static cl::opt<bool> ClOpt("asan-opt", cl::desc("Optimize instrumentation"),
                           cl::Hidden, cl::init(true));
static cl::opt<bool> ClOptSameTemp(
    "asan-opt-same-temp", cl::desc("Instrument the same temp just once"),
    cl::Hidden, cl::init(true));
static cl::opt<bool> ClOptGlobals("asan-opt-globals",
                                  cl::desc("Don't instrument scalar globals"),
                                  cl::Hidden, cl::init(true));
static cl::opt<bool> ClOptStack(
    "asan-opt-stack", cl::desc("Don't instrument scalar stack variables"),
    cl::Hidden, cl::init(false));

static cl::opt<bool> ClCheckLifetime(
    "asan-check-lifetime",
    cl::desc("Use llvm.lifetime intrinsics to insert extra checks"), cl::Hidden,
    cl::init(false));

static cl::opt<bool> ClDynamicAllocaStack(
    "asan-stack-dynamic-alloca",
    cl::desc("Use dynamic alloca to represent stack variables"), cl::Hidden,
    cl::init(true));

static cl::opt<uint32_t> ClForceExperiment(
    "asan-force-experiment",
    cl::desc("Force optimization experiment (for testing)"), cl::Hidden,
    cl::init(0));

// Debug flags.
static cl::opt<int> ClDebug("asan-debug", cl::desc("debug"), cl::Hidden,
                            cl::init(0));
static cl::opt<int> ClDebugStack("asan-debug-stack", cl::desc("debug stack"),
                                 cl::Hidden, cl::init(0));
static cl::opt<std::string> ClDebugFunc("asan-debug-func", cl::Hidden,
                                        cl::desc("Debug func"));
static cl::opt<int> ClDebugMin("asan-debug-min", cl::desc("Debug min inst"),
                               cl::Hidden, cl::init(-1));
static cl::opt<int> ClDebugMax("asan-debug-max", cl::desc("Debug man inst"),
                               cl::Hidden, cl::init(-1));

STATISTIC(NumInstrumentedReads, "Number of instrumented reads");
STATISTIC(NumInstrumentedWrites, "Number of instrumented writes");
STATISTIC(NumOptimizedAccessesToGlobalVar,
          "Number of optimized accesses to global vars");
STATISTIC(NumOptimizedAccessesToStackVar,
          "Number of optimized accesses to stack vars");

namespace {
/// Frontend-provided metadata for source location.
struct LocationMetadata {
  StringRef Filename;
  int LineNo;
  int ColumnNo;

  LocationMetadata() : Filename(), LineNo(0), ColumnNo(0) {}

  bool empty() const { return Filename.empty(); }

  void parse(MDNode *MDN) {
    assert(MDN->getNumOperands() == 3);
    MDString *DIFilename = cast<MDString>(MDN->getOperand(0));
    Filename = DIFilename->getString();
    LineNo =
        mdconst::extract<ConstantInt>(MDN->getOperand(1))->getLimitedValue();
    ColumnNo =
        mdconst::extract<ConstantInt>(MDN->getOperand(2))->getLimitedValue();
  }
};

/// Frontend-provided metadata for global variables.
class GlobalsMetadata {
 public:
  struct Entry {
    Entry() : SourceLoc(), Name(), IsDynInit(false), IsBlacklisted(false) {}
    LocationMetadata SourceLoc;
    StringRef Name;
    bool IsDynInit;
    bool IsBlacklisted;
  };

  GlobalsMetadata() : inited_(false) {}

  void init(Module &M) {
    assert(!inited_);
    inited_ = true;
    NamedMDNode *Globals = M.getNamedMetadata("llvm.asan.globals");
    if (!Globals) return;
    for (auto MDN : Globals->operands()) {
      // Metadata node contains the global and the fields of "Entry".
      assert(MDN->getNumOperands() == 5);
      auto *GV = mdconst::extract_or_null<GlobalVariable>(MDN->getOperand(0));
      // The optimizer may optimize away a global entirely.
      if (!GV) continue;
      // We can already have an entry for GV if it was merged with another
      // global.
      Entry &E = Entries[GV];
      if (auto *Loc = cast_or_null<MDNode>(MDN->getOperand(1)))
        E.SourceLoc.parse(Loc);
      if (auto *Name = cast_or_null<MDString>(MDN->getOperand(2)))
        E.Name = Name->getString();
      ConstantInt *IsDynInit =
          mdconst::extract<ConstantInt>(MDN->getOperand(3));
      E.IsDynInit |= IsDynInit->isOne();
      ConstantInt *IsBlacklisted =
          mdconst::extract<ConstantInt>(MDN->getOperand(4));
      E.IsBlacklisted |= IsBlacklisted->isOne();
    }
  }

  /// Returns metadata entry for a given global.
  Entry get(GlobalVariable *G) const {
    auto Pos = Entries.find(G);
    return (Pos != Entries.end()) ? Pos->second : Entry();
  }

 private:
  bool inited_;
  DenseMap<GlobalVariable *, Entry> Entries;
};

/// This struct defines the shadow mapping using the rule:
///   shadow = (mem >> Scale) ADD-or-OR Offset.
struct ShadowMapping {
  int Scale;
  uint64_t Offset;
  bool OrShadowOffset;
};

static ShadowMapping getShadowMapping(Triple &TargetTriple, int LongSize,
                                      bool IsKasan) {
  bool IsAndroid = TargetTriple.getEnvironment() == llvm::Triple::Android;
  bool IsIOS = TargetTriple.isiOS();
  bool IsFreeBSD = TargetTriple.isOSFreeBSD();
  bool IsLinux = TargetTriple.isOSLinux();
  bool IsPPC64 = TargetTriple.getArch() == llvm::Triple::ppc64 ||
                 TargetTriple.getArch() == llvm::Triple::ppc64le;
  bool IsX86_64 = TargetTriple.getArch() == llvm::Triple::x86_64;
  bool IsMIPS32 = TargetTriple.getArch() == llvm::Triple::mips ||
                  TargetTriple.getArch() == llvm::Triple::mipsel;
  bool IsMIPS64 = TargetTriple.getArch() == llvm::Triple::mips64 ||
                  TargetTriple.getArch() == llvm::Triple::mips64el;
  bool IsAArch64 = TargetTriple.getArch() == llvm::Triple::aarch64;
  bool IsWindows = TargetTriple.isOSWindows();

  ShadowMapping Mapping;

  if (LongSize == 32) {
    if (IsAndroid)
      Mapping.Offset = 0;
    else if (IsMIPS32)
      Mapping.Offset = kMIPS32_ShadowOffset32;
    else if (IsFreeBSD)
      Mapping.Offset = kFreeBSD_ShadowOffset32;
    else if (IsIOS)
      Mapping.Offset = kIOSShadowOffset32;
    else if (IsWindows)
      Mapping.Offset = kWindowsShadowOffset32;
    else
      Mapping.Offset = kDefaultShadowOffset32;
  } else {  // LongSize == 64
    if (IsPPC64)
      Mapping.Offset = kPPC64_ShadowOffset64;
    else if (IsFreeBSD)
      Mapping.Offset = kFreeBSD_ShadowOffset64;
    else if (IsLinux && IsX86_64) {
      if (IsKasan)
        Mapping.Offset = kLinuxKasan_ShadowOffset64;
      else
        Mapping.Offset = kSmallX86_64ShadowOffset;
    } else if (IsMIPS64)
      Mapping.Offset = kMIPS64_ShadowOffset64;
    else if (IsAArch64)
      Mapping.Offset = kAArch64_ShadowOffset64;
    else
      Mapping.Offset = kDefaultShadowOffset64;
  }

  Mapping.Scale = kDefaultShadowScale;
  if (ClMappingScale) {
    Mapping.Scale = ClMappingScale;
  }

  // OR-ing shadow offset if more efficient (at least on x86) if the offset
  // is a power of two, but on ppc64 we have to use add since the shadow
  // offset is not necessary 1/8-th of the address space.
  Mapping.OrShadowOffset = !IsPPC64 && !(Mapping.Offset & (Mapping.Offset - 1));

  return Mapping;
}

static size_t RedzoneSizeForScale(int MappingScale) {
  // Redzone used for stack and globals is at least 32 bytes.
  // For scales 6 and 7, the redzone has to be 64 and 128 bytes respectively.
  return std::max(32U, 1U << MappingScale);
}

/// AddressSanitizer: instrument the code in module to find memory bugs.
struct AddressSanitizer : public FunctionPass {
  explicit AddressSanitizer(bool CompileKernel = false)
      : FunctionPass(ID), CompileKernel(CompileKernel || ClEnableKasan) {
    initializeAddressSanitizerPass(*PassRegistry::getPassRegistry());
  }
  StringRef getPassName() const override {
    return "AddressSanitizerFunctionPass";
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
  }
  uint64_t getAllocaSizeInBytes(AllocaInst *AI) const {
    Type *Ty = AI->getAllocatedType();
    uint64_t SizeInBytes =
        AI->getModule()->getDataLayout().getTypeAllocSize(Ty);
    return SizeInBytes;
  }
  /// Check if we want (and can) handle this alloca.
  bool isInterestingAlloca(AllocaInst &AI);

  // Check if we have dynamic alloca.
  bool isDynamicAlloca(AllocaInst &AI) const {
    return AI.isArrayAllocation() || !AI.isStaticAlloca();
  }

  /// If it is an interesting memory access, return the PointerOperand
  /// and set IsWrite/Alignment. Otherwise return nullptr.
  Value *isInterestingMemoryAccess(Instruction *I, bool *IsWrite,
                                   uint64_t *TypeSize, unsigned *Alignment);
  void instrumentMop(ObjectSizeOffsetVisitor &ObjSizeVis, Instruction *I,
                     bool UseCalls, const DataLayout &DL);
  void instrumentPointerComparisonOrSubtraction(Instruction *I);
  void instrumentAddress(Instruction *OrigIns, Instruction *InsertBefore,
                         Value *Addr, uint32_t TypeSize, bool IsWrite,
                         Value *SizeArgument, bool UseCalls, uint32_t Exp);
  void instrumentUnusualSizeOrAlignment(Instruction *I, Value *Addr,
                                        uint32_t TypeSize, bool IsWrite,
                                        Value *SizeArgument, bool UseCalls,
                                        uint32_t Exp);
  Value *createSlowPathCmp(IRBuilder<> &IRB, Value *AddrLong,
                           Value *ShadowValue, uint32_t TypeSize);
  Instruction *generateCrashCode(Instruction *InsertBefore, Value *Addr,
                                 bool IsWrite, size_t AccessSizeIndex,
                                 Value *SizeArgument, uint32_t Exp);
  void instrumentMemIntrinsic(MemIntrinsic *MI);
  Value *memToShadow(Value *Shadow, IRBuilder<> &IRB);
  bool runOnFunction(Function &F) override;
  bool maybeInsertAsanInitAtFunctionEntry(Function &F);
  bool doInitialization(Module &M) override;
  static char ID;  // Pass identification, replacement for typeid

  DominatorTree &getDominatorTree() const { return *DT; }

 private:
  void initializeCallbacks(Module &M);

  bool LooksLikeCodeInBug11395(Instruction *I);
  bool GlobalIsLinkerInitialized(GlobalVariable *G);
  bool isSafeAccess(ObjectSizeOffsetVisitor &ObjSizeVis, Value *Addr,
                    uint64_t TypeSize) const;

  LLVMContext *C;
  Triple TargetTriple;
  int LongSize;
  bool CompileKernel;
  Type *IntptrTy;
  ShadowMapping Mapping;
  DominatorTree *DT;
  Function *AsanCtorFunction = nullptr;
  Function *AsanInitFunction = nullptr;
  Function *AsanHandleNoReturnFunc;
  Function *AsanPtrCmpFunction, *AsanPtrSubFunction;
  // This array is indexed by AccessIsWrite, Experiment and log2(AccessSize).
  Function *AsanErrorCallback[2][2][kNumberOfAccessSizes];
  Function *AsanMemoryAccessCallback[2][2][kNumberOfAccessSizes];
  // This array is indexed by AccessIsWrite and Experiment.
  Function *AsanErrorCallbackSized[2][2];
  Function *AsanMemoryAccessCallbackSized[2][2];
  Function *AsanMemmove, *AsanMemcpy, *AsanMemset;
  InlineAsm *EmptyAsm;
  GlobalsMetadata GlobalsMD;
  DenseMap<AllocaInst *, bool> ProcessedAllocas;

  friend struct FunctionStackPoisoner;
};

class AddressSanitizerModule : public ModulePass {
 public:
  explicit AddressSanitizerModule(bool CompileKernel = false)
      : ModulePass(ID), CompileKernel(CompileKernel || ClEnableKasan) {}
  bool runOnModule(Module &M) override;
  static char ID;  // Pass identification, replacement for typeid
  StringRef getPassName() const override { return "AddressSanitizerModule"; }

 private:
  void initializeCallbacks(Module &M);

  bool InstrumentGlobals(IRBuilder<> &IRB, Module &M);
  bool ShouldInstrumentGlobal(GlobalVariable *G);
  void poisonOneInitializer(Function &GlobalInit, GlobalValue *ModuleName);
  void createInitializerPoisonCalls(Module &M, GlobalValue *ModuleName);
  size_t MinRedzoneSizeForGlobal() const {
    return RedzoneSizeForScale(Mapping.Scale);
  }

  GlobalsMetadata GlobalsMD;
  bool CompileKernel;
  Type *IntptrTy;
  LLVMContext *C;
  Triple TargetTriple;
  ShadowMapping Mapping;
  Function *AsanPoisonGlobals;
  Function *AsanUnpoisonGlobals;
  Function *AsanRegisterGlobals;
  Function *AsanUnregisterGlobals;
};

// Stack poisoning does not play well with exception handling.
// When an exception is thrown, we essentially bypass the code
// that unpoisones the stack. This is why the run-time library has
// to intercept __cxa_throw (as well as longjmp, etc) and unpoison the entire
// stack in the interceptor. This however does not work inside the
// actual function which catches the exception. Most likely because the
// compiler hoists the load of the shadow value somewhere too high.
// This causes asan to report a non-existing bug on 453.povray.
// It sounds like an LLVM bug.
struct FunctionStackPoisoner : public InstVisitor<FunctionStackPoisoner> {
  Function &F;
  AddressSanitizer &ASan;
  DIBuilder DIB;
  LLVMContext *C;
  Type *IntptrTy;
  Type *IntptrPtrTy;
  ShadowMapping Mapping;

  SmallVector<AllocaInst *, 16> AllocaVec;
  SmallVector<Instruction *, 8> RetVec;
  unsigned StackAlignment;

  Function *AsanStackMallocFunc[kMaxAsanStackMallocSizeClass + 1],
      *AsanStackFreeFunc[kMaxAsanStackMallocSizeClass + 1];
  Function *AsanPoisonStackMemoryFunc, *AsanUnpoisonStackMemoryFunc;
  Function *AsanAllocaPoisonFunc, *AsanAllocasUnpoisonFunc;

  // Stores a place and arguments of poisoning/unpoisoning call for alloca.
  struct AllocaPoisonCall {
    IntrinsicInst *InsBefore;
    AllocaInst *AI;
    uint64_t Size;
    bool DoPoison;
  };
  SmallVector<AllocaPoisonCall, 8> AllocaPoisonCallVec;

  SmallVector<AllocaInst *, 1> DynamicAllocaVec;
  SmallVector<IntrinsicInst *, 1> StackRestoreVec;
  AllocaInst *DynamicAllocaLayout = nullptr;

  // Maps Value to an AllocaInst from which the Value is originated.
  typedef DenseMap<Value *, AllocaInst *> AllocaForValueMapTy;
  AllocaForValueMapTy AllocaForValue;

  bool HasNonEmptyInlineAsm;
  std::unique_ptr<CallInst> EmptyInlineAsm;

  FunctionStackPoisoner(Function &F, AddressSanitizer &ASan)
      : F(F),
        ASan(ASan),
        DIB(*F.getParent(), /*AllowUnresolved*/ false),
        C(ASan.C),
        IntptrTy(ASan.IntptrTy),
        IntptrPtrTy(PointerType::get(IntptrTy, 0)),
        Mapping(ASan.Mapping),
        StackAlignment(1 << Mapping.Scale),
        HasNonEmptyInlineAsm(false),
        EmptyInlineAsm(CallInst::Create(ASan.EmptyAsm)) {}

  bool runOnFunction() {
    if (!ClStack) return false;
    // Collect alloca, ret, lifetime instructions etc.
    for (BasicBlock *BB : depth_first(&F.getEntryBlock())) visit(*BB);

    if (AllocaVec.empty() && DynamicAllocaVec.empty()) return false;

    initializeCallbacks(*F.getParent());

    poisonStack();

    if (ClDebugStack) {
      DEBUG(dbgs() << F);
    }
    return true;
  }

  // Finds all Alloca instructions and puts
  // poisoned red zones around all of them.
  // Then unpoison everything back before the function returns.
  void poisonStack();

  void createDynamicAllocasInitStorage();

  // ----------------------- Visitors.
  /// \brief Collect all Ret instructions.
  void visitReturnInst(ReturnInst &RI) { RetVec.push_back(&RI); }

  void unpoisonDynamicAllocasBeforeInst(Instruction *InstBefore,
                                        Value *SavedStack) {
    IRBuilder<> IRB(InstBefore);
    IRB.CreateCall(AsanAllocasUnpoisonFunc,
                   {IRB.CreateLoad(DynamicAllocaLayout),
                    IRB.CreatePtrToInt(SavedStack, IntptrTy)});
  }

  // Unpoison dynamic allocas redzones.
  void unpoisonDynamicAllocas() {
    for (auto &Ret : RetVec)
      unpoisonDynamicAllocasBeforeInst(Ret, DynamicAllocaLayout);

    for (auto &StackRestoreInst : StackRestoreVec)
      unpoisonDynamicAllocasBeforeInst(StackRestoreInst,
                                       StackRestoreInst->getOperand(0));
  }

  // Deploy and poison redzones around dynamic alloca call. To do this, we
  // should replace this call with another one with changed parameters and
  // replace all its uses with new address, so
  //   addr = alloca type, old_size, align
  // is replaced by
  //   new_size = (old_size + additional_size) * sizeof(type)
  //   tmp = alloca i8, new_size, max(align, 32)
  //   addr = tmp + 32 (first 32 bytes are for the left redzone).
  // Additional_size is added to make new memory allocation contain not only
  // requested memory, but also left, partial and right redzones.
  void handleDynamicAllocaCall(AllocaInst *AI);

  /// \brief Collect Alloca instructions we want (and can) handle.
  void visitAllocaInst(AllocaInst &AI) {
    if (!ASan.isInterestingAlloca(AI)) return;

    StackAlignment = std::max(StackAlignment, AI.getAlignment());
    if (ASan.isDynamicAlloca(AI))
      DynamicAllocaVec.push_back(&AI);
    else
      AllocaVec.push_back(&AI);
  }

  /// \brief Collect lifetime intrinsic calls to check for use-after-scope
  /// errors.
  void visitIntrinsicInst(IntrinsicInst &II) {
    Intrinsic::ID ID = II.getIntrinsicID();
    if (ID == Intrinsic::stackrestore) StackRestoreVec.push_back(&II);
    if (!ClCheckLifetime) return;
    if (ID != Intrinsic::lifetime_start && ID != Intrinsic::lifetime_end)
      return;
    // Found lifetime intrinsic, add ASan instrumentation if necessary.
    ConstantInt *Size = dyn_cast<ConstantInt>(II.getArgOperand(0));
    // If size argument is undefined, don't do anything.
    if (Size->isMinusOne()) return;
    // Check that size doesn't saturate uint64_t and can
    // be stored in IntptrTy.
    const uint64_t SizeValue = Size->getValue().getLimitedValue();
    if (SizeValue == ~0ULL ||
        !ConstantInt::isValueValidForType(IntptrTy, SizeValue))
      return;
    // Find alloca instruction that corresponds to llvm.lifetime argument.
    AllocaInst *AI = findAllocaForValue(II.getArgOperand(1));
    if (!AI) return;
    bool DoPoison = (ID == Intrinsic::lifetime_end);
    AllocaPoisonCall APC = {&II, AI, SizeValue, DoPoison};
    AllocaPoisonCallVec.push_back(APC);
  }

  void visitCallInst(CallInst &CI) {
    HasNonEmptyInlineAsm |=
        CI.isInlineAsm() && !CI.isIdenticalTo(EmptyInlineAsm.get());
  }

  // ---------------------- Helpers.
  void initializeCallbacks(Module &M);

  bool doesDominateAllExits(const Instruction *I) const {
    for (auto Ret : RetVec) {
      if (!ASan.getDominatorTree().dominates(I, Ret)) return false;
    }
    return true;
  }

  /// Finds alloca where the value comes from.
  AllocaInst *findAllocaForValue(Value *V);
  void poisonRedZones(ArrayRef<uint8_t> ShadowBytes, IRBuilder<> &IRB,
                      Value *ShadowBase, bool DoPoison);
  void poisonAlloca(Value *V, uint64_t Size, IRBuilder<> &IRB, bool DoPoison);

  void SetShadowToStackAfterReturnInlined(IRBuilder<> &IRB, Value *ShadowBase,
                                          int Size);
  Value *createAllocaForLayout(IRBuilder<> &IRB, const ASanStackFrameLayout &L,
                               bool Dynamic);
  PHINode *createPHI(IRBuilder<> &IRB, Value *Cond, Value *ValueIfTrue,
                     Instruction *ThenTerm, Value *ValueIfFalse);
};

}  // namespace

char AddressSanitizer::ID = 0;
INITIALIZE_PASS_BEGIN(
    AddressSanitizer, "asan",
    "AddressSanitizer: detects use-after-free and out-of-bounds bugs.", false,
    false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(
    AddressSanitizer, "asan",
    "AddressSanitizer: detects use-after-free and out-of-bounds bugs.", false,
    false)
FunctionPass *llvm::createAddressSanitizerFunctionPass(bool CompileKernel) {
  return new AddressSanitizer(CompileKernel);
}

char AddressSanitizerModule::ID = 0;
INITIALIZE_PASS(
    AddressSanitizerModule, "asan-module",
    "AddressSanitizer: detects use-after-free and out-of-bounds bugs."
    "ModulePass",
    false, false)
ModulePass *llvm::createAddressSanitizerModulePass(bool CompileKernel) {
  return new AddressSanitizerModule(CompileKernel);
}

static size_t TypeSizeToSizeIndex(uint32_t TypeSize) {
  size_t Res = countTrailingZeros(TypeSize / 8);
  assert(Res < kNumberOfAccessSizes);
  return Res;
}

// \brief Create a constant for Str so that we can pass it to the run-time lib.
static GlobalVariable *createPrivateGlobalForString(Module &M, StringRef Str,
                                                    bool AllowMerging) {
  Constant *StrConst = ConstantDataArray::getString(M.getContext(), Str);
  // We use private linkage for module-local strings. If they can be merged
  // with another one, we set the unnamed_addr attribute.
  GlobalVariable *GV =
      new GlobalVariable(M, StrConst->getType(), true,
                         GlobalValue::PrivateLinkage, StrConst, kAsanGenPrefix);
  if (AllowMerging) GV->setUnnamedAddr(true);
  GV->setAlignment(1);  // Strings may not be merged w/o setting align 1.
  return GV;
}

/// \brief Create a global describing a source location.
static GlobalVariable *createPrivateGlobalForSourceLoc(Module &M,
                                                       LocationMetadata MD) {
  Constant *LocData[] = {
      createPrivateGlobalForString(M, MD.Filename, true),
      ConstantInt::get(Type::getInt32Ty(M.getContext()), MD.LineNo),
      ConstantInt::get(Type::getInt32Ty(M.getContext()), MD.ColumnNo),
  };
  auto LocStruct = ConstantStruct::getAnon(LocData);
  auto GV = new GlobalVariable(M, LocStruct->getType(), true,
                               GlobalValue::PrivateLinkage, LocStruct,
                               kAsanGenPrefix);
  GV->setUnnamedAddr(true);
  return GV;
}

static bool GlobalWasGeneratedByAsan(GlobalVariable *G) {
  return G->getName().find(kAsanGenPrefix) == 0 ||
         G->getName().find(kSanCovGenPrefix) == 0;
}

Value *AddressSanitizer::memToShadow(Value *Shadow, IRBuilder<> &IRB) {
  // Shadow >> scale
  Shadow = IRB.CreateLShr(Shadow, Mapping.Scale);
  if (Mapping.Offset == 0) return Shadow;
  // (Shadow >> scale) | offset
  if (Mapping.OrShadowOffset)
    return IRB.CreateOr(Shadow, ConstantInt::get(IntptrTy, Mapping.Offset));
  else
    return IRB.CreateAdd(Shadow, ConstantInt::get(IntptrTy, Mapping.Offset));
}

// Instrument memset/memmove/memcpy
void AddressSanitizer::instrumentMemIntrinsic(MemIntrinsic *MI) {
  IRBuilder<> IRB(MI);
  if (isa<MemTransferInst>(MI)) {
    IRB.CreateCall(
        isa<MemMoveInst>(MI) ? AsanMemmove : AsanMemcpy,
        {IRB.CreatePointerCast(MI->getOperand(0), IRB.getInt8PtrTy()),
         IRB.CreatePointerCast(MI->getOperand(1), IRB.getInt8PtrTy()),
         IRB.CreateIntCast(MI->getOperand(2), IntptrTy, false)});
  } else if (isa<MemSetInst>(MI)) {
    IRB.CreateCall(
        AsanMemset,
        {IRB.CreatePointerCast(MI->getOperand(0), IRB.getInt8PtrTy()),
         IRB.CreateIntCast(MI->getOperand(1), IRB.getInt32Ty(), false),
         IRB.CreateIntCast(MI->getOperand(2), IntptrTy, false)});
  }
  MI->eraseFromParent();
}

/// Check if we want (and can) handle this alloca.
bool AddressSanitizer::isInterestingAlloca(AllocaInst &AI) {
  auto PreviouslySeenAllocaInfo = ProcessedAllocas.find(&AI);

  if (PreviouslySeenAllocaInfo != ProcessedAllocas.end())
    return PreviouslySeenAllocaInfo->getSecond();

  bool IsInteresting =
      (AI.getAllocatedType()->isSized() &&
       // alloca() may be called with 0 size, ignore it.
       getAllocaSizeInBytes(&AI) > 0 &&
       // We are only interested in allocas not promotable to registers.
       // Promotable allocas are common under -O0.
       (!ClSkipPromotableAllocas || !isAllocaPromotable(&AI) ||
        isDynamicAlloca(AI)));

  ProcessedAllocas[&AI] = IsInteresting;
  return IsInteresting;
}

/// If I is an interesting memory access, return the PointerOperand
/// and set IsWrite/Alignment. Otherwise return nullptr.
Value *AddressSanitizer::isInterestingMemoryAccess(Instruction *I,
                                                   bool *IsWrite,
                                                   uint64_t *TypeSize,
                                                   unsigned *Alignment) {
  // Skip memory accesses inserted by another instrumentation.
  if (I->getMetadata("nosanitize")) return nullptr;

  Value *PtrOperand = nullptr;
  const DataLayout &DL = I->getModule()->getDataLayout();
  if (LoadInst *LI = dyn_cast<LoadInst>(I)) {
    if (!ClInstrumentReads) return nullptr;
    *IsWrite = false;
    *TypeSize = DL.getTypeStoreSizeInBits(LI->getType());
    *Alignment = LI->getAlignment();
    PtrOperand = LI->getPointerOperand();
  } else if (StoreInst *SI = dyn_cast<StoreInst>(I)) {
    if (!ClInstrumentWrites) return nullptr;
    *IsWrite = true;
    *TypeSize = DL.getTypeStoreSizeInBits(SI->getValueOperand()->getType());
    *Alignment = SI->getAlignment();
    PtrOperand = SI->getPointerOperand();
  } else if (AtomicRMWInst *RMW = dyn_cast<AtomicRMWInst>(I)) {
    if (!ClInstrumentAtomics) return nullptr;
    *IsWrite = true;
    *TypeSize = DL.getTypeStoreSizeInBits(RMW->getValOperand()->getType());
    *Alignment = 0;
    PtrOperand = RMW->getPointerOperand();
  } else if (AtomicCmpXchgInst *XCHG = dyn_cast<AtomicCmpXchgInst>(I)) {
    if (!ClInstrumentAtomics) return nullptr;
    *IsWrite = true;
    *TypeSize = DL.getTypeStoreSizeInBits(XCHG->getCompareOperand()->getType());
    *Alignment = 0;
    PtrOperand = XCHG->getPointerOperand();
  }

  // Treat memory accesses to promotable allocas as non-interesting since they
  // will not cause memory violations. This greatly speeds up the instrumented
  // executable at -O0.
  if (ClSkipPromotableAllocas)
    if (auto AI = dyn_cast_or_null<AllocaInst>(PtrOperand))
      return isInterestingAlloca(*AI) ? AI : nullptr;

  return PtrOperand;
}

static bool isPointerOperand(Value *V) {
  return V->getType()->isPointerTy() || isa<PtrToIntInst>(V);
}

// This is a rough heuristic; it may cause both false positives and
// false negatives. The proper implementation requires cooperation with
// the frontend.
static bool isInterestingPointerComparisonOrSubtraction(Instruction *I) {
  if (ICmpInst *Cmp = dyn_cast<ICmpInst>(I)) {
    if (!Cmp->isRelational()) return false;
  } else if (BinaryOperator *BO = dyn_cast<BinaryOperator>(I)) {
    if (BO->getOpcode() != Instruction::Sub) return false;
  } else {
    return false;
  }
  if (!isPointerOperand(I->getOperand(0)) ||
      !isPointerOperand(I->getOperand(1)))
    return false;
  return true;
}

bool AddressSanitizer::GlobalIsLinkerInitialized(GlobalVariable *G) {
  // If a global variable does not have dynamic initialization we don't
  // have to instrument it.  However, if a global does not have initializer
  // at all, we assume it has dynamic initializer (in other TU).
  return G->hasInitializer() && !GlobalsMD.get(G).IsDynInit;
}

void AddressSanitizer::instrumentPointerComparisonOrSubtraction(
    Instruction *I) {
  IRBuilder<> IRB(I);
  Function *F = isa<ICmpInst>(I) ? AsanPtrCmpFunction : AsanPtrSubFunction;
  Value *Param[2] = {I->getOperand(0), I->getOperand(1)};
  for (int i = 0; i < 2; i++) {
    if (Param[i]->getType()->isPointerTy())
      Param[i] = IRB.CreatePointerCast(Param[i], IntptrTy);
  }
  IRB.CreateCall(F, Param);
}

void AddressSanitizer::instrumentMop(ObjectSizeOffsetVisitor &ObjSizeVis,
                                     Instruction *I, bool UseCalls,
                                     const DataLayout &DL) {
  bool IsWrite = false;
  unsigned Alignment = 0;
  uint64_t TypeSize = 0;
  Value *Addr = isInterestingMemoryAccess(I, &IsWrite, &TypeSize, &Alignment);
  assert(Addr);

  // Optimization experiments.
  // The experiments can be used to evaluate potential optimizations that remove
  // instrumentation (assess false negatives). Instead of completely removing
  // some instrumentation, you set Exp to a non-zero value (mask of optimization
  // experiments that want to remove instrumentation of this instruction).
  // If Exp is non-zero, this pass will emit special calls into runtime
  // (e.g. __asan_report_exp_load1 instead of __asan_report_load1). These calls
  // make runtime terminate the program in a special way (with a different
  // exit status). Then you run the new compiler on a buggy corpus, collect
  // the special terminations (ideally, you don't see them at all -- no false
  // negatives) and make the decision on the optimization.
  uint32_t Exp = ClForceExperiment;

  if (ClOpt && ClOptGlobals) {
    // If initialization order checking is disabled, a simple access to a
    // dynamically initialized global is always valid.
    GlobalVariable *G = dyn_cast<GlobalVariable>(GetUnderlyingObject(Addr, DL));
    if (G != NULL && (!ClInitializers || GlobalIsLinkerInitialized(G)) &&
        isSafeAccess(ObjSizeVis, Addr, TypeSize)) {
      NumOptimizedAccessesToGlobalVar++;
      return;
    }
  }

  if (ClOpt && ClOptStack) {
    // A direct inbounds access to a stack variable is always valid.
    if (isa<AllocaInst>(GetUnderlyingObject(Addr, DL)) &&
        isSafeAccess(ObjSizeVis, Addr, TypeSize)) {
      NumOptimizedAccessesToStackVar++;
      return;
    }
  }

  if (IsWrite)
    NumInstrumentedWrites++;
  else
    NumInstrumentedReads++;

  unsigned Granularity = 1 << Mapping.Scale;
  // Instrument a 1-, 2-, 4-, 8-, or 16- byte access with one check
  // if the data is properly aligned.
  if ((TypeSize == 8 || TypeSize == 16 || TypeSize == 32 || TypeSize == 64 ||
       TypeSize == 128) &&
      (Alignment >= Granularity || Alignment == 0 || Alignment >= TypeSize / 8))
    return instrumentAddress(I, I, Addr, TypeSize, IsWrite, nullptr, UseCalls,
                             Exp);
  instrumentUnusualSizeOrAlignment(I, Addr, TypeSize, IsWrite, nullptr,
                                   UseCalls, Exp);
}

Instruction *AddressSanitizer::generateCrashCode(Instruction *InsertBefore,
                                                 Value *Addr, bool IsWrite,
                                                 size_t AccessSizeIndex,
                                                 Value *SizeArgument,
                                                 uint32_t Exp) {
  IRBuilder<> IRB(InsertBefore);
  Value *ExpVal = Exp == 0 ? nullptr : ConstantInt::get(IRB.getInt32Ty(), Exp);
  CallInst *Call = nullptr;
  if (SizeArgument) {
    if (Exp == 0)
      Call = IRB.CreateCall(AsanErrorCallbackSized[IsWrite][0],
                            {Addr, SizeArgument});
    else
      Call = IRB.CreateCall(AsanErrorCallbackSized[IsWrite][1],
                            {Addr, SizeArgument, ExpVal});
  } else {
    if (Exp == 0)
      Call =
          IRB.CreateCall(AsanErrorCallback[IsWrite][0][AccessSizeIndex], Addr);
    else
      Call = IRB.CreateCall(AsanErrorCallback[IsWrite][1][AccessSizeIndex],
                            {Addr, ExpVal});
  }

  // We don't do Call->setDoesNotReturn() because the BB already has
  // UnreachableInst at the end.
  // This EmptyAsm is required to avoid callback merge.
  IRB.CreateCall(EmptyAsm, {});
  return Call;
}

Value *AddressSanitizer::createSlowPathCmp(IRBuilder<> &IRB, Value *AddrLong,
                                           Value *ShadowValue,
                                           uint32_t TypeSize) {
  size_t Granularity = 1 << Mapping.Scale;
  // Addr & (Granularity - 1)
  Value *LastAccessedByte =
      IRB.CreateAnd(AddrLong, ConstantInt::get(IntptrTy, Granularity - 1));
  // (Addr & (Granularity - 1)) + size - 1
  if (TypeSize / 8 > 1)
    LastAccessedByte = IRB.CreateAdd(
        LastAccessedByte, ConstantInt::get(IntptrTy, TypeSize / 8 - 1));
  // (uint8_t) ((Addr & (Granularity-1)) + size - 1)
  LastAccessedByte =
      IRB.CreateIntCast(LastAccessedByte, ShadowValue->getType(), false);
  // ((uint8_t) ((Addr & (Granularity-1)) + size - 1)) >= ShadowValue
  return IRB.CreateICmpSGE(LastAccessedByte, ShadowValue);
}

void AddressSanitizer::instrumentAddress(Instruction *OrigIns,
                                         Instruction *InsertBefore, Value *Addr,
                                         uint32_t TypeSize, bool IsWrite,
                                         Value *SizeArgument, bool UseCalls,
                                         uint32_t Exp) {
  IRBuilder<> IRB(InsertBefore);
  Value *AddrLong = IRB.CreatePointerCast(Addr, IntptrTy);
  size_t AccessSizeIndex = TypeSizeToSizeIndex(TypeSize);

  if (UseCalls) {
    if (Exp == 0)
      IRB.CreateCall(AsanMemoryAccessCallback[IsWrite][0][AccessSizeIndex],
                     AddrLong);
    else
      IRB.CreateCall(AsanMemoryAccessCallback[IsWrite][1][AccessSizeIndex],
                     {AddrLong, ConstantInt::get(IRB.getInt32Ty(), Exp)});
    return;
  }

  Type *ShadowTy =
      IntegerType::get(*C, std::max(8U, TypeSize >> Mapping.Scale));
  Type *ShadowPtrTy = PointerType::get(ShadowTy, 0);
  Value *ShadowPtr = memToShadow(AddrLong, IRB);
  Value *CmpVal = Constant::getNullValue(ShadowTy);
  Value *ShadowValue =
      IRB.CreateLoad(IRB.CreateIntToPtr(ShadowPtr, ShadowPtrTy));

  Value *Cmp = IRB.CreateICmpNE(ShadowValue, CmpVal);
  size_t Granularity = 1 << Mapping.Scale;
  TerminatorInst *CrashTerm = nullptr;

  if (ClAlwaysSlowPath || (TypeSize < 8 * Granularity)) {
    // We use branch weights for the slow path check, to indicate that the slow
    // path is rarely taken. This seems to be the case for SPEC benchmarks.
    TerminatorInst *CheckTerm = SplitBlockAndInsertIfThen(
        Cmp, InsertBefore, false, MDBuilder(*C).createBranchWeights(1, 100000));
    assert(cast<BranchInst>(CheckTerm)->isUnconditional());
    BasicBlock *NextBB = CheckTerm->getSuccessor(0);
    IRB.SetInsertPoint(CheckTerm);
    Value *Cmp2 = createSlowPathCmp(IRB, AddrLong, ShadowValue, TypeSize);
    BasicBlock *CrashBlock =
        BasicBlock::Create(*C, "", NextBB->getParent(), NextBB);
    CrashTerm = new UnreachableInst(*C, CrashBlock);
    BranchInst *NewTerm = BranchInst::Create(CrashBlock, NextBB, Cmp2);
    ReplaceInstWithInst(CheckTerm, NewTerm);
  } else {
    CrashTerm = SplitBlockAndInsertIfThen(Cmp, InsertBefore, true);
  }

  Instruction *Crash = generateCrashCode(CrashTerm, AddrLong, IsWrite,
                                         AccessSizeIndex, SizeArgument, Exp);
  Crash->setDebugLoc(OrigIns->getDebugLoc());
}

// Instrument unusual size or unusual alignment.
// We can not do it with a single check, so we do 1-byte check for the first
// and the last bytes. We call __asan_report_*_n(addr, real_size) to be able
// to report the actual access size.
void AddressSanitizer::instrumentUnusualSizeOrAlignment(
    Instruction *I, Value *Addr, uint32_t TypeSize, bool IsWrite,
    Value *SizeArgument, bool UseCalls, uint32_t Exp) {
  IRBuilder<> IRB(I);
  Value *Size = ConstantInt::get(IntptrTy, TypeSize / 8);
  Value *AddrLong = IRB.CreatePointerCast(Addr, IntptrTy);
  if (UseCalls) {
    if (Exp == 0)
      IRB.CreateCall(AsanMemoryAccessCallbackSized[IsWrite][0],
                     {AddrLong, Size});
    else
      IRB.CreateCall(AsanMemoryAccessCallbackSized[IsWrite][1],
                     {AddrLong, Size, ConstantInt::get(IRB.getInt32Ty(), Exp)});
  } else {
    Value *LastByte = IRB.CreateIntToPtr(
        IRB.CreateAdd(AddrLong, ConstantInt::get(IntptrTy, TypeSize / 8 - 1)),
        Addr->getType());
    instrumentAddress(I, I, Addr, 8, IsWrite, Size, false, Exp);
    instrumentAddress(I, I, LastByte, 8, IsWrite, Size, false, Exp);
  }
}

void AddressSanitizerModule::poisonOneInitializer(Function &GlobalInit,
                                                  GlobalValue *ModuleName) {
  // Set up the arguments to our poison/unpoison functions.
  IRBuilder<> IRB(GlobalInit.begin()->getFirstInsertionPt());

  // Add a call to poison all external globals before the given function starts.
  Value *ModuleNameAddr = ConstantExpr::getPointerCast(ModuleName, IntptrTy);
  IRB.CreateCall(AsanPoisonGlobals, ModuleNameAddr);

  // Add calls to unpoison all globals before each return instruction.
  for (auto &BB : GlobalInit.getBasicBlockList())
    if (ReturnInst *RI = dyn_cast<ReturnInst>(BB.getTerminator()))
      CallInst::Create(AsanUnpoisonGlobals, "", RI);
}

void AddressSanitizerModule::createInitializerPoisonCalls(
    Module &M, GlobalValue *ModuleName) {
  GlobalVariable *GV = M.getGlobalVariable("llvm.global_ctors");

  ConstantArray *CA = cast<ConstantArray>(GV->getInitializer());
  for (Use &OP : CA->operands()) {
    if (isa<ConstantAggregateZero>(OP)) continue;
    ConstantStruct *CS = cast<ConstantStruct>(OP);

    // Must have a function or null ptr.
    if (Function *F = dyn_cast<Function>(CS->getOperand(1))) {
      if (F->getName() == kAsanModuleCtorName) continue;
      ConstantInt *Priority = dyn_cast<ConstantInt>(CS->getOperand(0));
      // Don't instrument CTORs that will run before asan.module_ctor.
      if (Priority->getLimitedValue() <= kAsanCtorAndDtorPriority) continue;
      poisonOneInitializer(*F, ModuleName);
    }
  }
}

bool AddressSanitizerModule::ShouldInstrumentGlobal(GlobalVariable *G) {
  Type *Ty = cast<PointerType>(G->getType())->getElementType();
  DEBUG(dbgs() << "GLOBAL: " << *G << "\n");

  if (GlobalsMD.get(G).IsBlacklisted) return false;
  if (!Ty->isSized()) return false;
  if (!G->hasInitializer()) return false;
  if (GlobalWasGeneratedByAsan(G)) return false;  // Our own global.
  // Touch only those globals that will not be defined in other modules.
  // Don't handle ODR linkage types and COMDATs since other modules may be built
  // without ASan.
  if (G->getLinkage() != GlobalVariable::ExternalLinkage &&
      G->getLinkage() != GlobalVariable::PrivateLinkage &&
      G->getLinkage() != GlobalVariable::InternalLinkage)
    return false;
  if (G->hasComdat()) return false;
  // Two problems with thread-locals:
  //   - The address of the main thread's copy can't be computed at link-time.
  //   - Need to poison all copies, not just the main thread's one.
  if (G->isThreadLocal()) return false;
  // For now, just ignore this Global if the alignment is large.
  if (G->getAlignment() > MinRedzoneSizeForGlobal()) return false;

  if (G->hasSection()) {
    StringRef Section(G->getSection());

    // Globals from llvm.metadata aren't emitted, do not instrument them.
    if (Section == "llvm.metadata") return false;
    // Do not instrument globals from special LLVM sections.
    if (Section.find("__llvm") != StringRef::npos) return false;

    // Callbacks put into the CRT initializer/terminator sections
    // should not be instrumented.
    // See https://code.google.com/p/address-sanitizer/issues/detail?id=305
    // and http://msdn.microsoft.com/en-US/en-en/library/bb918180(v=vs.120).aspx
    if (Section.startswith(".CRT")) {
      DEBUG(dbgs() << "Ignoring a global initializer callback: " << *G << "\n");
      return false;
    }

    if (TargetTriple.isOSBinFormatMachO()) {
      StringRef ParsedSegment, ParsedSection;
      unsigned TAA = 0, StubSize = 0;
      bool TAAParsed;
      std::string ErrorCode = MCSectionMachO::ParseSectionSpecifier(
          Section, ParsedSegment, ParsedSection, TAA, TAAParsed, StubSize);
      if (!ErrorCode.empty()) {
        assert(false && "Invalid section specifier.");
        return false;
      }

      // Ignore the globals from the __OBJC section. The ObjC runtime assumes
      // those conform to /usr/lib/objc/runtime.h, so we can't add redzones to
      // them.
      if (ParsedSegment == "__OBJC" ||
          (ParsedSegment == "__DATA" && ParsedSection.startswith("__objc_"))) {
        DEBUG(dbgs() << "Ignoring ObjC runtime global: " << *G << "\n");
        return false;
      }
      // See http://code.google.com/p/address-sanitizer/issues/detail?id=32
      // Constant CFString instances are compiled in the following way:
      //  -- the string buffer is emitted into
      //     __TEXT,__cstring,cstring_literals
      //  -- the constant NSConstantString structure referencing that buffer
      //     is placed into __DATA,__cfstring
      // Therefore there's no point in placing redzones into __DATA,__cfstring.
      // Moreover, it causes the linker to crash on OS X 10.7
      if (ParsedSegment == "__DATA" && ParsedSection == "__cfstring") {
        DEBUG(dbgs() << "Ignoring CFString: " << *G << "\n");
        return false;
      }
      // The linker merges the contents of cstring_literals and removes the
      // trailing zeroes.
      if (ParsedSegment == "__TEXT" && (TAA & MachO::S_CSTRING_LITERALS)) {
        DEBUG(dbgs() << "Ignoring a cstring literal: " << *G << "\n");
        return false;
      }
    }
  }

  return true;
}

void AddressSanitizerModule::initializeCallbacks(Module &M) {
  IRBuilder<> IRB(*C);
  // Declare our poisoning and unpoisoning functions.
  AsanPoisonGlobals = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      kAsanPoisonGlobalsName, IRB.getVoidTy(), IntptrTy, nullptr));
  AsanPoisonGlobals->setLinkage(Function::ExternalLinkage);
  AsanUnpoisonGlobals = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      kAsanUnpoisonGlobalsName, IRB.getVoidTy(), nullptr));
  AsanUnpoisonGlobals->setLinkage(Function::ExternalLinkage);
  // Declare functions that register/unregister globals.
  AsanRegisterGlobals = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      kAsanRegisterGlobalsName, IRB.getVoidTy(), IntptrTy, IntptrTy, nullptr));
  AsanRegisterGlobals->setLinkage(Function::ExternalLinkage);
  AsanUnregisterGlobals = checkSanitizerInterfaceFunction(
      M.getOrInsertFunction(kAsanUnregisterGlobalsName, IRB.getVoidTy(),
                            IntptrTy, IntptrTy, nullptr));
  AsanUnregisterGlobals->setLinkage(Function::ExternalLinkage);
}

// This function replaces all global variables with new variables that have
// trailing redzones. It also creates a function that poisons
// redzones and inserts this function into llvm.global_ctors.
bool AddressSanitizerModule::InstrumentGlobals(IRBuilder<> &IRB, Module &M) {
  GlobalsMD.init(M);

  SmallVector<GlobalVariable *, 16> GlobalsToChange;

  for (auto &G : M.globals()) {
    if (ShouldInstrumentGlobal(&G)) GlobalsToChange.push_back(&G);
  }

  size_t n = GlobalsToChange.size();
  if (n == 0) return false;

  // A global is described by a structure
  //   size_t beg;
  //   size_t size;
  //   size_t size_with_redzone;
  //   const char *name;
  //   const char *module_name;
  //   size_t has_dynamic_init;
  //   void *source_location;
  // We initialize an array of such structures and pass it to a run-time call.
  StructType *GlobalStructTy =
      StructType::get(IntptrTy, IntptrTy, IntptrTy, IntptrTy, IntptrTy,
                      IntptrTy, IntptrTy, nullptr);
  SmallVector<Constant *, 16> Initializers(n);

  bool HasDynamicallyInitializedGlobals = false;

  // We shouldn't merge same module names, as this string serves as unique
  // module ID in runtime.
  GlobalVariable *ModuleName = createPrivateGlobalForString(
      M, M.getModuleIdentifier(), /*AllowMerging*/ false);

  auto &DL = M.getDataLayout();
  for (size_t i = 0; i < n; i++) {
    static const uint64_t kMaxGlobalRedzone = 1 << 18;
    GlobalVariable *G = GlobalsToChange[i];

    auto MD = GlobalsMD.get(G);
    // Create string holding the global name (use global name from metadata
    // if it's available, otherwise just write the name of global variable).
    GlobalVariable *Name = createPrivateGlobalForString(
        M, MD.Name.empty() ? G->getName() : MD.Name,
        /*AllowMerging*/ true);

    PointerType *PtrTy = cast<PointerType>(G->getType());
    Type *Ty = PtrTy->getElementType();
    uint64_t SizeInBytes = DL.getTypeAllocSize(Ty);
    uint64_t MinRZ = MinRedzoneSizeForGlobal();
    // MinRZ <= RZ <= kMaxGlobalRedzone
    // and trying to make RZ to be ~ 1/4 of SizeInBytes.
    uint64_t RZ = std::max(
        MinRZ, std::min(kMaxGlobalRedzone, (SizeInBytes / MinRZ / 4) * MinRZ));
    uint64_t RightRedzoneSize = RZ;
    // Round up to MinRZ
    if (SizeInBytes % MinRZ) RightRedzoneSize += MinRZ - (SizeInBytes % MinRZ);
    assert(((RightRedzoneSize + SizeInBytes) % MinRZ) == 0);
    Type *RightRedZoneTy = ArrayType::get(IRB.getInt8Ty(), RightRedzoneSize);

    StructType *NewTy = StructType::get(Ty, RightRedZoneTy, nullptr);
    Constant *NewInitializer =
        ConstantStruct::get(NewTy, G->getInitializer(),
                            Constant::getNullValue(RightRedZoneTy), nullptr);

    // Create a new global variable with enough space for a redzone.
    GlobalValue::LinkageTypes Linkage = G->getLinkage();
    if (G->isConstant() && Linkage == GlobalValue::PrivateLinkage)
      Linkage = GlobalValue::InternalLinkage;
    GlobalVariable *NewGlobal =
        new GlobalVariable(M, NewTy, G->isConstant(), Linkage, NewInitializer,
                           "", G, G->getThreadLocalMode());
    NewGlobal->copyAttributesFrom(G);
    NewGlobal->setAlignment(MinRZ);

    Value *Indices2[2];
    Indices2[0] = IRB.getInt32(0);
    Indices2[1] = IRB.getInt32(0);

    G->replaceAllUsesWith(
        ConstantExpr::getGetElementPtr(NewTy, NewGlobal, Indices2, true));
    NewGlobal->takeName(G);
    G->eraseFromParent();

    Constant *SourceLoc;
    if (!MD.SourceLoc.empty()) {
      auto SourceLocGlobal = createPrivateGlobalForSourceLoc(M, MD.SourceLoc);
      SourceLoc = ConstantExpr::getPointerCast(SourceLocGlobal, IntptrTy);
    } else {
      SourceLoc = ConstantInt::get(IntptrTy, 0);
    }

    Initializers[i] = ConstantStruct::get(
        GlobalStructTy, ConstantExpr::getPointerCast(NewGlobal, IntptrTy),
        ConstantInt::get(IntptrTy, SizeInBytes),
        ConstantInt::get(IntptrTy, SizeInBytes + RightRedzoneSize),
        ConstantExpr::getPointerCast(Name, IntptrTy),
        ConstantExpr::getPointerCast(ModuleName, IntptrTy),
        ConstantInt::get(IntptrTy, MD.IsDynInit), SourceLoc, nullptr);

    if (ClInitializers && MD.IsDynInit) HasDynamicallyInitializedGlobals = true;

    DEBUG(dbgs() << "NEW GLOBAL: " << *NewGlobal << "\n");
  }

  ArrayType *ArrayOfGlobalStructTy = ArrayType::get(GlobalStructTy, n);
  GlobalVariable *AllGlobals = new GlobalVariable(
      M, ArrayOfGlobalStructTy, false, GlobalVariable::InternalLinkage,
      ConstantArray::get(ArrayOfGlobalStructTy, Initializers), "");

  // Create calls for poisoning before initializers run and unpoisoning after.
  if (HasDynamicallyInitializedGlobals)
    createInitializerPoisonCalls(M, ModuleName);
  IRB.CreateCall(AsanRegisterGlobals,
                 {IRB.CreatePointerCast(AllGlobals, IntptrTy),
                  ConstantInt::get(IntptrTy, n)});

  // We also need to unregister globals at the end, e.g. when a shared library
  // gets closed.
  Function *AsanDtorFunction =
      Function::Create(FunctionType::get(Type::getVoidTy(*C), false),
                       GlobalValue::InternalLinkage, kAsanModuleDtorName, &M);
  BasicBlock *AsanDtorBB = BasicBlock::Create(*C, "", AsanDtorFunction);
  IRBuilder<> IRB_Dtor(ReturnInst::Create(*C, AsanDtorBB));
  IRB_Dtor.CreateCall(AsanUnregisterGlobals,
                      {IRB.CreatePointerCast(AllGlobals, IntptrTy),
                       ConstantInt::get(IntptrTy, n)});
  appendToGlobalDtors(M, AsanDtorFunction, kAsanCtorAndDtorPriority);

  DEBUG(dbgs() << M);
  return true;
}

bool AddressSanitizerModule::runOnModule(Module &M) {
  C = &(M.getContext());
  int LongSize = M.getDataLayout().getPointerSizeInBits();
  IntptrTy = Type::getIntNTy(*C, LongSize);
  TargetTriple = Triple(M.getTargetTriple());
  Mapping = getShadowMapping(TargetTriple, LongSize, CompileKernel);
  initializeCallbacks(M);

  bool Changed = false;

  // TODO(glider): temporarily disabled globals instrumentation for KASan.
  if (ClGlobals && !CompileKernel) {
    Function *CtorFunc = M.getFunction(kAsanModuleCtorName);
    assert(CtorFunc);
    IRBuilder<> IRB(CtorFunc->getEntryBlock().getTerminator());
    Changed |= InstrumentGlobals(IRB, M);
  }

  return Changed;
}

void AddressSanitizer::initializeCallbacks(Module &M) {
  IRBuilder<> IRB(*C);
  // Create __asan_report* callbacks.
  // IsWrite, TypeSize and Exp are encoded in the function name.
  for (int Exp = 0; Exp < 2; Exp++) {
    for (size_t AccessIsWrite = 0; AccessIsWrite <= 1; AccessIsWrite++) {
      const std::string TypeStr = AccessIsWrite ? "store" : "load";
      const std::string ExpStr = Exp ? "exp_" : "";
      const std::string SuffixStr = CompileKernel ? "N" : "_n";
      const std::string EndingStr = CompileKernel ? "_noabort" : "";
      const Type *ExpType = Exp ? Type::getInt32Ty(*C) : nullptr;
      // TODO(glider): for KASan builds add _noabort to error reporting
      // functions and make them actually noabort (remove the UnreachableInst).
      AsanErrorCallbackSized[AccessIsWrite][Exp] =
          checkSanitizerInterfaceFunction(M.getOrInsertFunction(
              kAsanReportErrorTemplate + ExpStr + TypeStr + SuffixStr,
              IRB.getVoidTy(), IntptrTy, IntptrTy, ExpType, nullptr));
      AsanMemoryAccessCallbackSized[AccessIsWrite][Exp] =
          checkSanitizerInterfaceFunction(M.getOrInsertFunction(
              ClMemoryAccessCallbackPrefix + ExpStr + TypeStr + "N" + EndingStr,
              IRB.getVoidTy(), IntptrTy, IntptrTy, ExpType, nullptr));
      for (size_t AccessSizeIndex = 0; AccessSizeIndex < kNumberOfAccessSizes;
           AccessSizeIndex++) {
        const std::string Suffix = TypeStr + itostr((int64_t)(1) << AccessSizeIndex); // HLSL Change - keep as 64-bit operation
        AsanErrorCallback[AccessIsWrite][Exp][AccessSizeIndex] =
            checkSanitizerInterfaceFunction(M.getOrInsertFunction(
                kAsanReportErrorTemplate + ExpStr + Suffix,
                IRB.getVoidTy(), IntptrTy, ExpType, nullptr));
        AsanMemoryAccessCallback[AccessIsWrite][Exp][AccessSizeIndex] =
            checkSanitizerInterfaceFunction(M.getOrInsertFunction(
                ClMemoryAccessCallbackPrefix + ExpStr + Suffix + EndingStr,
                IRB.getVoidTy(), IntptrTy, ExpType, nullptr));
      }
    }
  }

  const std::string MemIntrinCallbackPrefix =
      CompileKernel ? std::string("") : ClMemoryAccessCallbackPrefix;
  AsanMemmove = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      MemIntrinCallbackPrefix + "memmove", IRB.getInt8PtrTy(),
      IRB.getInt8PtrTy(), IRB.getInt8PtrTy(), IntptrTy, nullptr));
  AsanMemcpy = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      MemIntrinCallbackPrefix + "memcpy", IRB.getInt8PtrTy(),
      IRB.getInt8PtrTy(), IRB.getInt8PtrTy(), IntptrTy, nullptr));
  AsanMemset = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      MemIntrinCallbackPrefix + "memset", IRB.getInt8PtrTy(),
      IRB.getInt8PtrTy(), IRB.getInt32Ty(), IntptrTy, nullptr));

  AsanHandleNoReturnFunc = checkSanitizerInterfaceFunction(
      M.getOrInsertFunction(kAsanHandleNoReturnName, IRB.getVoidTy(), nullptr));

  AsanPtrCmpFunction = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      kAsanPtrCmp, IRB.getVoidTy(), IntptrTy, IntptrTy, nullptr));
  AsanPtrSubFunction = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      kAsanPtrSub, IRB.getVoidTy(), IntptrTy, IntptrTy, nullptr));
  // We insert an empty inline asm after __asan_report* to avoid callback merge.
  EmptyAsm = InlineAsm::get(FunctionType::get(IRB.getVoidTy(), false),
                            StringRef(""), StringRef(""),
                            /*hasSideEffects=*/true);
}

// virtual
bool AddressSanitizer::doInitialization(Module &M) {
  // Initialize the private fields. No one has accessed them before.

  GlobalsMD.init(M);

  C = &(M.getContext());
  LongSize = M.getDataLayout().getPointerSizeInBits();
  IntptrTy = Type::getIntNTy(*C, LongSize);
  TargetTriple = Triple(M.getTargetTriple());

  if (!CompileKernel) {
    std::tie(AsanCtorFunction, AsanInitFunction) =
        createSanitizerCtorAndInitFunctions(M, kAsanModuleCtorName, kAsanInitName,
                                            /*InitArgTypes=*/{},
                                            /*InitArgs=*/{});
    appendToGlobalCtors(M, AsanCtorFunction, kAsanCtorAndDtorPriority);
  }
  Mapping = getShadowMapping(TargetTriple, LongSize, CompileKernel);
  return true;
}

bool AddressSanitizer::maybeInsertAsanInitAtFunctionEntry(Function &F) {
  // For each NSObject descendant having a +load method, this method is invoked
  // by the ObjC runtime before any of the static constructors is called.
  // Therefore we need to instrument such methods with a call to __asan_init
  // at the beginning in order to initialize our runtime before any access to
  // the shadow memory.
  // We cannot just ignore these methods, because they may call other
  // instrumented functions.
  if (F.getName().find(" load]") != std::string::npos) {
    IRBuilder<> IRB(F.begin()->begin());
    IRB.CreateCall(AsanInitFunction, {});
    return true;
  }
  return false;
}

bool AddressSanitizer::runOnFunction(Function &F) {
  if (&F == AsanCtorFunction) return false;
  if (F.getLinkage() == GlobalValue::AvailableExternallyLinkage) return false;
  DEBUG(dbgs() << "ASAN instrumenting:\n" << F << "\n");
  initializeCallbacks(*F.getParent());

  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();

  // If needed, insert __asan_init before checking for SanitizeAddress attr.
  maybeInsertAsanInitAtFunctionEntry(F);

  if (!F.hasFnAttribute(Attribute::SanitizeAddress)) return false;

  if (!ClDebugFunc.empty() && ClDebugFunc != F.getName()) return false;

  // We want to instrument every address only once per basic block (unless there
  // are calls between uses).
  SmallSet<Value *, 16> TempsToInstrument;
  SmallVector<Instruction *, 16> ToInstrument;
  SmallVector<Instruction *, 8> NoReturnCalls;
  SmallVector<BasicBlock *, 16> AllBlocks;
  SmallVector<Instruction *, 16> PointerComparisonsOrSubtracts;
  int NumAllocas = 0;
  bool IsWrite;
  unsigned Alignment;
  uint64_t TypeSize;

  // Fill the set of memory operations to instrument.
  for (auto &BB : F) {
    AllBlocks.push_back(&BB);
    TempsToInstrument.clear();
    int NumInsnsPerBB = 0;
    for (auto &Inst : BB) {
      if (LooksLikeCodeInBug11395(&Inst)) return false;
      if (Value *Addr = isInterestingMemoryAccess(&Inst, &IsWrite, &TypeSize,
                                                  &Alignment)) {
        if (ClOpt && ClOptSameTemp) {
          if (!TempsToInstrument.insert(Addr).second)
            continue;  // We've seen this temp in the current BB.
        }
      } else if (ClInvalidPointerPairs &&
                 isInterestingPointerComparisonOrSubtraction(&Inst)) {
        PointerComparisonsOrSubtracts.push_back(&Inst);
        continue;
      } else if (isa<MemIntrinsic>(Inst)) {
        // ok, take it.
      } else {
        if (isa<AllocaInst>(Inst)) NumAllocas++;
        CallSite CS(&Inst);
        if (CS) {
          // A call inside BB.
          TempsToInstrument.clear();
          if (CS.doesNotReturn()) NoReturnCalls.push_back(CS.getInstruction());
        }
        continue;
      }
      ToInstrument.push_back(&Inst);
      NumInsnsPerBB++;
      if (NumInsnsPerBB >= ClMaxInsnsToInstrumentPerBB) break;
    }
  }

  bool UseCalls =
      CompileKernel ||
      (ClInstrumentationWithCallsThreshold >= 0 &&
       ToInstrument.size() > (unsigned)ClInstrumentationWithCallsThreshold);
  const TargetLibraryInfo *TLI =
      &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();
  const DataLayout &DL = F.getParent()->getDataLayout();
  ObjectSizeOffsetVisitor ObjSizeVis(DL, TLI, F.getContext(),
                                     /*RoundToAlign=*/true);

  // Instrument.
  int NumInstrumented = 0;
  for (auto Inst : ToInstrument) {
    if (ClDebugMin < 0 || ClDebugMax < 0 ||
        (NumInstrumented >= ClDebugMin && NumInstrumented <= ClDebugMax)) {
      if (isInterestingMemoryAccess(Inst, &IsWrite, &TypeSize, &Alignment))
        instrumentMop(ObjSizeVis, Inst, UseCalls,
                      F.getParent()->getDataLayout());
      else
        instrumentMemIntrinsic(cast<MemIntrinsic>(Inst));
    }
    NumInstrumented++;
  }

  FunctionStackPoisoner FSP(F, *this);
  bool ChangedStack = FSP.runOnFunction();

  // We must unpoison the stack before every NoReturn call (throw, _exit, etc).
  // See e.g. http://code.google.com/p/address-sanitizer/issues/detail?id=37
  for (auto CI : NoReturnCalls) {
    IRBuilder<> IRB(CI);
    IRB.CreateCall(AsanHandleNoReturnFunc, {});
  }

  for (auto Inst : PointerComparisonsOrSubtracts) {
    instrumentPointerComparisonOrSubtraction(Inst);
    NumInstrumented++;
  }

  bool res = NumInstrumented > 0 || ChangedStack || !NoReturnCalls.empty();

  DEBUG(dbgs() << "ASAN done instrumenting: " << res << " " << F << "\n");

  return res;
}

// Workaround for bug 11395: we don't want to instrument stack in functions
// with large assembly blobs (32-bit only), otherwise reg alloc may crash.
// FIXME: remove once the bug 11395 is fixed.
bool AddressSanitizer::LooksLikeCodeInBug11395(Instruction *I) {
  if (LongSize != 32) return false;
  CallInst *CI = dyn_cast<CallInst>(I);
  if (!CI || !CI->isInlineAsm()) return false;
  if (CI->getNumArgOperands() <= 5) return false;
  // We have inline assembly with quite a few arguments.
  return true;
}

void FunctionStackPoisoner::initializeCallbacks(Module &M) {
  IRBuilder<> IRB(*C);
  for (int i = 0; i <= kMaxAsanStackMallocSizeClass; i++) {
    std::string Suffix = itostr(i);
    AsanStackMallocFunc[i] = checkSanitizerInterfaceFunction(
        M.getOrInsertFunction(kAsanStackMallocNameTemplate + Suffix, IntptrTy,
                              IntptrTy, nullptr));
    AsanStackFreeFunc[i] = checkSanitizerInterfaceFunction(
        M.getOrInsertFunction(kAsanStackFreeNameTemplate + Suffix,
                              IRB.getVoidTy(), IntptrTy, IntptrTy, nullptr));
  }
  AsanPoisonStackMemoryFunc = checkSanitizerInterfaceFunction(
      M.getOrInsertFunction(kAsanPoisonStackMemoryName, IRB.getVoidTy(),
                            IntptrTy, IntptrTy, nullptr));
  AsanUnpoisonStackMemoryFunc = checkSanitizerInterfaceFunction(
      M.getOrInsertFunction(kAsanUnpoisonStackMemoryName, IRB.getVoidTy(),
                            IntptrTy, IntptrTy, nullptr));
  AsanAllocaPoisonFunc = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      kAsanAllocaPoison, IRB.getVoidTy(), IntptrTy, IntptrTy, nullptr));
  AsanAllocasUnpoisonFunc =
      checkSanitizerInterfaceFunction(M.getOrInsertFunction(
          kAsanAllocasUnpoison, IRB.getVoidTy(), IntptrTy, IntptrTy, nullptr));
}

void FunctionStackPoisoner::poisonRedZones(ArrayRef<uint8_t> ShadowBytes,
                                           IRBuilder<> &IRB, Value *ShadowBase,
                                           bool DoPoison) {
  size_t n = ShadowBytes.size();
  size_t i = 0;
  // We need to (un)poison n bytes of stack shadow. Poison as many as we can
  // using 64-bit stores (if we are on 64-bit arch), then poison the rest
  // with 32-bit stores, then with 16-byte stores, then with 8-byte stores.
  for (size_t LargeStoreSizeInBytes = ASan.LongSize / 8;
       LargeStoreSizeInBytes != 0; LargeStoreSizeInBytes /= 2) {
    for (; i + LargeStoreSizeInBytes - 1 < n; i += LargeStoreSizeInBytes) {
      uint64_t Val = 0;
      for (size_t j = 0; j < LargeStoreSizeInBytes; j++) {
        if (F.getParent()->getDataLayout().isLittleEndian())
          Val |= (uint64_t)ShadowBytes[i + j] << (8 * j);
        else
          Val = (Val << 8) | ShadowBytes[i + j];
      }
      if (!Val) continue;
      Value *Ptr = IRB.CreateAdd(ShadowBase, ConstantInt::get(IntptrTy, i));
      Type *StoreTy = Type::getIntNTy(*C, LargeStoreSizeInBytes * 8);
      Value *Poison = ConstantInt::get(StoreTy, DoPoison ? Val : 0);
      IRB.CreateStore(Poison, IRB.CreateIntToPtr(Ptr, StoreTy->getPointerTo()));
    }
  }
}

// Fake stack allocator (asan_fake_stack.h) has 11 size classes
// for every power of 2 from kMinStackMallocSize to kMaxAsanStackMallocSizeClass
static int StackMallocSizeClass(uint64_t LocalStackSize) {
  assert(LocalStackSize <= kMaxStackMallocSize);
  uint64_t MaxSize = kMinStackMallocSize;
  for (int i = 0;; i++, MaxSize *= 2)
    if (LocalStackSize <= MaxSize) return i;
  llvm_unreachable("impossible LocalStackSize");
}

// Set Size bytes starting from ShadowBase to kAsanStackAfterReturnMagic.
// We can not use MemSet intrinsic because it may end up calling the actual
// memset. Size is a multiple of 8.
// Currently this generates 8-byte stores on x86_64; it may be better to
// generate wider stores.
void FunctionStackPoisoner::SetShadowToStackAfterReturnInlined(
    IRBuilder<> &IRB, Value *ShadowBase, int Size) {
  assert(!(Size % 8));

  // kAsanStackAfterReturnMagic is 0xf5.
  const uint64_t kAsanStackAfterReturnMagic64 = 0xf5f5f5f5f5f5f5f5ULL;

  for (int i = 0; i < Size; i += 8) {
    Value *p = IRB.CreateAdd(ShadowBase, ConstantInt::get(IntptrTy, i));
    IRB.CreateStore(
        ConstantInt::get(IRB.getInt64Ty(), kAsanStackAfterReturnMagic64),
        IRB.CreateIntToPtr(p, IRB.getInt64Ty()->getPointerTo()));
  }
}

PHINode *FunctionStackPoisoner::createPHI(IRBuilder<> &IRB, Value *Cond,
                                          Value *ValueIfTrue,
                                          Instruction *ThenTerm,
                                          Value *ValueIfFalse) {
  PHINode *PHI = IRB.CreatePHI(IntptrTy, 2);
  BasicBlock *CondBlock = cast<Instruction>(Cond)->getParent();
  PHI->addIncoming(ValueIfFalse, CondBlock);
  BasicBlock *ThenBlock = ThenTerm->getParent();
  PHI->addIncoming(ValueIfTrue, ThenBlock);
  return PHI;
}

Value *FunctionStackPoisoner::createAllocaForLayout(
    IRBuilder<> &IRB, const ASanStackFrameLayout &L, bool Dynamic) {
  AllocaInst *Alloca;
  if (Dynamic) {
    Alloca = IRB.CreateAlloca(IRB.getInt8Ty(),
                              ConstantInt::get(IRB.getInt64Ty(), L.FrameSize),
                              "MyAlloca");
  } else {
    Alloca = IRB.CreateAlloca(ArrayType::get(IRB.getInt8Ty(), L.FrameSize),
                              nullptr, "MyAlloca");
    assert(Alloca->isStaticAlloca());
  }
  assert((ClRealignStack & (ClRealignStack - 1)) == 0);
  size_t FrameAlignment = std::max(L.FrameAlignment, (size_t)ClRealignStack);
  Alloca->setAlignment(FrameAlignment);
  return IRB.CreatePointerCast(Alloca, IntptrTy);
}

void FunctionStackPoisoner::createDynamicAllocasInitStorage() {
  BasicBlock &FirstBB = *F.begin();
  IRBuilder<> IRB(dyn_cast<Instruction>(FirstBB.begin()));
  DynamicAllocaLayout = IRB.CreateAlloca(IntptrTy, nullptr);
  IRB.CreateStore(Constant::getNullValue(IntptrTy), DynamicAllocaLayout);
  DynamicAllocaLayout->setAlignment(32);
}

void FunctionStackPoisoner::poisonStack() {
  assert(AllocaVec.size() > 0 || DynamicAllocaVec.size() > 0);

  if (ClInstrumentAllocas && DynamicAllocaVec.size() > 0) {
    // Handle dynamic allocas.
    createDynamicAllocasInitStorage();
    for (auto &AI : DynamicAllocaVec) handleDynamicAllocaCall(AI);

    unpoisonDynamicAllocas();
  }

  if (AllocaVec.size() == 0) return;

  int StackMallocIdx = -1;
  DebugLoc EntryDebugLocation;
  if (auto SP = getDISubprogram(&F))
    EntryDebugLocation = DebugLoc::get(SP->getScopeLine(), 0, SP);

  Instruction *InsBefore = AllocaVec[0];
  IRBuilder<> IRB(InsBefore);
  IRB.SetCurrentDebugLocation(EntryDebugLocation);

  SmallVector<ASanStackVariableDescription, 16> SVD;
  SVD.reserve(AllocaVec.size());
  for (AllocaInst *AI : AllocaVec) {
    ASanStackVariableDescription D = {AI->getName().data(),
                                      ASan.getAllocaSizeInBytes(AI),
                                      AI->getAlignment(), AI, 0};
    SVD.push_back(D);
  }
  // Minimal header size (left redzone) is 4 pointers,
  // i.e. 32 bytes on 64-bit platforms and 16 bytes in 32-bit platforms.
  size_t MinHeaderSize = ASan.LongSize / 2;
  ASanStackFrameLayout L;
  ComputeASanStackFrameLayout(SVD, 1UL << Mapping.Scale, MinHeaderSize, &L);
  DEBUG(dbgs() << L.DescriptionString << " --- " << L.FrameSize << "\n");
  uint64_t LocalStackSize = L.FrameSize;
  bool DoStackMalloc = ClUseAfterReturn && !ASan.CompileKernel &&
                       LocalStackSize <= kMaxStackMallocSize;
  // Don't do dynamic alloca or stack malloc in presence of inline asm:
  // too often it makes assumptions on which registers are available.
  bool DoDynamicAlloca = ClDynamicAllocaStack && !HasNonEmptyInlineAsm;
  DoStackMalloc &= !HasNonEmptyInlineAsm;

  Value *StaticAlloca =
      DoDynamicAlloca ? nullptr : createAllocaForLayout(IRB, L, false);

  Value *FakeStack;
  Value *LocalStackBase;

  if (DoStackMalloc) {
    // void *FakeStack = __asan_option_detect_stack_use_after_return
    //     ? __asan_stack_malloc_N(LocalStackSize)
    //     : nullptr;
    // void *LocalStackBase = (FakeStack) ? FakeStack : alloca(LocalStackSize);
    Constant *OptionDetectUAR = F.getParent()->getOrInsertGlobal(
        kAsanOptionDetectUAR, IRB.getInt32Ty());
    Value *UARIsEnabled =
        IRB.CreateICmpNE(IRB.CreateLoad(OptionDetectUAR),
                         Constant::getNullValue(IRB.getInt32Ty()));
    Instruction *Term =
        SplitBlockAndInsertIfThen(UARIsEnabled, InsBefore, false);
    IRBuilder<> IRBIf(Term);
    IRBIf.SetCurrentDebugLocation(EntryDebugLocation);
    StackMallocIdx = StackMallocSizeClass(LocalStackSize);
    assert(StackMallocIdx <= kMaxAsanStackMallocSizeClass);
    Value *FakeStackValue =
        IRBIf.CreateCall(AsanStackMallocFunc[StackMallocIdx],
                         ConstantInt::get(IntptrTy, LocalStackSize));
    IRB.SetInsertPoint(InsBefore);
    IRB.SetCurrentDebugLocation(EntryDebugLocation);
    FakeStack = createPHI(IRB, UARIsEnabled, FakeStackValue, Term,
                          ConstantInt::get(IntptrTy, 0));

    Value *NoFakeStack =
        IRB.CreateICmpEQ(FakeStack, Constant::getNullValue(IntptrTy));
    Term = SplitBlockAndInsertIfThen(NoFakeStack, InsBefore, false);
    IRBIf.SetInsertPoint(Term);
    IRBIf.SetCurrentDebugLocation(EntryDebugLocation);
    Value *AllocaValue =
        DoDynamicAlloca ? createAllocaForLayout(IRBIf, L, true) : StaticAlloca;
    IRB.SetInsertPoint(InsBefore);
    IRB.SetCurrentDebugLocation(EntryDebugLocation);
    LocalStackBase = createPHI(IRB, NoFakeStack, AllocaValue, Term, FakeStack);
  } else {
    // void *FakeStack = nullptr;
    // void *LocalStackBase = alloca(LocalStackSize);
    FakeStack = ConstantInt::get(IntptrTy, 0);
    LocalStackBase =
        DoDynamicAlloca ? createAllocaForLayout(IRB, L, true) : StaticAlloca;
  }

  // Insert poison calls for lifetime intrinsics for alloca.
  bool HavePoisonedAllocas = false;
  for (const auto &APC : AllocaPoisonCallVec) {
    assert(APC.InsBefore);
    assert(APC.AI);
    IRBuilder<> IRB(APC.InsBefore);
    poisonAlloca(APC.AI, APC.Size, IRB, APC.DoPoison);
    HavePoisonedAllocas |= APC.DoPoison;
  }

  // Replace Alloca instructions with base+offset.
  for (const auto &Desc : SVD) {
    AllocaInst *AI = Desc.AI;
    Value *NewAllocaPtr = IRB.CreateIntToPtr(
        IRB.CreateAdd(LocalStackBase, ConstantInt::get(IntptrTy, Desc.Offset)),
        AI->getType());
    replaceDbgDeclareForAlloca(AI, NewAllocaPtr, DIB, /*Deref=*/true);
    AI->replaceAllUsesWith(NewAllocaPtr);
  }

  // The left-most redzone has enough space for at least 4 pointers.
  // Write the Magic value to redzone[0].
  Value *BasePlus0 = IRB.CreateIntToPtr(LocalStackBase, IntptrPtrTy);
  IRB.CreateStore(ConstantInt::get(IntptrTy, kCurrentStackFrameMagic),
                  BasePlus0);
  // Write the frame description constant to redzone[1].
  Value *BasePlus1 = IRB.CreateIntToPtr(
      IRB.CreateAdd(LocalStackBase,
                    ConstantInt::get(IntptrTy, ASan.LongSize / 8)),
      IntptrPtrTy);
  GlobalVariable *StackDescriptionGlobal =
      createPrivateGlobalForString(*F.getParent(), L.DescriptionString,
                                   /*AllowMerging*/ true);
  Value *Description = IRB.CreatePointerCast(StackDescriptionGlobal, IntptrTy);
  IRB.CreateStore(Description, BasePlus1);
  // Write the PC to redzone[2].
  Value *BasePlus2 = IRB.CreateIntToPtr(
      IRB.CreateAdd(LocalStackBase,
                    ConstantInt::get(IntptrTy, 2 * ASan.LongSize / 8)),
      IntptrPtrTy);
  IRB.CreateStore(IRB.CreatePointerCast(&F, IntptrTy), BasePlus2);

  // Poison the stack redzones at the entry.
  Value *ShadowBase = ASan.memToShadow(LocalStackBase, IRB);
  poisonRedZones(L.ShadowBytes, IRB, ShadowBase, true);

  // (Un)poison the stack before all ret instructions.
  for (auto Ret : RetVec) {
    IRBuilder<> IRBRet(Ret);
    // Mark the current frame as retired.
    IRBRet.CreateStore(ConstantInt::get(IntptrTy, kRetiredStackFrameMagic),
                       BasePlus0);
    if (DoStackMalloc) {
      assert(StackMallocIdx >= 0);
      // if FakeStack != 0  // LocalStackBase == FakeStack
      //     // In use-after-return mode, poison the whole stack frame.
      //     if StackMallocIdx <= 4
      //         // For small sizes inline the whole thing:
      //         memset(ShadowBase, kAsanStackAfterReturnMagic, ShadowSize);
      //         **SavedFlagPtr(FakeStack) = 0
      //     else
      //         __asan_stack_free_N(FakeStack, LocalStackSize)
      // else
      //     <This is not a fake stack; unpoison the redzones>
      Value *Cmp =
          IRBRet.CreateICmpNE(FakeStack, Constant::getNullValue(IntptrTy));
      TerminatorInst *ThenTerm, *ElseTerm;
      SplitBlockAndInsertIfThenElse(Cmp, Ret, &ThenTerm, &ElseTerm);

      IRBuilder<> IRBPoison(ThenTerm);
      if (StackMallocIdx <= 4) {
        int ClassSize = kMinStackMallocSize << StackMallocIdx;
        SetShadowToStackAfterReturnInlined(IRBPoison, ShadowBase,
                                           ClassSize >> Mapping.Scale);
        Value *SavedFlagPtrPtr = IRBPoison.CreateAdd(
            FakeStack,
            ConstantInt::get(IntptrTy, ClassSize - ASan.LongSize / 8));
        Value *SavedFlagPtr = IRBPoison.CreateLoad(
            IRBPoison.CreateIntToPtr(SavedFlagPtrPtr, IntptrPtrTy));
        IRBPoison.CreateStore(
            Constant::getNullValue(IRBPoison.getInt8Ty()),
            IRBPoison.CreateIntToPtr(SavedFlagPtr, IRBPoison.getInt8PtrTy()));
      } else {
        // For larger frames call __asan_stack_free_*.
        IRBPoison.CreateCall(
            AsanStackFreeFunc[StackMallocIdx],
            {FakeStack, ConstantInt::get(IntptrTy, LocalStackSize)});
      }

      IRBuilder<> IRBElse(ElseTerm);
      poisonRedZones(L.ShadowBytes, IRBElse, ShadowBase, false);
    } else if (HavePoisonedAllocas) {
      // If we poisoned some allocas in llvm.lifetime analysis,
      // unpoison whole stack frame now.
      poisonAlloca(LocalStackBase, LocalStackSize, IRBRet, false);
    } else {
      poisonRedZones(L.ShadowBytes, IRBRet, ShadowBase, false);
    }
  }

  // We are done. Remove the old unused alloca instructions.
  for (auto AI : AllocaVec) AI->eraseFromParent();
}

void FunctionStackPoisoner::poisonAlloca(Value *V, uint64_t Size,
                                         IRBuilder<> &IRB, bool DoPoison) {
  // For now just insert the call to ASan runtime.
  Value *AddrArg = IRB.CreatePointerCast(V, IntptrTy);
  Value *SizeArg = ConstantInt::get(IntptrTy, Size);
  IRB.CreateCall(
      DoPoison ? AsanPoisonStackMemoryFunc : AsanUnpoisonStackMemoryFunc,
      {AddrArg, SizeArg});
}

// Handling llvm.lifetime intrinsics for a given %alloca:
// (1) collect all llvm.lifetime.xxx(%size, %value) describing the alloca.
// (2) if %size is constant, poison memory for llvm.lifetime.end (to detect
//     invalid accesses) and unpoison it for llvm.lifetime.start (the memory
//     could be poisoned by previous llvm.lifetime.end instruction, as the
//     variable may go in and out of scope several times, e.g. in loops).
// (3) if we poisoned at least one %alloca in a function,
//     unpoison the whole stack frame at function exit.

AllocaInst *FunctionStackPoisoner::findAllocaForValue(Value *V) {
  if (AllocaInst *AI = dyn_cast<AllocaInst>(V))
    // We're intested only in allocas we can handle.
    return ASan.isInterestingAlloca(*AI) ? AI : nullptr;
  // See if we've already calculated (or started to calculate) alloca for a
  // given value.
  AllocaForValueMapTy::iterator I = AllocaForValue.find(V);
  if (I != AllocaForValue.end()) return I->second;
  // Store 0 while we're calculating alloca for value V to avoid
  // infinite recursion if the value references itself.
  AllocaForValue[V] = nullptr;
  AllocaInst *Res = nullptr;
  if (CastInst *CI = dyn_cast<CastInst>(V))
    Res = findAllocaForValue(CI->getOperand(0));
  else if (PHINode *PN = dyn_cast<PHINode>(V)) {
    for (Value *IncValue : PN->incoming_values()) {
      // Allow self-referencing phi-nodes.
      if (IncValue == PN) continue;
      AllocaInst *IncValueAI = findAllocaForValue(IncValue);
      // AI for incoming values should exist and should all be equal.
      if (IncValueAI == nullptr || (Res != nullptr && IncValueAI != Res))
        return nullptr;
      Res = IncValueAI;
    }
  }
  if (Res) AllocaForValue[V] = Res;
  return Res;
}

void FunctionStackPoisoner::handleDynamicAllocaCall(AllocaInst *AI) {
  IRBuilder<> IRB(AI);

  const unsigned Align = std::max(kAllocaRzSize, AI->getAlignment());
  const uint64_t AllocaRedzoneMask = kAllocaRzSize - 1;

  Value *Zero = Constant::getNullValue(IntptrTy);
  Value *AllocaRzSize = ConstantInt::get(IntptrTy, kAllocaRzSize);
  Value *AllocaRzMask = ConstantInt::get(IntptrTy, AllocaRedzoneMask);

  // Since we need to extend alloca with additional memory to locate
  // redzones, and OldSize is number of allocated blocks with
  // ElementSize size, get allocated memory size in bytes by
  // OldSize * ElementSize.
  const unsigned ElementSize =
      F.getParent()->getDataLayout().getTypeAllocSize(AI->getAllocatedType());
  Value *OldSize =
      IRB.CreateMul(IRB.CreateIntCast(AI->getArraySize(), IntptrTy, false),
                    ConstantInt::get(IntptrTy, ElementSize));

  // PartialSize = OldSize % 32
  Value *PartialSize = IRB.CreateAnd(OldSize, AllocaRzMask);

  // Misalign = kAllocaRzSize - PartialSize;
  Value *Misalign = IRB.CreateSub(AllocaRzSize, PartialSize);

  // PartialPadding = Misalign != kAllocaRzSize ? Misalign : 0;
  Value *Cond = IRB.CreateICmpNE(Misalign, AllocaRzSize);
  Value *PartialPadding = IRB.CreateSelect(Cond, Misalign, Zero);

  // AdditionalChunkSize = Align + PartialPadding + kAllocaRzSize
  // Align is added to locate left redzone, PartialPadding for possible
  // partial redzone and kAllocaRzSize for right redzone respectively.
  Value *AdditionalChunkSize = IRB.CreateAdd(
      ConstantInt::get(IntptrTy, Align + kAllocaRzSize), PartialPadding);

  Value *NewSize = IRB.CreateAdd(OldSize, AdditionalChunkSize);

  // Insert new alloca with new NewSize and Align params.
  AllocaInst *NewAlloca = IRB.CreateAlloca(IRB.getInt8Ty(), NewSize);
  NewAlloca->setAlignment(Align);

  // NewAddress = Address + Align
  Value *NewAddress = IRB.CreateAdd(IRB.CreatePtrToInt(NewAlloca, IntptrTy),
                                    ConstantInt::get(IntptrTy, Align));

  // Insert __asan_alloca_poison call for new created alloca.
  IRB.CreateCall(AsanAllocaPoisonFunc, {NewAddress, OldSize});

  // Store the last alloca's address to DynamicAllocaLayout. We'll need this
  // for unpoisoning stuff.
  IRB.CreateStore(IRB.CreatePtrToInt(NewAlloca, IntptrTy), DynamicAllocaLayout);

  Value *NewAddressPtr = IRB.CreateIntToPtr(NewAddress, AI->getType());

  // Replace all uses of AddessReturnedByAlloca with NewAddressPtr.
  AI->replaceAllUsesWith(NewAddressPtr);

  // We are done. Erase old alloca from parent.
  AI->eraseFromParent();
}

// isSafeAccess returns true if Addr is always inbounds with respect to its
// base object. For example, it is a field access or an array access with
// constant inbounds index.
bool AddressSanitizer::isSafeAccess(ObjectSizeOffsetVisitor &ObjSizeVis,
                                    Value *Addr, uint64_t TypeSize) const {
  SizeOffsetType SizeOffset = ObjSizeVis.compute(Addr);
  if (!ObjSizeVis.bothKnown(SizeOffset)) return false;
  uint64_t Size = SizeOffset.first.getZExtValue();
  int64_t Offset = SizeOffset.second.getSExtValue();
  // Three checks are required to ensure safety:
  // . Offset >= 0  (since the offset is given from the base ptr)
  // . Size >= Offset  (unsigned)
  // . Size - Offset >= NeededSize  (unsigned)
  return Offset >= 0 && Size >= uint64_t(Offset) &&
         Size - uint64_t(Offset) >= TypeSize / 8;
}
