//===- subzero/src/IceGlobalContext.cpp - Global context defs -------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines aspects of the compilation that persist across multiple
/// functions.
///
//===----------------------------------------------------------------------===//

#include "IceGlobalContext.h"

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceClFlags.h"
#include "IceDefs.h"
#include "IceELFObjectWriter.h"
#include "IceGlobalInits.h"
#include "IceLiveness.h"
#include "IceOperand.h"
#include "IceRevision.h"
#include "IceTargetLowering.h"
#include "IceTimerTree.h"
#include "IceTypes.def"
#include "IceTypes.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif // __clang__

#include "llvm/Support/Timer.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__

#include <algorithm> // max()

namespace std {
template <> struct hash<Ice::RelocatableTuple> {
  size_t operator()(const Ice::RelocatableTuple &Key) const {
    // Use the relocatable's name, plus the hash of a combination of the number
    // of OffsetExprs and the known, fixed offset for the reloc. We left shift
    // the known relocatable by 5 trying to minimize the interaction between the
    // bits in OffsetExpr.size() and Key.Offset.
    return hash<Ice::SizeT>()(Key.Name.getID()) +
           hash<std::size_t>()(Key.OffsetExpr.size() + (Key.Offset << 5));
  }
};
} // end of namespace std

namespace Ice {

namespace {

// Define the key comparison function for the constant pool's unordered_map,
// but only for key types of interest: integer types, floating point types, and
// the special RelocatableTuple.
template <typename KeyType, class Enable = void> struct KeyCompare {};

template <typename KeyType>
struct KeyCompare<KeyType,
                  typename std::enable_if<
                      std::is_integral<KeyType>::value ||
                      std::is_same<KeyType, RelocatableTuple>::value>::type> {
  bool operator()(const KeyType &Value1, const KeyType &Value2) const {
    return Value1 == Value2;
  }
};
template <typename KeyType>
struct KeyCompare<KeyType, typename std::enable_if<
                               std::is_floating_point<KeyType>::value>::type> {
  bool operator()(const KeyType &Value1, const KeyType &Value2) const {
    return !memcmp(&Value1, &Value2, sizeof(KeyType));
  }
};

// Define a key comparison function for sorting the constant pool's values
// after they are dumped to a vector. This covers integer types, floating point
// types, and ConstantRelocatable values.
template <typename ValueType, class Enable = void> struct KeyCompareLess {};

template <typename ValueType>
struct KeyCompareLess<ValueType,
                      typename std::enable_if<std::is_floating_point<
                          typename ValueType::PrimType>::value>::type> {
  bool operator()(const Constant *Const1, const Constant *Const2) const {
    using CompareType = uint64_t;
    static_assert(sizeof(typename ValueType::PrimType) <= sizeof(CompareType),
                  "Expected floating-point type of width 64-bit or less");
    typename ValueType::PrimType V1 = llvm::cast<ValueType>(Const1)->getValue();
    typename ValueType::PrimType V2 = llvm::cast<ValueType>(Const2)->getValue();
    // We avoid "V1<V2" because of NaN.
    // We avoid "memcmp(&V1,&V2,sizeof(V1))<0" which depends on the
    // endian-ness of the host system running Subzero.
    // Instead, compare the result of bit_cast to uint64_t.
    uint64_t I1 = 0, I2 = 0;
    memcpy(&I1, &V1, sizeof(V1));
    memcpy(&I2, &V2, sizeof(V2));
    return I1 < I2;
  }
};
template <typename ValueType>
struct KeyCompareLess<ValueType,
                      typename std::enable_if<std::is_integral<
                          typename ValueType::PrimType>::value>::type> {
  bool operator()(const Constant *Const1, const Constant *Const2) const {
    typename ValueType::PrimType V1 = llvm::cast<ValueType>(Const1)->getValue();
    typename ValueType::PrimType V2 = llvm::cast<ValueType>(Const2)->getValue();
    return V1 < V2;
  }
};
template <typename ValueType>
struct KeyCompareLess<
    ValueType, typename std::enable_if<
                   std::is_same<ValueType, ConstantRelocatable>::value>::type> {
  bool operator()(const Constant *Const1, const Constant *Const2) const {
    auto *V1 = llvm::cast<ValueType>(Const1);
    auto *V2 = llvm::cast<ValueType>(Const2);
    if (V1->getName() == V2->getName())
      return V1->getOffset() < V2->getOffset();
    return V1->getName() < V2->getName();
  }
};

// TypePool maps constants of type KeyType (e.g. float) to pointers to
// type ValueType (e.g. ConstantFloat).
template <Type Ty, typename KeyType, typename ValueType> class TypePool {
  TypePool(const TypePool &) = delete;
  TypePool &operator=(const TypePool &) = delete;

public:
  TypePool() = default;
  ValueType *getOrAdd(GlobalContext *Ctx, KeyType Key) {
    auto Iter = Pool.find(Key);
    if (Iter != Pool.end()) {
      Iter->second->updateLookupCount();
      return Iter->second;
    }
    auto *Result = ValueType::create(Ctx, Ty, Key);
    Pool[Key] = Result;
    Result->updateLookupCount();
    return Result;
  }
  ConstantList getConstantPool() const {
    ConstantList Constants;
    Constants.reserve(Pool.size());
    for (auto &I : Pool)
      Constants.push_back(I.second);
    // The sort (and its KeyCompareLess machinery) is not strictly necessary,
    // but is desirable for producing output that is deterministic across
    // unordered_map::iterator implementations.
    std::sort(Constants.begin(), Constants.end(), KeyCompareLess<ValueType>());
    return Constants;
  }
  size_t size() const { return Pool.size(); }

private:
  // Use the default hash function, and a custom key comparison function. The
  // key comparison function for floating point variables can't use the default
  // == based implementation because of special C++ semantics regarding +0.0,
  // -0.0, and NaN comparison. However, it's OK to use the default hash for
  // floating point values because KeyCompare is the final source of truth - in
  // the worst case a "false" collision must be resolved.
  using ContainerType =
      std::unordered_map<KeyType, ValueType *, std::hash<KeyType>,
                         KeyCompare<KeyType>>;
  ContainerType Pool;
};

// UndefPool maps ICE types to the corresponding ConstantUndef values.
class UndefPool {
  UndefPool(const UndefPool &) = delete;
  UndefPool &operator=(const UndefPool &) = delete;

public:
  UndefPool() : Pool(IceType_NUM) {}

