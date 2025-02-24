
ifndef SUBZERO_LEVEL
# Top-level, not included from a subdir
SUBZERO_LEVEL := .
DIRS := src
PARALLEL_DIRS :=
endif

# Set LLVM source root level.
LEVEL := $(SUBZERO_LEVEL)/../..

# Include LLVM common makefile.
include $(LEVEL)/Makefile.common

# -O3 seems to trigger the following PNaCl ABI transform bug
# on method pointers, so override that with -O2:
# https://code.google.com/p/nativeclient/issues/detail?id=3857
CXX.Flags += -O2
# Newlib paired with libc++ requires gnu.
CXX.Flags += -std=gnu++11

ifeq ($(PNACL_BROWSER_TRANSLATOR),1)
  CPP.Defines += -DALLOW_DUMP=0 -DALLOW_LLVM_CL=0 -DALLOW_LLVM_IR=0 \
    -DALLOW_TIMERS=0 -DALLOW_LLVM_IR_AS_INPUT=0 -DALLOW_MINIMAL_BUILD=1 \
    -DALLOW_WASM=0 -DPNACL_BROWSER_TRANSLATOR=1
else
  CPP.Defines += -DALLOW_DUMP=1 -DALLOW_LLVM_CL=1 -DALLOW_LLVM_IR=1 \
    -DALLOW_TIMERS=1 -DALLOW_LLVM_IR_AS_INPUT=1 -DALLOW_MINIMAL_BUILD=0 \
    -DALLOW_WASM=0 -DPNACL_BROWSER_TRANSLATOR=0
  CXX.Flags += -Wno-undefined-var-template
endif

CPP.Defines += -DPNACL_LLVM
# SUBZERO_SRC_ROOT should already be set, but if not, set to cwd.
SUBZERO_SRC_ROOT ?= .
SZ_COMMIT_COUNT := $(shell git -C $(SUBZERO_SRC_ROOT) rev-list --count HEAD)
SZ_GIT_HASH := $(shell git -C $(SUBZERO_SRC_ROOT) rev-parse HEAD)
CPP.Defines += -DSUBZERO_REVISION=$(SZ_COMMIT_COUNT)_$(SZ_GIT_HASH)
