//===- subzero/src/IceGlobalContext.h - Global context defs -----*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares aspects of the compilation that persist across multiple
/// functions.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEGLOBALCONTEXT_H
#define SUBZERO_SRC_ICEGLOBALCONTEXT_H

#include "IceClFlags.h"
#include "IceDefs.h"
#include "IceInstrumentation.h"
#include "IceIntrinsics.h"
#include "IceStringPool.h"
#include "IceSwitchLowering.h"
#include "IceTargetLowering.def"
#include "IceThreading.h"
#include "IceTimerTree.h"
#include "IceTypes.h"
#include "IceUtils.h"

#include <array>
#include <atomic>
#include <cassert>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace Ice {

class ConstantPool;
class EmitterWorkItem;
class FuncSigType;
class Instrumentation;

// Runtime helper function IDs

enum class RuntimeHelper {
#define X(Tag, Name) H_##Tag,
  RUNTIME_HELPER_FUNCTIONS_TABLE
#undef X
      H_Num
};

/// OptWorkItem is a simple wrapper used to pass parse information on a function
/// block, to a translator thread.
class OptWorkItem {
  OptWorkItem(const OptWorkItem &) = delete;
  OptWorkItem &operator=(const OptWorkItem &) = delete;

public:
  // Get the Cfg for the funtion to translate.
  virtual std::unique_ptr<Cfg> getParsedCfg() = 0;
  virtual ~OptWorkItem() = default;

protected:
  OptWorkItem() = default;
};

class GlobalContext {
  GlobalContext() = delete;
  GlobalContext(const GlobalContext &) = delete;
  GlobalContext &operator=(const GlobalContext &) = delete;

  /// CodeStats collects rudimentary statistics during translation.
  class CodeStats {
    CodeStats(const CodeStats &) = delete;
    CodeStats &operator=(const CodeStats &) = default;
#define CODESTATS_TABLE                                                        \
  /* dump string, enum value */                                                \
  X("Inst Count  ", InstCount)                                                 \
  X("Regs Saved  ", RegsSaved)                                                 \
  X("Frame Bytes ", FrameByte)                                                 \
  X("Spills      ", NumSpills)                                                 \
  X("Fills       ", NumFills)                                                  \
  X("R/P Imms    ", NumRPImms)
    //#define X(str, tag)

  public:
    enum CSTag {
#define X(str, tag) CS_##tag,
      CODESTATS_TABLE
#undef X
          CS_NUM
    };
    CodeStats() { reset(); }
    void reset() { Stats.fill(0); }
    void update(CSTag Tag, uint32_t Count = 1) {
      assert(static_cast<SizeT>(Tag) < Stats.size());
      Stats[Tag] += Count;
    }
    void add(const CodeStats &Other) {
      for (uint32_t i = 0; i < Stats.size(); ++i)
        Stats[i] += Other.Stats[i];
    }
    /// Dumps the stats for the given Cfg.  If Func==nullptr, it identifies it
    /// as the "final" cumulative stats instead as a specific function's name.
    void dump(const Cfg *Func, GlobalContext *Ctx);

  private:
    std::array<uint32_t, CS_NUM> Stats;
  };

  /// TimerList is a vector of TimerStack objects, with extra methods
  /// to initialize and merge these vectors.
  class TimerList : public std::vector<TimerStack> {
    TimerList(const TimerList &) = delete;
    TimerList &operator=(const TimerList &) = delete;

  public:
    TimerList() = default;
    /// initInto() initializes a target list of timers based on the
    /// current list.  In particular, it creates the same number of
    /// timers, in the same order, with the same names, but initially
    /// empty of timing data.
    void initInto(TimerList &Dest) const {
      if (!BuildDefs::timers())
        return;
      Dest.clear();
      for (const TimerStack &Stack : *this) {
        Dest.push_back(TimerStack(Stack.getName()));
      }
    }
    void mergeFrom(TimerList &Src) {
      if (!BuildDefs::timers())
        return;
      assert(size() == Src.size());
      size_type i = 0;
      for (TimerStack &Stack : *this) {
        assert(Stack.getName() == Src[i].getName());
        Stack.mergeFrom(Src[i]);
        ++i;
      }
    }
  };

