set(LLVM_LINK_COMPONENTS
  Support
  DxilHash
  )

add_clang_unittest(DxilHashTests
  DxilHashTest.cpp
  )