  ConstantUndef *getOrAdd(GlobalContext *Ctx, Type Ty) {
    if (Pool[Ty] == nullptr)
      Pool[Ty] = ConstantUndef::create(Ctx, Ty);
    return Pool[Ty];
  }

private:
  std::vector<ConstantUndef *> Pool;
};

} // end of anonymous namespace

// The global constant pool bundles individual pools of each type of
// interest.
class ConstantPool {
  ConstantPool(const ConstantPool &) = delete;
  ConstantPool &operator=(const ConstantPool &) = delete;

public:
  ConstantPool() = default;
  TypePool<IceType_f32, float, ConstantFloat> Floats;
  TypePool<IceType_f64, double, ConstantDouble> Doubles;
  TypePool<IceType_i1, int8_t, ConstantInteger32> Integers1;
  TypePool<IceType_i8, int8_t, ConstantInteger32> Integers8;
  TypePool<IceType_i16, int16_t, ConstantInteger32> Integers16;
  TypePool<IceType_i32, int32_t, ConstantInteger32> Integers32;
  TypePool<IceType_i64, int64_t, ConstantInteger64> Integers64;
  TypePool<IceType_i32, RelocatableTuple, ConstantRelocatable> Relocatables;
  TypePool<IceType_i32, RelocatableTuple, ConstantRelocatable>
      ExternRelocatables;
  UndefPool Undefs;
};

void GlobalContext::waitForWorkerThreads() {
  if (WaitForWorkerThreadsCalled.exchange(true))
    return;
  optQueueNotifyEnd();
  for (std::thread &Worker : TranslationThreads) {
    Worker.join();
  }
  TranslationThreads.clear();

  // Only notify the emit queue to end after all the translation threads have
  // ended.
  emitQueueNotifyEnd();
  for (std::thread &Worker : EmitterThreads) {
    Worker.join();
  }
  EmitterThreads.clear();

  if (BuildDefs::timers()) {
    auto Timers = getTimers();
    for (ThreadContext *TLS : AllThreadContexts)
      Timers->mergeFrom(TLS->Timers);
  }
  if (BuildDefs::dump()) {
    // Do a separate loop over AllThreadContexts to avoid holding two locks at
    // once.
    auto Stats = getStatsCumulative();
    for (ThreadContext *TLS : AllThreadContexts)
      Stats->add(TLS->StatsCumulative);
  }
}

void GlobalContext::CodeStats::dump(const Cfg *Func, GlobalContext *Ctx) {
  if (!BuildDefs::dump())
    return;
  OstreamLocker _(Ctx);
  Ostream &Str = Ctx->getStrDump();
  const std::string Name =
      (Func == nullptr ? "_FINAL_" : Func->getFunctionNameAndSize());
#define X(str, tag)                                                            \
  Str << "|" << Name << "|" str "|" << Stats[CS_##tag] << "\n";
  CODESTATS_TABLE
#undef X
  Str << "|" << Name << "|Spills+Fills|"
      << Stats[CS_NumSpills] + Stats[CS_NumFills] << "\n";
  Str << "|" << Name << "|Memory Usage     |";
  if (const auto MemUsed = static_cast<size_t>(
          llvm::TimeRecord::getCurrentTime(false).getMemUsed())) {
    static constexpr size_t _1MB = 1024 * 1024;
    Str << (MemUsed / _1MB) << " MB";
  } else {
    Str << "(requires '-track-memory')";
  }
  Str << "\n";
  Str << "|" << Name << "|CPool Sizes ";
  {
    auto Pool = Ctx->getConstPool();
    Str << "|f32=" << Pool->Floats.size();
    Str << "|f64=" << Pool->Doubles.size();
    Str << "|i1=" << Pool->Integers1.size();
    Str << "|i8=" << Pool->Integers8.size();
    Str << "|i16=" << Pool->Integers16.size();
    Str << "|i32=" << Pool->Integers32.size();
    Str << "|i64=" << Pool->Integers64.size();
    Str << "|Rel=" << Pool->Relocatables.size();
    Str << "|ExtRel=" << Pool->ExternRelocatables.size();
  }
  Str << "\n";
  if (Func != nullptr) {
    Str << "|" << Name << "|Cfg Memory       |" << Func->getTotalMemoryMB()
        << " MB\n";
    Str << "|" << Name << "|Liveness Memory  |" << Func->getLivenessMemoryMB()
        << " MB\n";
  }
}

namespace {

// By default, wake up the main parser thread when the OptQ gets half empty.
static constexpr size_t DefaultOptQWakeupSize = GlobalContext::MaxOptQSize >> 1;

} // end of anonymous namespace

GlobalContext::GlobalContext(Ostream *OsDump, Ostream *OsEmit, Ostream *OsError,
                             ELFStreamer *ELFStr)
    : Strings(new StringPool()), ConstPool(new ConstantPool()), ErrorStatus(),
      StrDump(OsDump), StrEmit(OsEmit), StrError(OsError), ObjectWriter(),
      OptQWakeupSize(std::max(DefaultOptQWakeupSize,
                              size_t(getFlags().getNumTranslationThreads()))),
      OptQ(/*Sequential=*/getFlags().isSequential(),
           /*MaxSize=*/
           getFlags().isParseParallel()
               ? MaxOptQSize
               : getFlags().getNumTranslationThreads()),
      // EmitQ is allowed unlimited size.
      EmitQ(/*Sequential=*/getFlags().isSequential()),
      DataLowering(TargetDataLowering::createLowering(this)) {
  assert(OsDump && "OsDump is not defined for GlobalContext");
  assert(OsEmit && "OsEmit is not defined for GlobalContext");
  assert(OsError && "OsError is not defined for GlobalContext");
  // Make sure thread_local fields are properly initialized before any
  // accesses are made.  Do this here instead of at the start of
  // main() so that all clients (e.g. unit tests) can benefit for
  // free.
  GlobalContext::TlsInit();
  Cfg::TlsInit();
  Liveness::TlsInit();
  // Create a new ThreadContext for the current thread.  No need to
  // lock AllThreadContexts at this point since no other threads have
  // access yet to this GlobalContext object.
  ThreadContext *MyTLS = new ThreadContext();
  AllThreadContexts.push_back(MyTLS);
  ICE_TLS_SET_FIELD(TLS, MyTLS);
  // Pre-register built-in stack names.
  if (BuildDefs::timers()) {
    // TODO(stichnot): There needs to be a strong relationship between
    // the newTimerStackID() return values and TSK_Default/TSK_Funcs.
    newTimerStackID("Total across all functions");
    newTimerStackID("Per-function summary");
  }
  Timers.initInto(MyTLS->Timers);
  switch (getFlags().getOutFileType()) {
  case FT_Elf:
    ObjectWriter.reset(new ELFObjectWriter(*this, *ELFStr));
    break;
  case FT_Asm:
  case FT_Iasm:
    break;
  }
// Cache up front common constants.
#define X(tag, sizeLog2, align, elts, elty, str, rcstr)                        \
  ConstZeroForType[IceType_##tag] = getConstantZeroInternal(IceType_##tag);
  ICETYPE_TABLE;
#undef X
  ConstantTrue = getConstantInt1Internal(1);
// Define runtime helper functions.
#define X(Tag, Name)                                                           \
  RuntimeHelperFunc[static_cast<size_t>(RuntimeHelper::H_##Tag)] =             \
      getConstantExternSym(getGlobalString(Name));
  RUNTIME_HELPER_FUNCTIONS_TABLE
#undef X

  TargetLowering::staticInit(this);

  if (getFlags().getEmitRevision()) {
    // Embed the Subzero revision into the compiled binary by creating a special
    // global variable initialized with the revision string.
    auto *Revision = VariableDeclaration::create(&Globals, true);
    Revision->setName(this, "__Sz_revision");
    Revision->setIsConstant(true);
    const char *RevisionString = getSubzeroRevision();
    Revision->addInitializer(VariableDeclaration::DataInitializer::create(
        &Globals, RevisionString, 1 + strlen(RevisionString)));
    Globals.push_back(Revision);
  }
}

void GlobalContext::translateFunctionsWrapper(ThreadContext *MyTLS) {
  ICE_TLS_SET_FIELD(TLS, MyTLS);
  translateFunctions();
}

void GlobalContext::translateFunctions() {
  TimerMarker Timer(TimerStack::TT_translateFunctions, this);
  while (std::unique_ptr<OptWorkItem> OptItem = optQueueBlockingPop()) {
    std::unique_ptr<EmitterWorkItem> Item;
    auto Func = OptItem->getParsedCfg();
    // Install Func in TLS for Cfg-specific container allocators.
    CfgLocalAllocatorScope _(Func.get());
    // Reset per-function stats being accumulated in TLS.
    resetStats();
    // Set verbose level to none if the current function does NOT match the
    // -verbose-focus command-line option.
    if (!getFlags().matchVerboseFocusOn(Func->getFunctionName(),
                                        Func->getSequenceNumber()))
      Func->setVerbose(IceV_None);
    // Disable translation if -notranslate is specified, or if the current
    // function matches the -translate-only option.  If translation is disabled,
    // just dump the high-level IR and continue.
    if (getFlags().getDisableTranslation() ||
        !getFlags().matchTranslateOnly(Func->getFunctionName(),
                                       Func->getSequenceNumber())) {
      Func->dump();
      // Add a dummy work item as a placeholder.  This maintains sequence
      // numbers so that the emitter thread will emit subsequent functions.
      Item = makeUnique<EmitterWorkItem>(Func->getSequenceNumber());
      emitQueueBlockingPush(std::move(Item));
      continue; // Func goes out of scope and gets deleted
    }

    Func->translate();
    if (Func->hasError()) {
      getErrorStatus()->assign(EC_Translation);
      OstreamLocker L(this);
      getStrError() << "ICE translation error: " << Func->getFunctionName()
                    << ": " << Func->getError() << ": "
                    << Func->getFunctionNameAndSize() << "\n";
      Item = makeUnique<EmitterWorkItem>(Func->getSequenceNumber());
    } else {
      Func->getAssembler<>()->setInternal(Func->getInternal());
      switch (getFlags().getOutFileType()) {
      case FT_Elf:
      case FT_Iasm: {
        Func->emitIAS();
        // The Cfg has already emitted into the assembly buffer, so
        // stats have been fully collected into this thread's TLS.
        // Dump them before TLS is reset for the next Cfg.
        if (BuildDefs::dump())
          dumpStats(Func.get());
        auto Asm = Func->releaseAssembler();
        // Copy relevant fields into Asm before Func is deleted.
        Asm->setFunctionName(Func->getFunctionName());
        Item = makeUnique<EmitterWorkItem>(Func->getSequenceNumber(),
                                           std::move(Asm));
        Item->setGlobalInits(Func->getGlobalInits());
      } break;
      case FT_Asm:
        // The Cfg has not been emitted yet, so stats are not ready
        // to be dumped.
        std::unique_ptr<VariableDeclarationList> GlobalInits =
            Func->getGlobalInits();
        Item = makeUnique<EmitterWorkItem>(Func->getSequenceNumber(),
                                           std::move(Func));
        Item->setGlobalInits(std::move(GlobalInits));
        break;
      }
    }
    assert(Item != nullptr);
    emitQueueBlockingPush(std::move(Item));
    // The Cfg now gets deleted as Func goes out of scope.
  }
}

namespace {

// Ensure Pending is large enough that Pending[Index] is valid.
void resizePending(std::vector<std::unique_ptr<EmitterWorkItem>> *Pending,
                   uint32_t Index) {
  if (Index >= Pending->size())
    Utils::reserveAndResize(*Pending, Index + 1);
}

} // end of anonymous namespace

// static
void GlobalContext::TlsInit() { ICE_TLS_INIT_FIELD(TLS); }

void GlobalContext::emitFileHeader() {
  TimerMarker T1(Ice::TimerStack::TT_emitAsm, this);
  if (getFlags().getOutFileType() == FT_Elf) {
    getObjectWriter()->writeInitialELFHeader();
  } else {
    if (!BuildDefs::dump()) {
      getStrError() << "emitFileHeader for non-ELF";
      getErrorStatus()->assign(EC_Translation);
    }
    TargetHeaderLowering::createLowering(this)->lower();
  }
}

void GlobalContext::lowerConstants() { DataLowering->lowerConstants(); }

void GlobalContext::lowerJumpTables() { DataLowering->lowerJumpTables(); }

void GlobalContext::emitTargetRODataSections() {
  DataLowering->emitTargetRODataSections();
}

void GlobalContext::lowerGlobals(const std::string &SectionSuffix) {
  TimerMarker T(TimerStack::TT_emitGlobalInitializers, this);
  const bool DumpGlobalVariables =
      BuildDefs::dump() && (getFlags().getVerbose() & IceV_GlobalInit) &&
      getFlags().matchVerboseFocusOn("", 0);
  if (DumpGlobalVariables) {
    OstreamLocker L(this);
    Ostream &Stream = getStrDump();
    for (const Ice::VariableDeclaration *Global : Globals) {
      Global->dump(Stream);
    }
  }
  if (getFlags().getDisableTranslation())
    return;

  if (!BuildDefs::minimal() && Instrumentor)
    Instrumentor->instrumentGlobals(Globals);

  DataLowering->lowerGlobals(Globals, SectionSuffix);
  if (DisposeGlobalVariablesAfterLowering) {
    Globals.clearAndPurge();
  } else {
    Globals.clear();
  }
}

void GlobalContext::emitterWrapper(ThreadContext *MyTLS) {
  ICE_TLS_SET_FIELD(TLS, MyTLS);
  emitItems();
}

void GlobalContext::emitItems() {
  const bool Threaded = !getFlags().isSequential();
  // Pending is a vector containing the reassembled, ordered list of
  // work items.  When we're ready for the next item, we first check
  // whether it's in the Pending list.  If not, we take an item from
  // the work queue, and if it's not the item we're waiting for, we
  // insert it into Pending and repeat.  The work item is deleted
  // after it is processed.
  std::vector<std::unique_ptr<EmitterWorkItem>> Pending;
  uint32_t DesiredSequenceNumber = getFirstSequenceNumber();
  uint32_t ShuffleStartIndex = DesiredSequenceNumber;
  uint32_t ShuffleEndIndex = DesiredSequenceNumber;
  bool EmitQueueEmpty = false;

  while (!EmitQueueEmpty) {
    resizePending(&Pending, DesiredSequenceNumber);
    // See if Pending contains DesiredSequenceNumber.
    if (Pending[DesiredSequenceNumber] == nullptr) {
      // We need to fetch an EmitterWorkItem from the queue.
      auto RawItem = emitQueueBlockingPop();
      if (RawItem == nullptr) {
        // This is the notifier for an empty queue.
        EmitQueueEmpty = true;
      } else {
        // We get an EmitterWorkItem, we need to add it to Pending.
        uint32_t ItemSeq = RawItem->getSequenceNumber();
        if (Threaded && ItemSeq != DesiredSequenceNumber) {
          // Not the desired one, add it to Pending but do not increase
          // DesiredSequenceNumber. Continue the loop, do not emit the item.
          resizePending(&Pending, ItemSeq);
          Pending[ItemSeq] = std::move(RawItem);
          continue;
        }
        // ItemSeq == DesiredSequenceNumber, we need to check if we should
        // emit it or not. If !Threaded, we're OK with ItemSeq !=
        // DesiredSequenceNumber.
        Pending[DesiredSequenceNumber] = std::move(RawItem);
      }
    }

    // We have the desired EmitterWorkItem or nullptr as the end notifier.
    // If the emitter queue is not empty, increase DesiredSequenceNumber and
    // ShuffleEndIndex.
    if (!EmitQueueEmpty) {
      DesiredSequenceNumber++;
      ShuffleEndIndex++;
    }

    // Emit the item from ShuffleStartIndex to ShuffleEndIndex.
    for (uint32_t I = ShuffleStartIndex; I < ShuffleEndIndex; I++) {
      std::unique_ptr<EmitterWorkItem> Item = std::move(Pending[I]);

      switch (Item->getKind()) {
      case EmitterWorkItem::WI_Nop:
        break;
      case EmitterWorkItem::WI_GlobalInits: {
        accumulateGlobals(Item->getGlobalInits());
      } break;
      case EmitterWorkItem::WI_Asm: {
        lowerGlobalsIfNoCodeHasBeenSeen();
        accumulateGlobals(Item->getGlobalInits());

        std::unique_ptr<Assembler> Asm = Item->getAsm();
        Asm->alignFunction();
        GlobalString Name = Asm->getFunctionName();
        switch (getFlags().getOutFileType()) {
        case FT_Elf:
          getObjectWriter()->writeFunctionCode(Name, Asm->getInternal(),
                                               Asm.get());
          break;
        case FT_Iasm: {
          OstreamLocker L(this);
          Cfg::emitTextHeader(Name, this, Asm.get());
          Asm->emitIASBytes(this);
        } break;
        case FT_Asm:
          llvm::report_fatal_error("Unexpected FT_Asm");
          break;
        }
      } break;
      case EmitterWorkItem::WI_Cfg: {
        if (!BuildDefs::dump())
          llvm::report_fatal_error("WI_Cfg work item created inappropriately");
        lowerGlobalsIfNoCodeHasBeenSeen();
        accumulateGlobals(Item->getGlobalInits());

        assert(getFlags().getOutFileType() == FT_Asm);
        std::unique_ptr<Cfg> Func = Item->getCfg();
        // Unfortunately, we have to temporarily install the Cfg in TLS
        // because Variable::asType() uses the allocator to create the
        // differently-typed copy.
        CfgLocalAllocatorScope _(Func.get());
        Func->emit();
        dumpStats(Func.get());
      } break;
      }
    }
    // Update the start index for next shuffling queue
    ShuffleStartIndex = ShuffleEndIndex;
  }

  // In case there are no code to be generated, we invoke the conditional
  // lowerGlobals again -- this is a no-op if code has been emitted.
  lowerGlobalsIfNoCodeHasBeenSeen();
}

GlobalContext::~GlobalContext() {
  llvm::DeleteContainerPointers(AllThreadContexts);
  LockedPtr<DestructorArray> Dtors = getDestructors();
  // Destructors are invoked in the opposite object construction order.
  for (const auto &Dtor : reverse_range(*Dtors))
    Dtor();
}

void GlobalContext::dumpStrings() {
  if (!getFlags().getDumpStrings())
    return;
  OstreamLocker _(this);
  Ostream &Str = getStrDump();
  Str << "GlobalContext strings:\n";
  getStrings()->dump(Str);
}

void GlobalContext::dumpConstantLookupCounts() {
  if (!BuildDefs::dump())
    return;
  const bool DumpCounts = (getFlags().getVerbose() & IceV_ConstPoolStats) &&
                          getFlags().matchVerboseFocusOn("", 0);
  if (!DumpCounts)
    return;

  OstreamLocker _(this);
  Ostream &Str = getStrDump();
  Str << "Constant pool use stats: count+value+type\n";
#define X(WhichPool)                                                           \
  for (auto *C : getConstPool()->WhichPool.getConstantPool()) {                \
    Str << C->getLookupCount() << " ";                                         \
    C->dump(Str);                                                              \
    Str << " " << C->getType() << "\n";                                        \
  }
  X(Integers1);
  X(Integers8);
  X(Integers16);
  X(Integers32);
  X(Integers64);
  X(Floats);
  X(Doubles);
  X(Relocatables);
  X(ExternRelocatables);
#undef X
}

// TODO(stichnot): Consider adding thread-local caches of constant pool entries
// to reduce contention.

// All locking is done by the getConstantInt[0-9]+() target function.
Constant *GlobalContext::getConstantInt(Type Ty, int64_t Value) {
  switch (Ty) {
  case IceType_i1:
    return getConstantInt1(Value);
  case IceType_i8:
    return getConstantInt8(Value);
  case IceType_i16:
    return getConstantInt16(Value);
  case IceType_i32:
    return getConstantInt32(Value);
  case IceType_i64:
    return getConstantInt64(Value);
  default:
    llvm_unreachable("Bad integer type for getConstant");
  }
  return nullptr;
}

Constant *GlobalContext::getConstantInt1Internal(int8_t ConstantInt1) {
  ConstantInt1 &= INT8_C(1);
  return getConstPool()->Integers1.getOrAdd(this, ConstantInt1);
}

Constant *GlobalContext::getConstantInt8Internal(int8_t ConstantInt8) {
  return getConstPool()->Integers8.getOrAdd(this, ConstantInt8);
}

Constant *GlobalContext::getConstantInt16Internal(int16_t ConstantInt16) {
  return getConstPool()->Integers16.getOrAdd(this, ConstantInt16);
}

Constant *GlobalContext::getConstantInt32Internal(int32_t ConstantInt32) {
  return getConstPool()->Integers32.getOrAdd(this, ConstantInt32);
}

Constant *GlobalContext::getConstantInt64Internal(int64_t ConstantInt64) {
  return getConstPool()->Integers64.getOrAdd(this, ConstantInt64);
}

Constant *GlobalContext::getConstantFloat(float ConstantFloat) {
  return getConstPool()->Floats.getOrAdd(this, ConstantFloat);
}

Constant *GlobalContext::getConstantDouble(double ConstantDouble) {
  return getConstPool()->Doubles.getOrAdd(this, ConstantDouble);
}

Constant *GlobalContext::getConstantSymWithEmitString(
    const RelocOffsetT Offset, const RelocOffsetArray &OffsetExpr,
    GlobalString Name, const std::string &EmitString) {
  return getConstPool()->Relocatables.getOrAdd(
      this, RelocatableTuple(Offset, OffsetExpr, Name, EmitString));
}

Constant *GlobalContext::getConstantSym(RelocOffsetT Offset,
                                        GlobalString Name) {
  constexpr char EmptyEmitString[] = "";
  return getConstantSymWithEmitString(Offset, {}, Name, EmptyEmitString);
}

Constant *GlobalContext::getConstantExternSym(GlobalString Name) {
  constexpr RelocOffsetT Offset = 0;
  return getConstPool()->ExternRelocatables.getOrAdd(
      this, RelocatableTuple(Offset, {}, Name));
}

Constant *GlobalContext::getConstantUndef(Type Ty) {
  return getConstPool()->Undefs.getOrAdd(this, Ty);
}

Constant *GlobalContext::getConstantZero(Type Ty) {
  Constant *Zero = ConstZeroForType[Ty];
  if (Zero == nullptr)
    llvm::report_fatal_error("Unsupported constant type: " + typeStdString(Ty));
  return Zero;
}

// All locking is done by the getConstant*() target function.
Constant *GlobalContext::getConstantZeroInternal(Type Ty) {
  switch (Ty) {
  case IceType_i1:
    return getConstantInt1Internal(0);
  case IceType_i8:
    return getConstantInt8Internal(0);
  case IceType_i16:
    return getConstantInt16Internal(0);
  case IceType_i32:
    return getConstantInt32Internal(0);
  case IceType_i64:
    return getConstantInt64Internal(0);
  case IceType_f32:
    return getConstantFloat(0);
  case IceType_f64:
    return getConstantDouble(0);
  default:
    return nullptr;
  }
}

ConstantList GlobalContext::getConstantPool(Type Ty) {
  switch (Ty) {
  case IceType_i1:
  case IceType_i8:
    return getConstPool()->Integers8.getConstantPool();
  case IceType_i16:
    return getConstPool()->Integers16.getConstantPool();
  case IceType_i32:
    return getConstPool()->Integers32.getConstantPool();
  case IceType_i64:
    return getConstPool()->Integers64.getConstantPool();
  case IceType_f32:
    return getConstPool()->Floats.getConstantPool();
  case IceType_f64:
    return getConstPool()->Doubles.getConstantPool();
  case IceType_v4i1:
  case IceType_v8i1:
  case IceType_v16i1:
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
  case IceType_v4f32:
    llvm::report_fatal_error("Unsupported constant type: " + typeStdString(Ty));
    break;
  case IceType_void:
  case IceType_NUM:
    break;
  }
  llvm_unreachable("Unknown type");
}

ConstantList GlobalContext::getConstantExternSyms() {
  return getConstPool()->ExternRelocatables.getConstantPool();
}

GlobalString GlobalContext::getGlobalString(const std::string &Name) {
  return GlobalString::createWithString(this, Name);
}

JumpTableDataList GlobalContext::getJumpTables() {
  JumpTableDataList JumpTables(*getJumpTableList());
  // Make order deterministic by sorting into functions and then ID of the jump
  // table within that function.
  std::sort(JumpTables.begin(), JumpTables.end(),
            [](const JumpTableData &A, const JumpTableData &B) {
              if (A.getFunctionName() != B.getFunctionName())
                return A.getFunctionName() < B.getFunctionName();
              return A.getId() < B.getId();
            });

  return JumpTables;
}

void GlobalContext::addJumpTableData(JumpTableData JumpTable) {
  getJumpTableList()->emplace_back(std::move(JumpTable));
}

TimerStackIdT GlobalContext::newTimerStackID(const std::string &Name) {
  if (!BuildDefs::timers())
    return 0;
  auto Timers = getTimers();
  TimerStackIdT NewID = Timers->size();
  Timers->push_back(TimerStack(Name));
  return NewID;
}

TimerIdT GlobalContext::getTimerID(TimerStackIdT StackID,
                                   const std::string &Name) {
  auto *Timers = &ICE_TLS_GET_FIELD(TLS)->Timers;
  assert(StackID < Timers->size());
  return Timers->at(StackID).getTimerID(Name);
}

void GlobalContext::pushTimer(TimerIdT ID, TimerStackIdT StackID) {
  auto *Timers = &ICE_TLS_GET_FIELD(TLS)->Timers;
  assert(StackID < Timers->size());
  Timers->at(StackID).push(ID);
}

void GlobalContext::popTimer(TimerIdT ID, TimerStackIdT StackID) {
  auto *Timers = &ICE_TLS_GET_FIELD(TLS)->Timers;
  assert(StackID < Timers->size());
  Timers->at(StackID).pop(ID);
}

void GlobalContext::resetTimer(TimerStackIdT StackID) {
  auto *Timers = &ICE_TLS_GET_FIELD(TLS)->Timers;
  assert(StackID < Timers->size());
  Timers->at(StackID).reset();
}

std::string GlobalContext::getTimerName(TimerStackIdT StackID) {
  auto *Timers = &ICE_TLS_GET_FIELD(TLS)->Timers;
  assert(StackID < Timers->size());
  return Timers->at(StackID).getName();
}

void GlobalContext::setTimerName(TimerStackIdT StackID,
                                 const std::string &NewName) {
  auto *Timers = &ICE_TLS_GET_FIELD(TLS)->Timers;
  assert(StackID < Timers->size());
  Timers->at(StackID).setName(NewName);
}

// Note: optQueueBlockingPush and optQueueBlockingPop use unique_ptr at the
// interface to take and transfer ownership, but they internally store the raw
// Cfg pointer in the work queue. This allows e.g. future queue optimizations
// such as the use of atomics to modify queue elements.
void GlobalContext::optQueueBlockingPush(std::unique_ptr<OptWorkItem> Item) {
  assert(Item);
  {
    TimerMarker _(TimerStack::TT_qTransPush, this);
    OptQ.blockingPush(std::move(Item));
  }
  if (getFlags().isSequential())
    translateFunctions();
}

std::unique_ptr<OptWorkItem> GlobalContext::optQueueBlockingPop() {
  TimerMarker _(TimerStack::TT_qTransPop, this);
  return OptQ.blockingPop(OptQWakeupSize);
}

void GlobalContext::emitQueueBlockingPush(
    std::unique_ptr<EmitterWorkItem> Item) {
  assert(Item);
  {
    TimerMarker _(TimerStack::TT_qEmitPush, this);
    EmitQ.blockingPush(std::move(Item));
  }
  if (getFlags().isSequential())
    emitItems();
}

std::unique_ptr<EmitterWorkItem> GlobalContext::emitQueueBlockingPop() {
  TimerMarker _(TimerStack::TT_qEmitPop, this);
  return EmitQ.blockingPop();
}

void GlobalContext::initParserThread() {
  ThreadContext *Tls = new ThreadContext();
  auto Timers = getTimers();
  Timers->initInto(Tls->Timers);
  AllThreadContexts.push_back(Tls);
  ICE_TLS_SET_FIELD(TLS, Tls);
}

void GlobalContext::startWorkerThreads() {
  size_t NumWorkers = getFlags().getNumTranslationThreads();
  auto Timers = getTimers();
  for (size_t i = 0; i < NumWorkers; ++i) {
    ThreadContext *WorkerTLS = new ThreadContext();
    Timers->initInto(WorkerTLS->Timers);
    AllThreadContexts.push_back(WorkerTLS);
    TranslationThreads.push_back(std::thread(
        &GlobalContext::translateFunctionsWrapper, this, WorkerTLS));
  }
  if (NumWorkers) {
    ThreadContext *WorkerTLS = new ThreadContext();
    Timers->initInto(WorkerTLS->Timers);
    AllThreadContexts.push_back(WorkerTLS);
    EmitterThreads.push_back(
        std::thread(&GlobalContext::emitterWrapper, this, WorkerTLS));
  }
}

void GlobalContext::resetStats() {
  if (BuildDefs::dump())
    ICE_TLS_GET_FIELD(TLS)->StatsFunction.reset();
}

void GlobalContext::dumpStats(const Cfg *Func) {
  if (!getFlags().getDumpStats())
    return;
  if (Func == nullptr) {
    getStatsCumulative()->dump(Func, this);
  } else {
    ICE_TLS_GET_FIELD(TLS)->StatsFunction.dump(Func, this);
  }
}

void GlobalContext::statsUpdateEmitted(uint32_t InstCount) {
  if (!getFlags().getDumpStats())
    return;
  ThreadContext *Tls = ICE_TLS_GET_FIELD(TLS);
  Tls->StatsFunction.update(CodeStats::CS_InstCount, InstCount);
  Tls->StatsCumulative.update(CodeStats::CS_InstCount, InstCount);
}

void GlobalContext::statsUpdateRegistersSaved(uint32_t Num) {
  if (!getFlags().getDumpStats())
    return;
  ThreadContext *Tls = ICE_TLS_GET_FIELD(TLS);
  Tls->StatsFunction.update(CodeStats::CS_RegsSaved, Num);
  Tls->StatsCumulative.update(CodeStats::CS_RegsSaved, Num);
}

void GlobalContext::statsUpdateFrameBytes(uint32_t Bytes) {
  if (!getFlags().getDumpStats())
    return;
  ThreadContext *Tls = ICE_TLS_GET_FIELD(TLS);
  Tls->StatsFunction.update(CodeStats::CS_FrameByte, Bytes);
  Tls->StatsCumulative.update(CodeStats::CS_FrameByte, Bytes);
}

void GlobalContext::statsUpdateSpills() {
  if (!getFlags().getDumpStats())
    return;
  ThreadContext *Tls = ICE_TLS_GET_FIELD(TLS);
  Tls->StatsFunction.update(CodeStats::CS_NumSpills);
  Tls->StatsCumulative.update(CodeStats::CS_NumSpills);
}

void GlobalContext::statsUpdateFills() {
  if (!getFlags().getDumpStats())
    return;
  ThreadContext *Tls = ICE_TLS_GET_FIELD(TLS);
  Tls->StatsFunction.update(CodeStats::CS_NumFills);
  Tls->StatsCumulative.update(CodeStats::CS_NumFills);
}

void GlobalContext::statsUpdateRPImms() {
  if (!getFlags().getDumpStats())
    return;
  ThreadContext *Tls = ICE_TLS_GET_FIELD(TLS);
  Tls->StatsFunction.update(CodeStats::CS_NumRPImms);
  Tls->StatsCumulative.update(CodeStats::CS_NumRPImms);
}

void GlobalContext::dumpTimers(TimerStackIdT StackID, bool DumpCumulative) {
  if (!BuildDefs::timers())
    return;
  auto Timers = getTimers();
  assert(Timers->size() > StackID);
  OstreamLocker L(this);
  Timers->at(StackID).dump(getStrDump(), DumpCumulative);
}

void GlobalContext::dumpLocalTimers(const std::string &TimerNameOverride,
                                    TimerStackIdT StackID,
                                    bool DumpCumulative) {
  if (!BuildDefs::timers())
    return;
  auto *Timers = &ICE_TLS_GET_FIELD(TLS)->Timers;
  assert(Timers->size() > StackID);
  // Temporarily override the thread-local timer name with the given name.
  // Don't do it permanently because the final timer merge at the end expects
  // the thread-local timer names to be the same as the global timer name.
  auto OrigName = getTimerName(StackID);
  setTimerName(StackID, TimerNameOverride);
  {
    OstreamLocker _(this);
    Timers->at(StackID).dump(getStrDump(), DumpCumulative);
  }
  setTimerName(StackID, OrigName);
}

LockedPtr<StringPool>
GlobalStringPoolTraits::getStrings(const GlobalContext *PoolOwner) {
  return PoolOwner->getStrings();
}

TimerIdT TimerMarker::getTimerIdFromFuncName(GlobalContext *Ctx,
                                             const std::string &FuncName) {
  if (!BuildDefs::timers())
    return 0;
  if (!getFlags().getTimeEachFunction())
    return 0;
  return Ctx->getTimerID(GlobalContext::TSK_Funcs, FuncName);
}

void TimerMarker::push() {
  switch (StackID) {
  case GlobalContext::TSK_Default:
    Active = getFlags().getSubzeroTimingEnabled() ||
             !getFlags().getTimingFocusOnString().empty();
    break;
  case GlobalContext::TSK_Funcs:
    Active = getFlags().getTimeEachFunction();
    break;
  default:
    break;
  }
  if (Active)
    Ctx->pushTimer(ID, StackID);
}

void TimerMarker::pushCfg(const Cfg *Func) {
  Ctx = Func->getContext();
  Active = Func->getFocusedTiming() || getFlags().getSubzeroTimingEnabled();
  if (Active)
    Ctx->pushTimer(ID, StackID);
}

ICE_TLS_DEFINE_FIELD(GlobalContext::ThreadContext *, GlobalContext, TLS);

} // end of namespace Ice
