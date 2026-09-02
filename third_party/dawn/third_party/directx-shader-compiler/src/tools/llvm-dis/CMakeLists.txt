set(LLVM_LINK_COMPONENTS
  BitReader
  Core
  DxilContainer
  Support
  MSSupport # HLSL Change
  )

add_llvm_tool(llvm-dis
  llvm-dis.cpp
  )

# HLSL Change Starts
if (NOT HLSL_OPTIONAL_PROJS_IN_DEFAULT)
  set_target_properties(llvm-dis PROPERTIES EXCLUDE_FROM_ALL ON EXCLUDE_FROM_DEFAULT_BUILD ON)
endif ()
# HLSL Change Ends
