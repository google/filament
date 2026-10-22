# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License.
# See LICENSE.TXT for details.

add_llvm_library(LLVMDxilCompression
  DxilCompression.cpp
  miniz.c

  ADDITIONAL_HEADER_DIRS
)

# By default, miniz performs unaligned loads/stores in x86/x64 platforms. This
# is undefined behavior in C, and so we explicitly disable it.
target_compile_definitions(LLVMDxilCompression PRIVATE MINIZ_USE_UNALIGNED_LOADS_AND_STORES=0)

add_dependencies(LLVMDxilCompression intrinsics_gen)

