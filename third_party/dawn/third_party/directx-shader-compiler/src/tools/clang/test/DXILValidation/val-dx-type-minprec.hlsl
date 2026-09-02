// This is intended to be used to help generate validation tests to test fixes
// for the way DxilOperations gets initialized on an existing llvm module.
//
// There are a couple interacting issues:
// 1. When RefreshCache occurs, it doesn't fill in the m_pResRetType cache.
//  - This is because in GetOpFunc, if it finds a function matching the
//    expected name, it simply returns that, skipping function type
//    construction.
//  - This causes a validation error if CheckAccessFullyMapped is validated
//    before a function that calls it.  This depends on the order of
//    function declarations/definitions in the module.
//  - The solution is to move the check after type construction, and verify
//    that the function type matches expectations as well.
// 2. When RefreshCache() and FixOverloadNames() was called from DxilOperations
//    constructor, the m_LowPrecisionMode has not yet been set, leading to
//    incorrect overload types when allowing function type construction to
//    proceed in GetOpFunc before looking for a matching function by name.
//  - This leads to an incorrect type for 16-bit CBufRet type being used.
//  - The solution is to move RefreshCache() and FixOverloadNames() calls
//    to after we actually know the m_LowPrecisionMode, so that the correct
//    types may be constructed.
//
// So, this test will generate code that uses both CheckAccessFullyMapped and
// use of min-precision types from a constant buffer, which will be translated
// to native 16 bit types when -enable-16bit-types is specified.
//  - The CheckAccessFullyMapped issue is tested by moving the declare of
//    checkAccessFullyMapped to before the main function in the IR after
//    compiling this HLSL.
//  - The incorrect constant buffer type would lead to the wrong return type in
//    the associated type slot for the intrinsic overload used.
//  - Both of these issues would be caught by the validator.

// Generated tests:
//  val-dx-type-minprec.ll
//  val-dx-type-lowprec.ll

// Instructions for constructing .ll tests:
// Compile these targets to generate tests:
//  dxc val-dx-type-minprec.hlsl -T vs_6_3 -enable-16bit-types -Fc val-dx-type-lowprec.ll
//  dxc val-dx-type-minprec.hlsl -T vs_6_3 -Fc val-dx-type-minprec.ll
// Move checkAccessFullyMapped declaration before the main function.
// Add run line with: `%dxv %s' to the top of the modified .ll output.

struct MyStruct {
  min16float f;
};

min16int4 i1;
// Extra credit: using i2 on -enable-16bit-types will extractelement from
// components above the number of components available in the min-precision
// equivalent type.
min16int4 i2;

StructuredBuffer<MyStruct> SB;

min16float main() : OUT {
  uint status;
  min16float result = SB.Load(i2.y, status).f;
  return CheckAccessFullyMapped(status) ? result : -1.0;
}
