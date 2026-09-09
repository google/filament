set(LLVM_OPTIONAL_SOURCES RewriteModernObjC.cpp) # HLSL Change - ignore file

set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_library(clangRewriteFrontend
  FixItRewriter.cpp
  FrontendActions_rewrite.cpp
  HTMLPrint.cpp
  InclusionRewriter.cpp
  RewriteMacros.cpp
  # RewriteModernObjC.cpp # HLSL Change
  RewriteObjC.cpp
  RewriteTest.cpp

  LINK_LIBS
  clangAST
  clangBasic
  clangEdit
  clangFrontend
  clangLex
  clangRewrite
  )
