// RUN: %dxc -E plain -T vs_6_0 %s | FileCheck %s

// CHECK: @plain

//
// without also putting them in a static assertion
//
// we use -Wno-conversion because many of the assignments result in precision loss
//

// non-standard: consider right-hand-side const as equivalent
//template<typename T>             struct is_same<T, const T> : public true_type{};
// non-standard: consider right-hand-side lvalue as equivalent
//template<typename T>             struct is_same<T, T&>      : public true_type{};

// __decltype is the GCC way of saying 'decltype', but doesn't require C++11
#ifdef VERIFY_FXC
#endif

// To test with the classic compiler, run
// fxc.exe /T vs_5_1 scalar-operators-assign.hlsl
// with vs_2_0 (the default) min16float usage produces a complaint that it's not supported

float4 plain(float4 param4 : FOO) : FOO {
    bool        bools       = 0;
    int         ints        = 0;
    uint        uints       = 0;
    dword       dwords      = 0;
    half        halfs       = 0;
    float       floats      = 0;
    double      doubles     = 0;
    min16float  min16floats = 0;
    min10float  min10floats = 0;
    min16int    min16ints   = 0;
    min12int    min12ints   = 0;
    min16uint   min16uints  = 0;
  
  //
  // 
  // signed integer remainder is not supported on minimum-precision types; Cast to int to use 32-bit division
  // *and*
  // signed division remainder is not supported on minimum-precision types; Cast to int to use 32-bit division
  // are both mapped to
  // signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division
  //
  return 1.2;

}