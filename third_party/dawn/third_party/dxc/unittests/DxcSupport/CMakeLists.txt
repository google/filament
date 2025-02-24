set(LLVM_LINK_COMPONENTS
  Support
  dxcSupport
  )

add_clang_unittest(DxcSupportTests
  WinAdapterTest.cpp
  )