  /// ThreadContext contains thread-local data.  This data can be
  /// combined/reduced as needed after all threads complete.
  class ThreadContext {
    ThreadContext(const ThreadContext &) = delete;
    ThreadContext &operator=(const ThreadContext &) = delete;

  public:
    ThreadContext() = default;
    CodeStats StatsFunction;
    CodeStats StatsCumulative;
    TimerList Timers;
  };

public:
  /// The dump stream is a log stream while emit is the stream code
  /// is emitted to. The error stream is strictly for logging errors.
  GlobalContext(Ostream *OsDump, Ostream *OsEmit, Ostream *OsError,
                ELFStreamer *ELFStreamer);
  ~GlobalContext();

  void dumpStrings();
  ///
  /// The dump, error, and emit streams need to be used by only one
  /// thread at a time.  This is done by exclusively reserving the
  /// streams via lockStr() and unlockStr().  The OstreamLocker class
  /// can be used to conveniently manage this.
  ///
  /// The model is that a thread grabs the stream lock, then does an
  /// arbitrary amount of work during which far-away callees may grab
  /// the stream and do something with it, and finally the thread
  /// releases the stream lock.  This allows large chunks of output to
  /// be dumped or emitted without risking interleaving from multiple
  /// threads.
  void lockStr() { StrLock.lock(); }
  void unlockStr() { StrLock.unlock(); }
  Ostream &getStrDump() { return *StrDump; }
  Ostream &getStrError() { return *StrError; }
  Ostream &getStrEmit() { return *StrEmit; }
  void setStrEmit(Ostream &NewStrEmit) { StrEmit = &NewStrEmit; }

  LockedPtr<ErrorCode> getErrorStatus() {
    return LockedPtr<ErrorCode>(&ErrorStatus, &ErrorStatusLock);
  }

  /// \name Manage Constants.
  /// @{
  // getConstant*() functions are not const because they might add something to
  // the constant pool.
  Constant *getConstantInt(Type Ty, int64_t Value);
  Constant *getConstantInt1(int8_t ConstantInt1) {
    ConstantInt1 &= INT8_C(1);
    switch (ConstantInt1) {
    case 0:
      return getConstantZero(IceType_i1);
    case 1:
      return ConstantTrue;
    default:
      assert(false && "getConstantInt1 not on true/false");
      return getConstantInt1Internal(ConstantInt1);
    }
  }
  Constant *getConstantInt8(int8_t ConstantInt8) {
    switch (ConstantInt8) {
    case 0:
      return getConstantZero(IceType_i8);
    default:
      return getConstantInt8Internal(ConstantInt8);
    }
  }
  Constant *getConstantInt16(int16_t ConstantInt16) {
    switch (ConstantInt16) {
    case 0:
      return getConstantZero(IceType_i16);
    default:
      return getConstantInt16Internal(ConstantInt16);
    }
  }
  Constant *getConstantInt32(int32_t ConstantInt32) {
    switch (ConstantInt32) {
    case 0:
      return getConstantZero(IceType_i32);
    default:
      return getConstantInt32Internal(ConstantInt32);
    }
  }
  Constant *getConstantInt64(int64_t ConstantInt64) {
    switch (ConstantInt64) {
    case 0:
      return getConstantZero(IceType_i64);
    default:
      return getConstantInt64Internal(ConstantInt64);
    }
  }
  Constant *getConstantFloat(float Value);
  Constant *getConstantDouble(double Value);
  /// Returns a symbolic constant.
  Constant *getConstantSymWithEmitString(const RelocOffsetT Offset,
                                         const RelocOffsetArray &OffsetExpr,
                                         GlobalString Name,
                                         const std::string &EmitString);
  Constant *getConstantSym(RelocOffsetT Offset, GlobalString Name);
  Constant *getConstantExternSym(GlobalString Name);
  /// Returns an undef.
  Constant *getConstantUndef(Type Ty);
  /// Returns a zero value.
  Constant *getConstantZero(Type Ty);
  /// getConstantPool() returns a copy of the constant pool for constants of a
  /// given type.
  ConstantList getConstantPool(Type Ty);
  /// Returns a copy of the list of external symbols.
  ConstantList getConstantExternSyms();
  /// @}
  Constant *getRuntimeHelperFunc(RuntimeHelper FuncID) const {
    assert(FuncID < RuntimeHelper::H_Num);
    Constant *Result = RuntimeHelperFunc[static_cast<size_t>(FuncID)];
    assert(Result != nullptr && "No such runtime helper function");
    return Result;
  }
  GlobalString getGlobalString(const std::string &Name);

