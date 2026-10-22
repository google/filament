set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangDynamicASTMatchers
  Diagnostics.cpp
  VariantValue.cpp
  Parser.cpp
  Registry.cpp

  LINK_LIBS
  clangAST
  clangASTMatchers
  clangBasic
  )

# HLSL Changes Start
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj") # otherwise will hit fatal error C1128 on x64
# HLSL Changes End
