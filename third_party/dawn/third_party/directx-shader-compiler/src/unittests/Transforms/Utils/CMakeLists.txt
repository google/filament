set(LLVM_LINK_COMPONENTS
  Core
  DXIL # HLSL Change
  Support
  TransformUtils
  )

add_llvm_unittest(UtilsTests
  ASanStackFrameLayoutTest.cpp
  Cloning.cpp
  IntegerDivision.cpp
  Local.cpp
  ValueMapperTest.cpp
  )
