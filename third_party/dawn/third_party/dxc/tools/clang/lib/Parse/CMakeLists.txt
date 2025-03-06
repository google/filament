set(LLVM_LINK_COMPONENTS
#  MC            # HLSL Change
#  MCParser      # HLSL Change
  Support
  )

add_clang_library(clangParse
  ParseAST.cpp
  ParseCXXInlineMethods.cpp
  ParseDecl.cpp
  ParseDeclCXX.cpp
  ParseExpr.cpp
  ParseExprCXX.cpp
  ParseInit.cpp
  ParseObjc.cpp
  ParseOpenMP.cpp
  ParsePragma.cpp
  ParseStmt.cpp
  ParseStmtAsm.cpp
  ParseTemplate.cpp
  ParseTentative.cpp
  Parser.cpp
  ParseHLSL.cpp # HLSL Change
  HLSLRootSignature.cpp # HLSL Change

  LINK_LIBS
  clangAST
  clangBasic
  clangLex
  clangSema
  )
