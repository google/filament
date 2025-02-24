add_llvm_tool_subdirectory(llvm-config)

# Build polly before the tools: the tools link against polly when
# LINK_POLLY_INTO_TOOLS is set.
if(WITH_POLLY)
  add_llvm_external_project(polly)
else(WITH_POLLY)
  list(APPEND LLVM_IMPLICIT_PROJECT_IGNORE "${LLVM_MAIN_SRC_DIR}/tools/polly")
endif(WITH_POLLY)

if( LLVM_BUILD_LLVM_DYLIB )
  add_llvm_tool_subdirectory(llvm-shlib)
else()
  ignore_llvm_tool_subdirectory(llvm-shlib)
endif()

add_llvm_tool_subdirectory(opt) # HLSL Change

add_llvm_tool_subdirectory(llvm-as) # HLSL Change
add_llvm_tool_subdirectory(llvm-dis) # HLSL Change
# add_llvm_tool_subdirectory(llvm-mc) # HLSL Change

# HLSL Change Begins
if (MSVC)
# This target can currently only be built on Windows.
add_llvm_tool_subdirectory(dxexp)
endif (MSVC)
# HLSL Change ends

# add_llvm_tool_subdirectory(llc) # HLSL Change
# add_llvm_tool_subdirectory(llvm-ar) # HLSL Change
# add_llvm_tool_subdirectory(llvm-nm) # HLSL Change
# add_llvm_tool_subdirectory(llvm-size) # HLSL Change

# add_llvm_tool_subdirectory(llvm-cov) # HLSL Change
# add_llvm_tool_subdirectory(llvm-profdata) # HLSL Change
add_llvm_tool_subdirectory(llvm-link) # HLSL Change
# add_llvm_tool_subdirectory(lli) # HLSL Change

add_llvm_tool_subdirectory(llvm-extract) # HLSL Change
add_llvm_tool_subdirectory(llvm-diff) # HLSL Change
# add_llvm_tool_subdirectory(macho-dump) # HLSL Change
# add_llvm_tool_subdirectory(llvm-objdump) # HLSL Change
# add_llvm_tool_subdirectory(llvm-readobj) # HLSL Change
# add_llvm_tool_subdirectory(llvm-rtdyld) # HLSL Change
# add_llvm_tool_subdirectory(llvm-dwarfdump) # HLSL Change
# add_llvm_tool_subdirectory(dsymutil) # HLSL Change
# add_llvm_tool_subdirectory(llvm-cxxdump) # HLSL Change
# HLSL Change - remove llvm-jitlistener conditional on LLVM_USE_INTEL_JITEVENTS

# add_llvm_tool_subdirectory(bugpoint) # HLSL Change
# add_llvm_tool_subdirectory(bugpoint-passes) # HLSL Change
add_llvm_tool_subdirectory(llvm-bcanalyzer) # HLSL Change
add_llvm_tool_subdirectory(llvm-stress) # HLSL Change
# add_llvm_tool_subdirectory(llvm-mcmarkup) # HLSL Change

add_llvm_tool_subdirectory(verify-uselistorder) # HLSL Change

# add_llvm_tool_subdirectory(llvm-symbolizer) # HLSL Change

# add_llvm_tool_subdirectory(llvm-c-test) # HLSL Change

# add_llvm_tool_subdirectory(obj2yaml) # HLSL Change
# add_llvm_tool_subdirectory(yaml2obj) # HLSL Change

# add_llvm_tool_subdirectory(llvm-go) # HLSL Change

# add_llvm_tool_subdirectory(llvm-pdbdump) # HLSL Change

if(NOT CYGWIN AND LLVM_ENABLE_PIC)
#   add_llvm_tool_subdirectory(lto) # HLSL Change
#   add_llvm_tool_subdirectory(llvm-lto) # HLSL Change
else()
  ignore_llvm_tool_subdirectory(lto)
  ignore_llvm_tool_subdirectory(llvm-lto)
endif()

# add_llvm_tool_subdirectory(gold) # HLSL Change

add_llvm_external_project(clang)
# add_llvm_external_project(llgo) # HLSL Change
# add_llvm_external_project(lld) # HLSL Change
# add_llvm_external_project(lldb) # HLSL Change

# Automatically add remaining sub-directories containing a 'CMakeLists.txt'
# file as external projects.
# add_llvm_implicit_external_projects() # HLSL Change

set(LLVM_COMMON_DEPENDS ${LLVM_COMMON_DEPENDS} PARENT_SCOPE)
