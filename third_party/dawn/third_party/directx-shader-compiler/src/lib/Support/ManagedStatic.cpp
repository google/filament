//===-- ManagedStatic.cpp - Static Global wrapper -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the ManagedStatic class and llvm_shutdown().
//
//===----------------------------------------------------------------------===//
//
// The following changes have been backported from upstream llvm:
//
// 56349e8b6d85 Fix for memory leak reported by Valgrind
// 928071ae4ef5 [Support] Replace sys::Mutex with their standard equivalents.
// 3d5360a4398b Replace llvm::MutexGuard/UniqueLock with their standard equivalents
// c06a470fc843 Try once more to ensure constant initializaton of ManagedStatics
// 41fe3a54c261 Ensure that ManagedStatic is constant initialized in MSVC 2017 & 2019
// 74de08031f5d [ManagedStatic] Avoid putting function pointers in template args.
// f65e4ce2c48a [ADT, Support, TableGen] Fix some Clang-tidy modernize-use-default and Include What You Use warnings; other minor fixes (NFC).
// 832d04207810 [ManagedStatic] Reimplement double-checked locking with std::atomic.
// dd1463823a0c This is yet another attempt to re-instate r220932 as discussed in D19271.
//

#include "llvm/Support/ManagedStatic.h"
#include "llvm/Config/config.h"
#include "llvm/Support/Threading.h"
#include <cassert>
#include <mutex>
using namespace llvm;

static const ManagedStaticBase *StaticList = nullptr;

static std::recursive_mutex *getManagedStaticMutex() {
  static std::recursive_mutex m;
  return &m;
}

void ManagedStaticBase::RegisterManagedStatic(void *(*Creator)(),
                                              void (*Deleter)(void*)) const {
  assert(Creator);
  if (llvm_is_multithreaded()) {
    std::lock_guard<std::recursive_mutex> Lock(*getManagedStaticMutex());

    if (!Ptr.load(std::memory_order_relaxed)) {
      void *Tmp = Creator();

      Ptr.store(Tmp, std::memory_order_release);
      DeleterFn = Deleter;

      // Add to list of managed statics.
      Next = StaticList;
      StaticList = this;
    }
  } else {
    assert(!Ptr && !DeleterFn && !Next &&
           "Partially initialized ManagedStatic!?");
    Ptr = Creator();
    DeleterFn = Deleter;

    // Add to list of managed statics.
    Next = StaticList;
    StaticList = this;
  }
}

void ManagedStaticBase::destroy() const {
  assert(DeleterFn && "ManagedStatic not initialized correctly!");
  assert(StaticList == this &&
         "Not destroyed in reverse order of construction?");
  // Unlink from list.
  StaticList = Next;
  Next = nullptr;

  // Destroy memory.
  DeleterFn(Ptr);

  // Cleanup.
  Ptr = nullptr;
  DeleterFn = nullptr;
}

/// llvm_shutdown - Deallocate and destroy all ManagedStatic variables.
/// IMPORTANT: it's only safe to call llvm_shutdown() in single thread,
/// without any other threads executing LLVM APIs.
/// llvm_shutdown() should be the last use of LLVM APIs.
void llvm::llvm_shutdown() {
  while (StaticList)
    StaticList->destroy();
}
