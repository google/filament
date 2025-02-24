//===- subzero/src/IceDefs.h - Common Subzero declarations ------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares various useful types and classes that have widespread use
/// across Subzero.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEDEFS_H
#define SUBZERO_SRC_ICEDEFS_H

#include "IceBuildDefs.h" // TODO(stichnot): move into individual files
#include "IceMemory.h"
#include "IceTLS.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/ilist.h"
#include "llvm/ADT/ilist_node.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ELF.h"
#include "llvm/Support/raw_ostream.h"

#include <cassert>
#include <cstdint>
#include <cstdio>     // snprintf
#include <functional> // std::less
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#define XSTRINGIFY(x) STRINGIFY(x)
#define STRINGIFY(x) #x

namespace Ice {

class Assembler;
template <template <typename> class> class BitVectorTmpl;
class Cfg;
class CfgNode;
class Constant;
class ELFFileStreamer;
class ELFObjectWriter;
class ELFStreamer;
class FunctionDeclaration;
class GlobalContext;
class GlobalDeclaration;
class Inst;
class InstAssign;
class InstJumpTable;
class InstPhi;
class InstSwitch;
class InstTarget;
class LiveRange;
class Liveness;
class Operand;
class TargetDataLowering;
class TargetLowering;
class Variable;
class VariableDeclaration;
class VariablesMetadata;

/// SizeT is for holding small-ish limits like number of source operands in an
/// instruction. It is used instead of size_t (which may be 64-bits wide) when
/// we want to save space.
using SizeT = uint32_t;

constexpr char GlobalOffsetTable[] = "_GLOBAL_OFFSET_TABLE_";
// makeUnique should be used when memory is expected to be allocated from the
// heap (as opposed to allocated from some Allocator.) It is intended to be
// used instead of new.
//
// The expected usage is as follows
//
// class MyClass {
// public:
//   static std::unique_ptr<MyClass> create(<ctor_args>) {
//     return makeUnique<MyClass>(<ctor_args>);
//   }
//
// private:
//   ENABLE_MAKE_UNIQUE;
//
//   MyClass(<ctor_args>) ...
// }
//
// ENABLE_MAKE_UNIQUE is a trick that is necessary if MyClass' ctor is private.
// Private ctors are highly encouraged when you're writing a class that you'd
// like to have allocated with makeUnique as it would prevent users from
// declaring stack allocated variables.
namespace Internal {
struct MakeUniqueEnabler {
  template <class T, class... Args>
  static std::unique_ptr<T> create(Args &&... TheArgs) {
    std::unique_ptr<T> Unique(new T(std::forward<Args>(TheArgs)...));
    return Unique;
  }
};
} // end of namespace Internal

template <class T, class... Args>
static std::unique_ptr<T> makeUnique(Args &&... TheArgs) {
  return ::Ice::Internal::MakeUniqueEnabler::create<T>(
      std::forward<Args>(TheArgs)...);
}

#define ENABLE_MAKE_UNIQUE friend struct ::Ice::Internal::MakeUniqueEnabler

using InstList = llvm::ilist<Inst>;
// Ideally PhiList would be llvm::ilist<InstPhi>, and similar for AssignList,
// but this runs into issues with SFINAE.
using PhiList = InstList;
using AssignList = InstList;

// Standard library containers with CfgLocalAllocator.
template <typename T> using CfgList = std::list<T, CfgLocalAllocator<T>>;
template <typename T, typename H = std::hash<T>, typename Eq = std::equal_to<T>>
using CfgUnorderedSet = std::unordered_set<T, H, Eq, CfgLocalAllocator<T>>;
template <typename T, typename Cmp = std::less<T>>
using CfgSet = std::set<T, Cmp, CfgLocalAllocator<T>>;
template <typename T, typename U, typename H = std::hash<T>,
          typename Eq = std::equal_to<T>>
using CfgUnorderedMap =
    std::unordered_map<T, U, H, Eq, CfgLocalAllocator<std::pair<const T, U>>>;
template <typename T> using CfgVector = std::vector<T, CfgLocalAllocator<T>>;

// Containers that are arena-allocated from the Cfg's allocator.
using OperandList = CfgVector<Operand *>;
using VarList = CfgVector<Variable *>;
using NodeList = CfgVector<CfgNode *>;

// Containers that use the default (global) allocator.
using ConstantList = std::vector<Constant *>;
using FunctionDeclarationList = std::vector<FunctionDeclaration *>;

/// VariableDeclarationList is a container for holding VariableDeclarations --
/// i.e., Global Variables. It is also used to create said variables, and their
/// initializers in an arena.
class VariableDeclarationList {
  VariableDeclarationList(const VariableDeclarationList &) = delete;
  VariableDeclarationList &operator=(const VariableDeclarationList &) = delete;
  VariableDeclarationList(VariableDeclarationList &&) = delete;
  VariableDeclarationList &operator=(VariableDeclarationList &&) = delete;

public:
  using VariableDeclarationArray = std::vector<VariableDeclaration *>;

  VariableDeclarationList() : Arena(new ArenaAllocator()) {}

  ~VariableDeclarationList() { clearAndPurge(); }