  /// Return a locked pointer to the registered jump tables.
  JumpTableDataList getJumpTables();
  /// Adds JumpTable to the list of know jump tables, for a posteriori emission.
  void addJumpTableData(JumpTableData JumpTable);

  /// Allocate data of type T using the global allocator. We allow entities
  /// allocated from this global allocator to be either trivially or
  /// non-trivially destructible. We optimize the case when T is trivially
  /// destructible by not registering a destructor. Destructors will be invoked
  /// during GlobalContext destruction in the reverse object creation order.
  template <typename T>
  typename std::enable_if<std::is_trivially_destructible<T>::value, T>::type *
  allocate() {
    return getAllocator()->Allocate<T>();
  }

  template <typename T>
  typename std::enable_if<!std::is_trivially_destructible<T>::value, T>::type *
  allocate() {
    T *Ret = getAllocator()->Allocate<T>();
    getDestructors()->emplace_back([Ret]() { Ret->~T(); });
    return Ret;
  }

  ELFObjectWriter *getObjectWriter() const { return ObjectWriter.get(); }

  /// Reset stats at the beginning of a function.
  void resetStats();
  void dumpStats(const Cfg *Func = nullptr);
  void statsUpdateEmitted(uint32_t InstCount);
  void statsUpdateRegistersSaved(uint32_t Num);
  void statsUpdateFrameBytes(uint32_t Bytes);
  void statsUpdateSpills();
  void statsUpdateFills();

  /// Number of Randomized or Pooled Immediates
  void statsUpdateRPImms();

  /// These are predefined TimerStackIdT values.
  enum TimerStackKind { TSK_Default = 0, TSK_Funcs, TSK_Num };

  /// newTimerStackID() creates a new TimerStack in the global space. It does
  /// not affect any TimerStack objects in TLS.
  TimerStackIdT newTimerStackID(const std::string &Name);
  /// dumpTimers() dumps the global timer data.  This assumes all the
  /// thread-local copies of timer data have been merged into the global timer
  /// data.
  void dumpTimers(TimerStackIdT StackID = TSK_Default,
                  bool DumpCumulative = true);
  void dumpLocalTimers(const std::string &TimerNameOverride,
                       TimerStackIdT StackID = TSK_Default,
                       bool DumpCumulative = true);
  /// The following methods affect only the calling thread's TLS timer data.
  TimerIdT getTimerID(TimerStackIdT StackID, const std::string &Name);
  void pushTimer(TimerIdT ID, TimerStackIdT StackID);
  void popTimer(TimerIdT ID, TimerStackIdT StackID);
  void resetTimer(TimerStackIdT StackID);
  std::string getTimerName(TimerStackIdT StackID);
  void setTimerName(TimerStackIdT StackID, const std::string &NewName);

