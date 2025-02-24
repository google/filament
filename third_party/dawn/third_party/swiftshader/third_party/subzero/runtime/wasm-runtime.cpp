//===- subzero/runtime/wasm-runtime.cpp - Subzero WASM runtime source -----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the system calls required by the libc that is included
// in WebAssembly programs.
//
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#ifdef WASM_TRACE_RUNTIME
#define TRACE_ENTRY()                                                          \
  { std::cerr << __func__ << "(...) = "; }
template <typename T> T trace(T x) {
  std::cerr << x << std::endl;
  return x;
}
void trace() { std::cerr << "(void)" << std::endl; }
#else
#define TRACE_ENTRY()
template <typename T> T trace(T x) { return x; }
void trace() {}
#endif // WASM_TRACE_RUNTIME

extern "C" {
char *WASM_MEMORY;
extern uint32_t WASM_DATA_SIZE;
extern uint32_t WASM_NUM_PAGES;
} // end of extern "C"

namespace {
uint32_t HeapBreak;

// TODO (eholk): make all of these constexpr.
const uint32_t PageSizeLog2 = 16;
const uint32_t PageSize = 1 << PageSizeLog2; // 64KB
const uint32_t StackPtrLoc = 1024;           // defined by emscripten

uint32_t pageNum(uint32_t Index) { return Index >> PageSizeLog2; }
} // end of anonymous namespace

namespace env {
double floor(double X) { return std::floor(X); }

float floor(float X) { return std::floor(X); }
} // end of namespace env

// TODO (eholk): move the C parts outside and use C++ name mangling.

namespace {

/// Some runtime functions need to return pointers. The WasmData struct is used
/// to preallocate space for these on the heap.
struct WasmData {

  /// StrBuf is returned by functions that return strings.
  char StrBuf[256];
};

WasmData *GlobalData = NULL;

int toWasm(void *Ptr) {
  return reinterpret_cast<int>(reinterpret_cast<char *>(Ptr) - WASM_MEMORY);
}

template <typename T> T *wasmPtr(int Index) {
  if (pageNum(Index) < WASM_NUM_PAGES) {
    return reinterpret_cast<T *>(WASM_MEMORY + Index);
  }
  abort();
}

template <typename T> class WasmPtr {
  int Ptr;

public:
  WasmPtr(int Ptr) : Ptr(Ptr) {
    // TODO (eholk): make this a static_assert once we have C++11
    assert(sizeof(*this) == sizeof(int));
  }

  WasmPtr(T *Ptr) : Ptr(toWasm(Ptr)) {}

  T &operator*() const { return *asPtr(); }

  T *asPtr() const { return wasmPtr<T>(Ptr); }

  int asInt() const { return Ptr; }
};

typedef WasmPtr<char> WasmCharPtr;

template <typename T> class WasmArray {
  int Ptr;

public:
  WasmArray(int Ptr) : Ptr(Ptr) {
    // TODO (eholk): make this a static_assert once we have C++11.
    assert(sizeof(*this) == sizeof(int));
  }

  T &operator[](unsigned int Index) const { return wasmPtr<T>(Ptr)[Index]; }
};
} // end of anonymous namespace

