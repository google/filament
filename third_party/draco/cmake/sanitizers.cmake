if(DRACO_CMAKE_SANITIZERS_CMAKE_)
  return()
endif()
set(DRACO_CMAKE_SANITIZERS_CMAKE_ 1)

if(MSVC OR NOT SANITIZE)
  return()
endif()

include("${draco_root}/cmake/compiler_flags.cmake")

string(TOLOWER ${SANITIZE} SANITIZE)

# Require the sanitizer requested.
require_linker_flag("-fsanitize=${SANITIZE}")
require_compiler_flag("-fsanitize=${SANITIZE}" YES)

# Make callstacks accurate.
require_compiler_flag("-fno-omit-frame-pointer -fno-optimize-sibling-calls" YES)
