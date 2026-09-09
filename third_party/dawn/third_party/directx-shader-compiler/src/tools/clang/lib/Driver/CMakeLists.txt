set(LLVM_LINK_COMPONENTS
  Option
  Support
  )

set(HLSL_IGNORE_SOURCES
  Action.cpp
  Compilation.cpp
  CrossWindowsToolChain.cpp
  Driver.cpp
  Job.cpp
  MinGWToolChain.cpp
  Multilib.cpp
  MSVCToolChain.cpp
  Phases.cpp
  SanitizerArgs.cpp
  Tool.cpp
  ToolChain.cpp
  ToolChains.cpp
  Tools.cpp
  Types.cpp
  )

add_clang_library(clangDriver
  DriverOptions.cpp

  DEPENDS
  ClangDriverOptions

  LINK_LIBS
  clangBasic
  )