  /// This is the first work item sequence number that the parser produces, and
  /// correspondingly the first sequence number that the emitter thread will
  /// wait for. Start numbering at 1 to leave room for a sentinel, in case e.g.
  /// we wish to inject items with a special sequence number that may be
  /// executed out of order.
  static constexpr uint32_t getFirstSequenceNumber() { return 1; }
  /// Adds a newly parsed and constructed function to the Cfg work queue.
  /// Notifies any idle workers that a new function is available for
  /// translating. May block if the work queue is too large, in order to control
  /// memory footprint.
  void optQueueBlockingPush(std::unique_ptr<OptWorkItem> Item);
  /// Takes a Cfg from the work queue for translating. May block if the work
  /// queue is currently empty. Returns nullptr if there is no more work - the
  /// queue is empty and either end() has been called or the Sequential flag was
  /// set.
  std::unique_ptr<OptWorkItem> optQueueBlockingPop();
  /// Notifies that no more work will be added to the work queue.
  void optQueueNotifyEnd() { OptQ.notifyEnd(); }

  /// Emit file header for output file.
  void emitFileHeader();

  void lowerConstants();

  void lowerJumpTables();

  /// Emit target specific read-only data sections if any. E.g., for MIPS this
  /// generates a .MIPS.abiflags section.
  void emitTargetRODataSections();

  void emitQueueBlockingPush(std::unique_ptr<EmitterWorkItem> Item);
  std::unique_ptr<EmitterWorkItem> emitQueueBlockingPop();
  void emitQueueNotifyEnd() { EmitQ.notifyEnd(); }

  void initParserThread();
  void startWorkerThreads();

  void waitForWorkerThreads();

  /// sets the instrumentation object to use.
  void setInstrumentation(std::unique_ptr<Instrumentation> Instr) {
    if (!BuildDefs::minimal())
      Instrumentor = std::move(Instr);
  }

  void instrumentFunc(Cfg *Func) {
    if (!BuildDefs::minimal() && Instrumentor)
      Instrumentor->instrumentFunc(Func);
  }

  /// Translation thread startup routine.
  void translateFunctionsWrapper(ThreadContext *MyTLS);
  /// Translate functions from the Cfg queue until the queue is empty.
  void translateFunctions();

  /// Emitter thread startup routine.
  void emitterWrapper(ThreadContext *MyTLS);
  /// Emit functions and global initializers from the emitter queue until the
  /// queue is empty.
  void emitItems();

  /// Uses DataLowering to lower Globals. Side effects:
  ///  - discards the initializer list for the global variable in Globals.
  ///  - clears the Globals array.
  void lowerGlobals(const std::string &SectionSuffix);

  void dumpConstantLookupCounts();

  /// DisposeGlobalVariablesAfterLowering controls whether the memory used by
  /// GlobaleVariables can be reclaimed right after they have been lowered.
  /// @{
  bool getDisposeGlobalVariablesAfterLowering() const {
    return DisposeGlobalVariablesAfterLowering;
  }

  void setDisposeGlobalVariablesAfterLowering(bool Value) {
    DisposeGlobalVariablesAfterLowering = Value;
  }
  /// @}

  LockedPtr<StringPool> getStrings() const {
    return LockedPtr<StringPool>(Strings.get(), &StringsLock);
  }

  LockedPtr<VariableDeclarationList> getGlobals() {
    return LockedPtr<VariableDeclarationList>(&Globals, &InitAllocLock);
  }

  /// Number of function blocks that can be queued before waiting for
  /// translation
  /// threads to consume.
  static constexpr size_t MaxOptQSize = 1 << 16;

private:
  // Try to ensure mutexes are allocated on separate cache lines.

  // Destructors collaborate with Allocator
  ICE_CACHELINE_BOUNDARY;
  // Managed by getAllocator()
  mutable GlobalLockType AllocLock;
  ArenaAllocator Allocator;

  ICE_CACHELINE_BOUNDARY;
  // Managed by getInitializerAllocator()
  mutable GlobalLockType InitAllocLock;
  VariableDeclarationList Globals;

  ICE_CACHELINE_BOUNDARY;
  // Managed by getDestructors()
  using DestructorArray = std::vector<std::function<void()>>;
  mutable GlobalLockType DestructorsLock;
  DestructorArray Destructors;

