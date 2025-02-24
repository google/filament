set(LLVM_LINK_COMPONENTS
  HLSL # SemaHLSL.cpp references {Has|Get}UnsignedOpcode()
  Support
  )

# HLSL Change Begin
add_hlsl_hctgen(DxilIntrinsicTables BUILD_DIR OUTPUT gen_intrin_main_tables_15.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
list(APPEND LLVM_COMMON_DEPENDS DxilIntrinsicTables)
# HLSL Change End

add_clang_library(clangSema
  AnalysisBasedWarnings.cpp
  AttributeList.cpp
  CodeCompleteConsumer.cpp
  DeclSpec.cpp
  DelayedDiagnostic.cpp
  IdentifierResolver.cpp
  JumpDiagnostics.cpp
  MultiplexExternalSemaSource.cpp
  Scope.cpp
  ScopeInfo.cpp
  Sema.cpp
  SemaAccess.cpp
  SemaAttr.cpp
  SemaCXXScopeSpec.cpp
  SemaCast.cpp
  SemaChecking.cpp
  SemaCodeComplete.cpp
  SemaConsumer.cpp
  SemaCUDA.cpp
  SemaDecl.cpp
  SemaDeclAttr.cpp
  SemaDeclCXX.cpp
  SemaDeclObjC.cpp
  SemaDXR.cpp
  SemaExceptionSpec.cpp
  SemaExpr.cpp
  SemaExprCXX.cpp
  SemaExprMember.cpp
  SemaExprObjC.cpp
  SemaFixItUtils.cpp
  SemaHLSL.cpp # HLSL Change
  SemaHLSLDiagnoseTU.cpp # HLSL Change
  SemaInit.cpp
  SemaLambda.cpp
  SemaLookup.cpp
  SemaObjCProperty.cpp
  SemaOpenMP.cpp
  SemaOverload.cpp
  SemaPseudoObject.cpp
  SemaStmt.cpp
  SemaStmtAsm.cpp
  SemaStmtAttr.cpp
  SemaTemplate.cpp
  SemaTemplateDeduction.cpp
  SemaTemplateInstantiate.cpp
  SemaTemplateInstantiateDecl.cpp
  SemaTemplateVariadic.cpp
  SemaType.cpp
  TypeLocBuilder.cpp

  LINK_LIBS
  clangAST
  clangAnalysis
  clangBasic
  clangEdit
  clangLex
  )

# HLSL Change Begin
# Sema got too big for debug builds on arm64ec, which means it will hit
# other targets too eventually.
if(WIN32)
  if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
  endif()
endif(WIN32)
# HLSL Change End
