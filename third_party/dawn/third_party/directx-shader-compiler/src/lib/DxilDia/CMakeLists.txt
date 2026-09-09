# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.

if (MSVC)
  find_package(DiaSDK REQUIRED) # Used for constants and declarations.
endif (MSVC)

if (MSVC)
  add_llvm_library(LLVMDxilDia
    DxcPixCompilationInfo.cpp
    DxcPixDxilDebugInfo.cpp
    DxcPixDxilStorage.cpp
    DxcPixEntrypoints.cpp
    DxcPixLiveVariables.cpp
    DxcPixLiveVariables_FragmentIterator.cpp
    DxcPixTypes.cpp
    DxcPixVariables.cpp
    DxilDia.cpp
    DxilDiaDataSource.cpp
    DxilDiaEnumTables.cpp
    DxilDiaSession.cpp
    DxilDiaSymbolManager.cpp
    DxilDiaTable.cpp
    DxilDiaTableFrameData.cpp
    DxilDiaTableInjectedSources.cpp
    DxilDiaTableInputAssemblyFile.cpp
    DxilDiaTableLineNumbers.cpp
    DxilDiaTableSections.cpp
    DxilDiaTableSegmentMap.cpp
    DxilDiaTableSourceFiles.cpp
    DxilDiaTableSymbols.cpp

    ADDITIONAL_HEADER_DIRS
    ${LLVM_MAIN_INCLUDE_DIR}/llvm/IR
  )
else(MSVC)
  # DxcPixLiveVariables_FragmentIterator is not dependent on dia.
  # It is used by PixTest.
  set(HLSL_IGNORE_SOURCES
    DxcPixCompilationInfo.cpp
    DxcPixDxilDebugInfo.cpp
    DxcPixDxilStorage.cpp
    DxcPixEntrypoints.cpp
    DxcPixLiveVariables.cpp
    DxcPixTypes.cpp
    DxcPixVariables.cpp
    DxilDia.cpp
    DxilDiaDataSource.cpp
    DxilDiaEnumTables.cpp
    DxilDiaSession.cpp
    DxilDiaSymbolManager.cpp
    DxilDiaTable.cpp
    DxilDiaTableFrameData.cpp
    DxilDiaTableInjectedSources.cpp
    DxilDiaTableInputAssemblyFile.cpp
    DxilDiaTableLineNumbers.cpp
    DxilDiaTableSections.cpp
    DxilDiaTableSegmentMap.cpp
    DxilDiaTableSourceFiles.cpp
    DxilDiaTableSymbols.cpp
    )
  add_llvm_library(LLVMDxilDia
    DxcPixLiveVariables_FragmentIterator.cpp

    ADDITIONAL_HEADER_DIRS
    ${LLVM_MAIN_INCLUDE_DIR}/llvm/IR
  )
endif(MSVC)

if (MSVC)
  target_link_libraries(LLVMDxilDia PRIVATE ${LIBRARIES} ${DIASDK_LIBRARIES})
  include_directories(AFTER ${LLVM_INCLUDE_DIR}/dxc/Tracing ${DIASDK_INCLUDE_DIRS})
endif (MSVC)

add_dependencies(LLVMDxilDia LLVMDxilPIXPasses intrinsics_gen)
