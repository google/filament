set(LLVM_LINK_COMPONENTS
  Support
  )

# HLSL Change - add ignored sources
set(HLSL_IGNORE_SOURCES
  CocoaConventions.cpp
  FormatString.cpp
  PrintfFormatString.cpp
  ScanfFormatString.cpp
  )

add_clang_library(clangAnalysis
  AnalysisDeclContext.cpp
  BodyFarm.cpp
  CFG.cpp
  CFGReachabilityAnalysis.cpp
  CFGStmtMap.cpp
  CallGraph.cpp
  Consumed.cpp
  CodeInjector.cpp
  Dominators.cpp
  LiveVariables.cpp
  ObjCNoReturn.cpp
  PostOrderCFGView.cpp
  ProgramPoint.cpp
  PseudoConstantAnalysis.cpp
  ReachableCode.cpp
  ThreadSafety.cpp
  ThreadSafetyCommon.cpp
  ThreadSafetyLogical.cpp
  ThreadSafetyTIL.cpp
  UninitializedValues.cpp

  LINK_LIBS
  clangAST
  clangBasic
  clangLex
  )
