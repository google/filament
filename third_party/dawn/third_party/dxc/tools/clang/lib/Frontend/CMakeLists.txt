add_subdirectory(Rewrite)

set(LLVM_LINK_COMPONENTS
  BitReader
  Option
  Support
  )

# HLSL Change - add ignored sources
set(HLSL_IGNORE_SOURCES ChainedIncludesSource.cpp)

add_clang_library(clangFrontend
  ASTConsumers.cpp
  ASTMerge.cpp
  ASTUnit.cpp
  CacheTokens.cpp
  ChainedDiagnosticConsumer.cpp
  # ChainedIncludesSource.cpp      # HLSL Change
  CodeGenOptions.cpp
  CompilerInstance.cpp
  CompilerInvocation.cpp
  CreateInvocationFromCommandLine.cpp
  DependencyFile.cpp
  DependencyGraph.cpp
  DiagnosticRenderer.cpp
  FrontendAction.cpp
  FrontendActions.cpp
  FrontendOptions.cpp
  HeaderIncludeGen.cpp
  InitHeaderSearch.cpp
  InitPreprocessor.cpp
  LangStandards.cpp
  LayoutOverrideSource.cpp
  LogDiagnosticPrinter.cpp
  ModuleDependencyCollector.cpp
  MultiplexConsumer.cpp
  PCHContainerOperations.cpp
  PrintPreprocessedOutput.cpp
  SerializedDiagnosticPrinter.cpp
  SerializedDiagnosticReader.cpp
  TextDiagnostic.cpp
  TextDiagnosticBuffer.cpp
  TextDiagnosticPrinter.cpp
  VerifyDiagnosticConsumer.cpp

  DEPENDS
  ClangDriverOptions
  TablegenHLSLOptions
  hlsl_dxcversion_autogen # HLSL Change

  LINK_LIBS
  clangAST
  clangBasic
  clangDriver
  clangEdit
  clangLex
  clangParse
  clangSema
#  clangSerialization # HLSL Change
  )
target_include_directories(clangFrontend PUBLIC ${HLSL_VERSION_LOCATION}) # HLSL Change