  ICE_CACHELINE_BOUNDARY;
  // Managed by getStrings()
  mutable GlobalLockType StringsLock;
  std::unique_ptr<StringPool> Strings;

  ICE_CACHELINE_BOUNDARY;
  // Managed by getConstPool()
  mutable GlobalLockType ConstPoolLock;
  std::unique_ptr<ConstantPool> ConstPool;

  ICE_CACHELINE_BOUNDARY;
  // Managed by getJumpTableList()
  mutable GlobalLockType JumpTablesLock;
  JumpTableDataList JumpTableList;

  ICE_CACHELINE_BOUNDARY;
  // Managed by getErrorStatus()
  mutable GlobalLockType ErrorStatusLock;
  ErrorCode ErrorStatus;

  ICE_CACHELINE_BOUNDARY;
  // Managed by getStatsCumulative()
  mutable GlobalLockType StatsLock;
  CodeStats StatsCumulative;

  ICE_CACHELINE_BOUNDARY;
  // Managed by getTimers()
  mutable GlobalLockType TimerLock;
  TimerList Timers;

  ICE_CACHELINE_BOUNDARY;
  /// StrLock is a global lock on the dump and emit output streams.
  using StrLockType = std::mutex;
  StrLockType StrLock;
  Ostream *StrDump;  /// Stream for dumping / diagnostics
  Ostream *StrEmit;  /// Stream for code emission
  Ostream *StrError; /// Stream for logging errors.

  // True if waitForWorkerThreads() has been called.
  std::atomic_bool WaitForWorkerThreadsCalled;

  ICE_CACHELINE_BOUNDARY;

  // TODO(jpp): move to EmitterContext.
  std::unique_ptr<ELFObjectWriter> ObjectWriter;
  // Value defining when to wake up the main parse thread.
  const size_t OptQWakeupSize;
  BoundedProducerConsumerQueue<OptWorkItem, MaxOptQSize> OptQ;
  BoundedProducerConsumerQueue<EmitterWorkItem> EmitQ;
  // DataLowering is only ever used by a single thread at a time (either in
  // emitItems(), or in IceCompiler::run before the compilation is over.)
  // TODO(jpp): move to EmitterContext.
  std::unique_ptr<TargetDataLowering> DataLowering;
  /// If !HasEmittedCode, SubZero will accumulate all Globals (which are "true"
  /// program global variables) until the first code WorkItem is seen.
  // TODO(jpp): move to EmitterContext.
  bool HasSeenCode = false;
  // If Instrumentor is not empty then it will be used to instrument globals and
  // CFGs.
  std::unique_ptr<Instrumentation> Instrumentor = nullptr;
  /// Indicates if global variable declarations can be disposed of right after
  /// lowering.
  bool DisposeGlobalVariablesAfterLowering = true;
  Constant *ConstZeroForType[IceType_NUM];
  Constant *ConstantTrue;
  // Holds the constants representing each runtime helper function.
  Constant *RuntimeHelperFunc[static_cast<size_t>(RuntimeHelper::H_Num)];

  Constant *getConstantZeroInternal(Type Ty);
  Constant *getConstantIntInternal(Type Ty, int64_t Value);
  Constant *getConstantInt1Internal(int8_t ConstantInt1);
  Constant *getConstantInt8Internal(int8_t ConstantInt8);
  Constant *getConstantInt16Internal(int16_t ConstantInt16);
  Constant *getConstantInt32Internal(int32_t ConstantInt32);
  Constant *getConstantInt64Internal(int64_t ConstantInt64);
  LockedPtr<ArenaAllocator> getAllocator() {
    return LockedPtr<ArenaAllocator>(&Allocator, &AllocLock);
  }
  LockedPtr<VariableDeclarationList> getInitializerAllocator() {
    return LockedPtr<VariableDeclarationList>(&Globals, &InitAllocLock);
  }
  LockedPtr<ConstantPool> getConstPool() {
    return LockedPtr<ConstantPool>(ConstPool.get(), &ConstPoolLock);
  }
  LockedPtr<JumpTableDataList> getJumpTableList() {
    return LockedPtr<JumpTableDataList>(&JumpTableList, &JumpTablesLock);
  }
  LockedPtr<CodeStats> getStatsCumulative() {
    return LockedPtr<CodeStats>(&StatsCumulative, &StatsLock);
  }
  LockedPtr<TimerList> getTimers() {
    return LockedPtr<TimerList>(&Timers, &TimerLock);
  }
  LockedPtr<DestructorArray> getDestructors() {
    return LockedPtr<DestructorArray>(&Destructors, &DestructorsLock);
  }