// TODO (eholk): move the C parts outside and use C++ name mangling.
extern "C" {

void __Sz_bounds_fail() {
  std::cerr << "Bounds check failure" << std::endl;
  abort();
}

void __Sz_indirect_fail() {
  std::cerr << "Invalid indirect call target" << std::endl;
  abort();
}

extern char WASM_DATA_INIT[];

void env$$abort() {
  fprintf(stderr, "Aborting...\n");
  abort();
}

void env$$_abort() { env$$abort(); }

double env$$floor_f(float X) {
  TRACE_ENTRY();
  return env::floor(X);
}
double env$$floor_d(double X) {
  TRACE_ENTRY();
  return env::floor(X);
}

void env$$exit(int Status) {
  TRACE_ENTRY();
  exit(Status);
}
void env$$_exit(int Status) {
  TRACE_ENTRY();
  env$$exit(Status);
}

#define UNIMPLEMENTED(f)                                                       \
  void env$$##f() {                                                            \
    fprintf(stderr, "Unimplemented: " #f "\n");                                \
    abort();                                                                   \
  }

int32_t env$$sbrk(int32_t Increment) {
  TRACE_ENTRY();
  uint32_t OldBreak = HeapBreak;
  HeapBreak += Increment;
  return trace(OldBreak);
}

UNIMPLEMENTED(__addtf3)
UNIMPLEMENTED(__assert_fail)
UNIMPLEMENTED(__builtin_apply)
UNIMPLEMENTED(__builtin_apply_args)
UNIMPLEMENTED(__builtin_isinff)
UNIMPLEMENTED(__builtin_isinfl)
UNIMPLEMENTED(__builtin_malloc)
UNIMPLEMENTED(__divtf3)
UNIMPLEMENTED(__eqtf2)
UNIMPLEMENTED(__extenddftf2)
UNIMPLEMENTED(__extendsftf2)
UNIMPLEMENTED(__fixsfti)
UNIMPLEMENTED(__fixtfdi)
UNIMPLEMENTED(__fixtfsi)
UNIMPLEMENTED(__fixunstfsi)
UNIMPLEMENTED(__floatditf)
UNIMPLEMENTED(__floatsitf)
UNIMPLEMENTED(__floatunsitf)
UNIMPLEMENTED(__getf2)
UNIMPLEMENTED(__letf2)
UNIMPLEMENTED(__lttf2)
UNIMPLEMENTED(__multf3)
UNIMPLEMENTED(__multi3)
UNIMPLEMENTED(__netf2)
UNIMPLEMENTED(__subtf3)
UNIMPLEMENTED(__syscall140) // sys_llseek
UNIMPLEMENTED(__syscall221) // sys_fcntl64
UNIMPLEMENTED(__trunctfdf2)
UNIMPLEMENTED(__trunctfsf2)
UNIMPLEMENTED(__unordtf2)
UNIMPLEMENTED(longjmp)
UNIMPLEMENTED(pthread_cleanup_pop)
UNIMPLEMENTED(pthread_cleanup_push)
UNIMPLEMENTED(pthread_self)
UNIMPLEMENTED(setjmp)

extern int __szwasm_main(int, WasmPtr<WasmCharPtr>);

#define WASM_REF(Type, Index) (WasmPtr<Type>(Index).asPtr())
#define WASM_DEREF(Type, Index) (*WASM_REF(Type, Index))

int main(int argc, const char **argv) {
  // Create the heap.
  std::vector<char> WasmHeap(WASM_NUM_PAGES << PageSizeLog2);
  WASM_MEMORY = WasmHeap.data();
  std::copy(WASM_DATA_INIT, WASM_DATA_INIT + WASM_DATA_SIZE, WasmHeap.begin());

  // TODO (eholk): align these allocations correctly.

  // Allocate space for the global data.
  HeapBreak = WASM_DATA_SIZE;
  GlobalData = WASM_REF(WasmData, HeapBreak);
  HeapBreak += sizeof(WasmData);

  // copy the command line arguments.
  WasmPtr<WasmCharPtr> WasmArgV = HeapBreak;
  WasmPtr<char> *WasmArgVPtr = WasmArgV.asPtr();
  HeapBreak += argc * sizeof(*WasmArgVPtr);

  for (int i = 0; i < argc; ++i) {
    WasmArgVPtr[i] = HeapBreak;
    strcpy(WASM_REF(char, HeapBreak), argv[i]);
    HeapBreak += strlen(argv[i]) + 1;
  }

  // Initialize the break to the nearest page boundary after the data segment
  HeapBreak = (WASM_DATA_SIZE + PageSize - 1) & ~(PageSize - 1);

  // Initialize the stack pointer.
  WASM_DEREF(int32_t, StackPtrLoc) = WASM_NUM_PAGES << PageSizeLog2;

  return __szwasm_main(argc, WasmArgV);
}

int env$$abs(int a) {
  TRACE_ENTRY();
  return trace(abs(a));
}

clock_t env$$clock() {
  TRACE_ENTRY();
  return trace(clock());
}

int env$$ctime(WasmPtr<time_t> Time) {
  TRACE_ENTRY();
  char *CTime = ctime(Time.asPtr());
  strncpy(GlobalData->StrBuf, CTime, sizeof(GlobalData->StrBuf));
  GlobalData->StrBuf[sizeof(GlobalData->StrBuf) - 1] = '\0';
  return trace(WasmPtr<char>(GlobalData->StrBuf).asInt());
}

double env$$pow(double x, double y) {
  TRACE_ENTRY();
  return trace(pow(x, y));
}

time_t env$$time(WasmPtr<time_t> Time) {
  TRACE_ENTRY();
  time_t *TimePtr = WASM_REF(time_t, Time);
  return trace(time(TimePtr));
}

// lock and unlock are no-ops in wasm.js, so we mimic that behavior.
void env$$__lock(int32_t) {
  TRACE_ENTRY();
  trace();
}

void env$$__unlock(int32_t) {
  TRACE_ENTRY();
  trace();
}

/// sys_read
int env$$__syscall3(int Which, WasmArray<int> VarArgs) {
  TRACE_ENTRY();
  int Fd = VarArgs[0];
  int Buffer = VarArgs[1];
  int Length = VarArgs[2];

  return trace(read(Fd, WASM_REF(char *, Buffer), Length));
}

/// sys_write
int env$$__syscall4(int Which, WasmArray<int> VarArgs) {
  TRACE_ENTRY();
  int Fd = VarArgs[0];
  int Buffer = VarArgs[1];
  int Length = VarArgs[2];

  return trace(write(Fd, WASM_REF(char *, Buffer), Length));
}

/// sys_open
int env$$__syscall5(int Which, WasmArray<int> VarArgs) {
  TRACE_ENTRY();
  int WasmPath = VarArgs[0];
  int Flags = VarArgs[1];
  int Mode = VarArgs[2];
  const char *Path = WASM_REF(char, WasmPath);

  return trace(open(Path, Flags, Mode));
}

/// sys_close
int env$$__syscall6(int Which, WasmArray<int> VarArgs) {
  TRACE_ENTRY();
  int Fd = VarArgs[0];

  return trace(close(Fd));
}

/// sys_unlink
int env$$__syscall10(int Which, WasmArray<int> VarArgs) {
  TRACE_ENTRY();
  int WasmPath = VarArgs[0];
  const char *Path = WASM_REF(char, WasmPath);

  return trace(unlink(Path));
}

/// sys_getpid
int env$$__syscall20(int Which, WasmArray<int> VarArgs) {
  TRACE_ENTRY();
  (void)Which;
  (void)VarArgs;

  return trace(getpid());
}

/// sys_rmdir
int env$$__syscall40(int Which, WasmArray<int> VarArgs) {
  TRACE_ENTRY();
  int WasmPath = VarArgs[0];
  const char *Path = WASM_REF(char, WasmPath);

  return trace(rmdir(Path));
}

/// sys_ioctl
int env$$__syscall54(int Which, WasmArray<int> VarArgs) {
  TRACE_ENTRY();
  int Fd = VarArgs[0];
  int Op = VarArgs[1];
  int ArgP = VarArgs[2];

  switch (Op) {
  case TCGETS: {
    // struct termios has no pointers. Otherwise, we'd have to rewrite them.
    struct termios *TermIOS = WASM_REF(struct termios, ArgP);
    return trace(ioctl(Fd, TCGETS, TermIOS));
  }
  default:
    // TODO (eholk): implement more ioctls
    return trace(-ENOTTY);
  }
}

struct IoVec {
  WasmPtr<char> Ptr;
  int Length;
};

/// sys_readv
int env$$__syscall145(int Which, WasmArray<int> VarArgs) {
  TRACE_ENTRY();
  int Fd = VarArgs[0];
  WasmArray<IoVec> Iov = VarArgs[1];
  int Iovcnt = VarArgs[2];

  int Count = 0;

  for (int I = 0; I < Iovcnt; ++I) {
    int Curr = read(Fd, Iov[I].Ptr.asPtr(), Iov[I].Length);

    if (Curr < 0) {
      return trace(-1);
    }
    Count += Curr;
  }
  return trace(Count);
}

/// sys_writev
int env$$__syscall146(int Which, WasmArray<int> VarArgs) {
  TRACE_ENTRY();
  int Fd = VarArgs[0];
  WasmArray<IoVec> Iov = VarArgs[1];
  int Iovcnt = VarArgs[2];

  int Count = 0;

  for (int I = 0; I < Iovcnt; ++I) {
    int Curr = write(Fd, Iov[I].Ptr.asPtr(), Iov[I].Length);

    if (Curr < 0) {
      return trace(-1);
    }
    Count += Curr;
  }
  return trace(Count);
}

/// sys_mmap_pgoff
int env$$__syscall192(int Which, WasmArray<int> VarArgs) {
  TRACE_ENTRY();
  (void)Which;
  (void)VarArgs;

  // TODO (eholk): figure out how to implement this.

  return trace(-ENOMEM);
}
} // end of extern "C"
