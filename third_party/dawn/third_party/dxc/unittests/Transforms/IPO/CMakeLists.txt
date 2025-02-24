set(LLVM_LINK_COMPONENTS
  Core
  DXIL   # HLSL Change
  Support
  IPO
  )

add_llvm_unittest(IPOTests
  LowerBitSets.cpp
  )
