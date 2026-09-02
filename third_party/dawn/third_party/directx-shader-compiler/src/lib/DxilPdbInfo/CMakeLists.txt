
add_llvm_library(LLVMDxilPdbInfo
  DxilPdbInfoWriter.cpp

  ADDITIONAL_HEADER_DIRS
)

add_dependencies(LLVMDxilPdbInfo intrinsics_gen)

