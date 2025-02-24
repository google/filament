//===- subzero/src/LinuxMallocProfiling.cpp - malloc/new tracing  ---------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief malloc/new/...caller tracing.
///
//===----------------------------------------------------------------------===//

#include "LinuxMallocProfiling.h"

#ifdef ALLOW_LINUX_MALLOC_PROFILE

#include <dlfcn.h>
#include <malloc.h>
#include <unordered_map>

extern "C" void *__libc_malloc(size_t size);

namespace {
// The Callers structure allocates memory, which would perturb the tracing.
// InAllocatorFunction is true when we are tracing a new/malloc/...
bool InAllocatorFunction = false;

// Keep track of the number of times a particular call site address invoked an
// allocator. NOTE: this is not thread safe, so the user must invoke with
// --threads=0 to enable profiling.
using MallocMap = std::unordered_map<void *, uint64_t>;
MallocMap *Callers;

void *internalAllocator(size_t size, void *caller) {
  if (Callers != nullptr && !InAllocatorFunction) {
    InAllocatorFunction = true;
    ++(*Callers)[caller];
    InAllocatorFunction = false;
  }
  return __libc_malloc(size);
}
} // end of anonymous namespace

// new, new[], and malloc are all defined as weak symbols to allow them to be
// overridden by user code.  This gives us a convenient place to hook allocation
// tracking, to record the IP of the caller, which we get from the call to
// __builtin_return_address.
void *operator new(size_t size) {
  void *caller = __builtin_return_address(0);
  return internalAllocator(size, caller);
}

void *operator new[](size_t size) {
  void *caller = __builtin_return_address(0);
  return internalAllocator(size, caller);
}

extern "C" void *malloc(size_t size) {
  void *caller = __builtin_return_address(0);
  return internalAllocator(size, caller);
}

namespace Ice {

LinuxMallocProfiling::LinuxMallocProfiling(size_t NumThreads, Ostream *Ls)
    : Ls(Ls) {
  if (NumThreads != 0) {
    *Ls << "NOTE: Malloc profiling is not thread safe. Use --threads=0 to "
           "enable.\n";
    return;
  }
  Callers = new MallocMap();
}

LinuxMallocProfiling::~LinuxMallocProfiling() {
  if (Callers == nullptr) {
    return;
  }
  for (const auto &C : *Callers) {
    Dl_info dli;
    dladdr(C.first, &dli);

    *Ls << C.second << " ";
    if (dli.dli_sname == NULL) {
      *Ls << C.first;
    } else {
      *Ls << dli.dli_sname;
    }
    *Ls << "\n";
  }
  delete Callers;
  Callers = nullptr;
}
} // end of namespace Ice

#else // !ALLOW_LINUX_MALLOC_PROFILE

namespace Ice {

LinuxMallocProfiling::LinuxMallocProfiling(size_t NumThreads, Ostream *Ls) {
  (void)NumThreads;
  (void)Ls;
}

LinuxMallocProfiling::~LinuxMallocProfiling() {}
} // end of namespace Ice

#endif // ALLOW_LINUX_MALLOC_PROFILE
