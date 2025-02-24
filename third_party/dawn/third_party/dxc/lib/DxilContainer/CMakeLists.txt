# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
add_llvm_library(LLVMDxilContainer
  D3DReflectionDumper.cpp
  D3DReflectionStrings.cpp
  DxilContainer.cpp
  DxilContainerAssembler.cpp
  DxilContainerReader.cpp
  DxcContainerBuilder.cpp
  DxilPipelineStateValidation.cpp
  DxilRDATBuilder.cpp
  DxilRuntimeReflection.cpp
  RDATDumper.cpp
  RDATDxilSubobjects.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/IR
)

add_dependencies(LLVMDxilContainer intrinsics_gen)