  template <typename T> T *allocate_initializer(SizeT Count = 1) {
    static_assert(
        std::is_trivially_destructible<T>::value,
        "allocate_initializer can only allocate trivially destructible types.");
    return Arena->Allocate<T>(Count);
  }

  template <typename T> T *allocate_variable_declaration() {
    static_assert(!std::is_trivially_destructible<T>::value,
                  "allocate_variable_declaration expects non-trivially "
                  "destructible types.");
    T *Ret = Arena->Allocate<T>();
    Dtors.emplace_back([Ret]() { Ret->~T(); });
    return Ret;
  }

  // This do nothing method is invoked when a global variable is created, but it
  // will not be emitted. If we ever need to track the created variable, having
  // this hook is handy.
  void willNotBeEmitted(VariableDeclaration *) {}

  /// Merges Other with this, effectively resetting Other to an empty state.
  void merge(VariableDeclarationList *Other) {
    assert(Other != nullptr);
    addArena(std::move(Other->Arena));
    for (std::size_t i = 0; i < Other->MergedArenas.size(); ++i) {
      addArena(std::move(Other->MergedArenas[i]));
    }
    Other->MergedArenas.clear();

    Dtors.insert(Dtors.end(), Other->Dtors.begin(), Other->Dtors.end());
    Other->Dtors.clear();

    Globals.insert(Globals.end(), Other->Globals.begin(), Other->Globals.end());
    Other->Globals.clear();
  }

  /// Destroys all GlobalVariables and initializers that this knows about
  /// (including those merged with it), and releases memory.
  void clearAndPurge() {
    if (Arena == nullptr) {
      // Arena is only null if this was merged, so we ensure there's no state
      // being held by this.
      assert(Dtors.empty());
      assert(Globals.empty());
      assert(MergedArenas.empty());
      return;
    }
    // Invokes destructors in reverse creation order.
    for (auto Dtor = Dtors.rbegin(); Dtor != Dtors.rend(); ++Dtor) {
      (*Dtor)();
    }
    Dtors.clear();
    Globals.clear();
    MergedArenas.clear();
    Arena->Reset();
  }

  /// Adapt the relevant parts of the std::vector<VariableDeclaration *>
  /// interface.
  /// @{
  VariableDeclarationArray::iterator begin() { return Globals.begin(); }

  VariableDeclarationArray::iterator end() { return Globals.end(); }

  VariableDeclarationArray::const_iterator begin() const {
    return Globals.begin();
  }

  VariableDeclarationArray::const_iterator end() const { return Globals.end(); }

  bool empty() const { return Globals.empty(); }

  VariableDeclarationArray::size_type size() const { return Globals.size(); }

  VariableDeclarationArray::reference
  at(VariableDeclarationArray::size_type Pos) {
    return Globals.at(Pos);
  }

  void push_back(VariableDeclaration *Global) { Globals.push_back(Global); }

  void reserve(VariableDeclarationArray::size_type Capacity) {
    Globals.reserve(Capacity);
  }

  void clear() { Globals.clear(); }

  VariableDeclarationArray::reference back() { return Globals.back(); }
  /// @}

private:
  using ArenaPtr = std::unique_ptr<ArenaAllocator>;
  using DestructorsArray = std::vector<std::function<void()>>;

  void addArena(ArenaPtr NewArena) {
    MergedArenas.emplace_back(std::move(NewArena));
  }

  ArenaPtr Arena;
  VariableDeclarationArray Globals;
  DestructorsArray Dtors;
  std::vector<ArenaPtr> MergedArenas;
};

/// InstNumberT is for holding an instruction number. Instruction numbers are
/// used for representing Variable live ranges.
using InstNumberT = int32_t;

/// A LiveBeginEndMapEntry maps a Variable::Number value to an Inst::Number
/// value, giving the instruction number that begins or ends a variable's live
/// range.
template <typename T>
using LivenessVector = std::vector<T, LivenessAllocator<T>>;
using LiveBeginEndMapEntry = std::pair<SizeT, InstNumberT>;
using LiveBeginEndMap = LivenessVector<LiveBeginEndMapEntry>;
using LivenessBV = BitVectorTmpl<LivenessAllocator>;

using TimerStackIdT = uint32_t;
using TimerIdT = uint32_t;

/// Use alignas(MaxCacheLineSize) to isolate variables/fields that might be
/// contended while multithreading. Assumes the maximum cache line size is 64.
enum { MaxCacheLineSize = 64 };
// Use ICE_CACHELINE_BOUNDARY to force the next field in a declaration
// list to be aligned to the next cache line.
#if defined(_MSC_VER)
#define ICE_CACHELINE_BOUNDARY __declspec(align(MaxCacheLineSize)) int : 0;
#else // !defined(_MSC_VER)
// Note: zero is added to work around the following GCC 4.8 bug (fixed in 4.9):
//       https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55382
#define ICE_CACHELINE_BOUNDARY                                                 \
  __attribute__((aligned(MaxCacheLineSize + 0))) int : 0
#endif // !defined(_MSC_VER)

using RelocOffsetT = int32_t;
enum { RelocAddrSize = 4 };

enum LivenessMode {
  /// Basic version of live-range-end calculation. Marks the last uses of
  /// variables based on dataflow analysis. Records the set of live-in and
  /// live-out variables for each block. Identifies and deletes dead
  /// instructions (primarily stores).
  Liveness_Basic,

