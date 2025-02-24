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

#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Threading.h"
#include <cassert>
#include <mutex>
using namespace llvm;

static const ManagedStaticBase *StaticList = nullptr;
static std::recursive_mutex *ManagedStaticMutex = nullptr;
LLVM_DEFINE_ONCE_FLAG(mutex_init_flag);

#if 0

static void initializeMutex() {
  ManagedStaticMutex = new std::recursive_mutex();
}

static std::recursive_mutex* getManagedStaticMutex() {
  // We need to use a function local static here, since this can get called
  // during a static constructor and we need to guarantee that it's initialized
  // correctly.
  llvm::call_once(mutex_init_flag, initializeMutex);
  return ManagedStaticMutex;
}

#else

// SwiftShader: from https://reviews.llvm.org/D83372
static std::recursive_mutex *getManagedStaticMutex() {
  static std::recursive_mutex m;
  return &m;
}

#endif

void ManagedStaticBase::RegisterManagedStatic(void *(*Creator)(),
                                              void (*Deleter)(void*)) const {
  assert(Creator);
  std::lock_guard<std::recursive_mutex> Lock(*getManagedStaticMutex());

  if (!Ptr.load(std::memory_order_relaxed)) {
    void *Tmp = Creator();

    Ptr.store(Tmp, std::memory_order_release);
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
void llvm::llvm_shutdown() {
  // SwiftShader: from https://reviews.llvm.org/D83372
  // This may be called after the mutex is destroyed. Instead of fixing this,
  // don't bother locking the mutex, and require llvm_shutdown to be called from
  // exactly one thread.
  // std::lock_guard<std::recursive_mutex> Lock(*getManagedStaticMutex());

  while (StaticList)
    StaticList->destroy();
}
