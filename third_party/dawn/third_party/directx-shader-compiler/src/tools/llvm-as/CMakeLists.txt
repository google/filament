set(LLVM_LINK_COMPONENTS
  AsmParser
  BitWriter
  Core
  Support
  MSSupport # HLSL Change
  )

add_llvm_tool(llvm-as
  llvm-as.cpp
  )

# HLSL Change Starts
if (NOT HLSL_OPTIONAL_PROJS_IN_DEFAULT)
  set_target_properties(llvm-as PROPERTIES EXCLUDE_FROM_ALL ON EXCLUDE_FROM_DEFAULT_BUILD ON)
endif ()
# HLSL Change Ends
