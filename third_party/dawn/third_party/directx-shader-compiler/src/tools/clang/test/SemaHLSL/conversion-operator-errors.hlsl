// RUN: %dxc -Tlib_6_3 -verify -HV 2021 %s

// This test verifies that dxcompiler generates an error when defining
// a conversion operator (cast operator), which is not supported in HLSL.

struct MyStruct {
  float4 f;

  // expected-error@+1 {{conversion operator overloading is not allowed}}
  operator float4() {
    return 42;
  }
};

struct AnotherStruct {
  int x;

  // expected-error@+1 {{conversion operator overloading is not allowed}}
  operator int() {
    return x;
  }

  // expected-error@+1 {{conversion operator overloading is not allowed}}
  operator bool() {
    return x != 0;
  }
};

template<typename T>
struct TemplateStruct {
  T value;

  // expected-error@+1 {{conversion operator overloading is not allowed}}
  operator T() {
    return value;
  }
};