  void accumulateGlobals(std::unique_ptr<VariableDeclarationList> Globls) {
    LockedPtr<VariableDeclarationList> _(&Globals, &InitAllocLock);
    if (Globls != nullptr) {
      Globals.merge(Globls.get());
      if (!BuildDefs::minimal() && Instrumentor != nullptr)
        Instrumentor->setHasSeenGlobals();
    }
  }

  void lowerGlobalsIfNoCodeHasBeenSeen() {
    if (HasSeenCode)
      return;
    constexpr char NoSuffix[] = "";
    lowerGlobals(NoSuffix);
    HasSeenCode = true;
  }

  llvm::SmallVector<ThreadContext *, 128> AllThreadContexts;
  llvm::SmallVector<std::thread, 128> TranslationThreads;
  llvm::SmallVector<std::thread, 128> EmitterThreads;
  // Each thread has its own TLS pointer which is also held in
  // AllThreadContexts.
  ICE_TLS_DECLARE_FIELD(ThreadContext *, TLS);

public:
  static void TlsInit();
};

/// Helper class to push and pop a timer marker. The constructor pushes a
/// marker, and the destructor pops it. This is for convenient timing of regions
/// of code.
class TimerMarker {
  TimerMarker() = delete;
  TimerMarker(const TimerMarker &) = delete;
  TimerMarker &operator=(const TimerMarker &) = delete;

public:
  TimerMarker(TimerIdT ID, GlobalContext *Ctx,
              TimerStackIdT StackID = GlobalContext::TSK_Default)
      : ID(ID), Ctx(Ctx), StackID(StackID) {
    if (BuildDefs::timers())
      push();
  }
  TimerMarker(TimerIdT ID, const Cfg *Func,
              TimerStackIdT StackID = GlobalContext::TSK_Default)
      : ID(ID), Ctx(nullptr), StackID(StackID) {
    // Ctx gets set at the beginning of pushCfg().
    if (BuildDefs::timers())
      pushCfg(Func);
  }
  TimerMarker(GlobalContext *Ctx, const std::string &FuncName)
      : ID(getTimerIdFromFuncName(Ctx, FuncName)), Ctx(Ctx),
        StackID(GlobalContext::TSK_Funcs) {
    if (BuildDefs::timers())
      push();
  }

  ~TimerMarker() {
    if (BuildDefs::timers() && Active)
      Ctx->popTimer(ID, StackID);
  }

private:
  void push();
  void pushCfg(const Cfg *Func);
  static TimerIdT getTimerIdFromFuncName(GlobalContext *Ctx,
                                         const std::string &FuncName);
  const TimerIdT ID;
  GlobalContext *Ctx;
  const TimerStackIdT StackID;
  bool Active = false;
};

/// Helper class for locking the streams and then automatically unlocking them.
class OstreamLocker {
private:
  OstreamLocker() = delete;
  OstreamLocker(const OstreamLocker &) = delete;
  OstreamLocker &operator=(const OstreamLocker &) = delete;

public:
  explicit OstreamLocker(GlobalContext *Ctx) : Ctx(Ctx) { Ctx->lockStr(); }
  ~OstreamLocker() { Ctx->unlockStr(); }

private:
  GlobalContext *const Ctx;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEGLOBALCONTEXT_H
