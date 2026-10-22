# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
add_hlsl_hctgen(DxilValidationInc OUTPUT DxilValidation.inc BUILD_DIR)
add_hlsl_hctgen(DxilValidation OUTPUT DxilValidationImpl.inc BUILD_DIR)

add_llvm_library(LLVMDxilValidation
  DxilContainerValidation.cpp
  DxilValidation.cpp
  DxilValidationUtils.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/IR
)

add_dependencies(LLVMDxilValidation intrinsics_gen)
