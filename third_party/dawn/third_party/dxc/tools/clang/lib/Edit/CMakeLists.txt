set(LLVM_LINK_COMPONENTS
  Support
  )

set (HLSL_IGNORE_SOURCES
  RewriteObjCFoundationAPI.cpp
  )

add_clang_library(clangEdit
  Commit.cpp
  EditedSource.cpp

  LINK_LIBS
  clangAST
  clangBasic
  clangLex
  )