  /// In addition to Liveness_Basic, also calculate the complete live range for
  /// each variable in a form suitable for interference calculation and register
  /// allocation.
  Liveness_Intervals
};

enum LCSEOptions {
  LCSE_Disabled,
  LCSE_EnabledSSA,  // Default Mode, assumes SSA.
  LCSE_EnabledNoSSA // Does not assume SSA, to be enabled if CSE is done later.
};

enum RegAllocKind {
  RAK_Unknown,
  RAK_Global,       /// full, global register allocation
  RAK_SecondChance, /// second-chance bin-packing after full regalloc attempt
  RAK_Phi,          /// infinite-weight Variables with active spilling/filling
  RAK_InfOnly       /// allocation only for infinite-weight Variables
};

enum VerboseItem {
  IceV_None = 0,
  IceV_Instructions = 1 << 0,
  IceV_Deleted = 1 << 1,
  IceV_InstNumbers = 1 << 2,
  IceV_Preds = 1 << 3,
  IceV_Succs = 1 << 4,
  IceV_Liveness = 1 << 5,
  IceV_RegOrigins = 1 << 6,
  IceV_LinearScan = 1 << 7,
  IceV_Frame = 1 << 8,
  IceV_AddrOpt = 1 << 9,
  IceV_Folding = 1 << 10,
  IceV_RMW = 1 << 11,
  IceV_Loop = 1 << 12,
  IceV_Mem = 1 << 13,
  // Leave some extra space to make it easier to add new per-pass items.
  IceV_NO_PER_PASS_DUMP_BEYOND = 1 << 19,
  // Items greater than IceV_NO_PER_PASS_DUMP_BEYOND don't by themselves trigger
  // per-pass Cfg dump output.
  IceV_Status = 1 << 20,
  IceV_AvailableRegs = 1 << 21,
  IceV_GlobalInit = 1 << 22,
  IceV_ConstPoolStats = 1 << 23,
  IceV_Wasm = 1 << 24,
  IceV_ShufMat = 1 << 25,
  IceV_All = ~IceV_None,
  IceV_Most =
      IceV_All & ~IceV_LinearScan & ~IceV_GlobalInit & ~IceV_ConstPoolStats
};
using VerboseMask = uint32_t;

enum FileType {
  FT_Elf, /// ELF .o file
  FT_Asm, /// Assembly .s file
  FT_Iasm /// "Integrated assembler" .byte-style .s file
};

using Ostream = llvm::raw_ostream;
using Fdstream = llvm::raw_fd_ostream;

using GlobalLockType = std::mutex;

/// LockedPtr is an RAII wrapper that allows automatically locked access to a
/// given pointer, automatically unlocking it when when the LockedPtr goes out
/// of scope.
template <typename T> class LockedPtr {
  LockedPtr() = delete;
  LockedPtr(const LockedPtr &) = delete;
  LockedPtr &operator=(const LockedPtr &) = delete;

public:
  LockedPtr(T *Value, GlobalLockType *Lock) : Value(Value), Lock(Lock) {
    Lock->lock();
  }
  LockedPtr(LockedPtr &&Other) : Value(Other.Value), Lock(Other.Lock) {
    Other.Value = nullptr;
    Other.Lock = nullptr;
  }
  ~LockedPtr() {
    if (Lock != nullptr)
      Lock->unlock();
  }
  T *operator->() const { return Value; }
  T &operator*() const { return *Value; }
  T *get() { return Value; }

private:
  T *Value;
  GlobalLockType *Lock;
};

enum ErrorCodes { EC_None = 0, EC_Args, EC_Bitcode, EC_Translation };

/// Wrapper around std::error_code for allowing multiple errors to be folded
/// into one. The current implementation keeps track of the first error, which
/// is likely to be the most useful one, and this could be extended to e.g.
/// collect a vector of errors.
class ErrorCode : public std::error_code {
  ErrorCode(const ErrorCode &) = delete;
  ErrorCode &operator=(const ErrorCode &) = delete;

public:
  ErrorCode() = default;
  void assign(ErrorCodes Code) {
    if (!HasError) {
      HasError = true;
      std::error_code::assign(Code, std::generic_category());
    }
  }
  void assign(int Code) { assign(static_cast<ErrorCodes>(Code)); }

private:
  bool HasError = false;
};

/// Reverse range adaptors written in terms of llvm::make_range().
template <typename T>
llvm::iterator_range<typename T::const_reverse_iterator>
reverse_range(const T &Container) {
  return llvm::make_range(Container.rbegin(), Container.rend());
}
template <typename T>
llvm::iterator_range<typename T::reverse_iterator> reverse_range(T &Container) {
  return llvm::make_range(Container.rbegin(), Container.rend());
}

using RelocOffsetArray = llvm::SmallVector<class RelocOffset *, 4>;

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEDEFS_H
