// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck -check-prefixes=CHECK,BUFFER %s
// RUN: %dxc -E main -T ps_6_0 -HV 2021 -DERANGE1 %s | FileCheck %s -check-prefix=ERANGE1
// RUN: %dxc -E main -T ps_6_0 -HV 2021 -DERANGE2 %s | FileCheck %s -check-prefix=ERANGE2
// RUN: %dxc -E main -T ps_6_0 -HV 2021 -DESCALAR %s | FileCheck %s -check-prefix=ESCALAR
// RUN: %dxc -E main -T ps_6_0 -HV 2021 -DEPARTIAL %s | FileCheck %s -check-prefix=EPARTIAL

// RUN: %dxc -E main -T ps_6_0 -HV 2021 -DWRAPCB=1 %s | FileCheck -check-prefixes=CHECK,WRAPPER %s
// RUN: %dxc -E main -T ps_6_0 -HV 2021 -DWRAPCB=2 %s | FileCheck -check-prefixes=CHECK,WRAPPER %s

// BUFFER: %CB0 = type { %"struct.Wrapper<Foo>" }
// BUFFER: %"struct.Wrapper<Foo>" = type { %struct.Foo }


// TODO: It is a bug that this global is being generated. Since the WrappedCB 
//       struct only contains a buffer object that is lifted out, the global
//       here is just an empty struct. We should just not emit it.
// WRAPPER: %"hostlayout.$Globals" = type { %"hostlayout.struct.WrappedCB<Foo>" }
// WRAPPER: %"hostlayout.struct.WrappedCB<Foo>" = type {}

// CHECK: define void @main

// BUFFER: !6 = !{i32 0, %CB0* undef, !"CB0", i32 0, i32 0, i32 1, i32 4, null}
// WRAPPER: !6 = !{i32 0, %"hostlayout.$Globals"* undef, !"$Globals", i32 0, i32 0, i32 1, i32 0, null}

template<typename T>
struct Wrapper {
  T value;
};

template<typename T>
struct WrappedCB {
#if WRAPCB == 1
  ConstantBuffer< Wrapper<T> > cb;
#else
  ConstantBuffer< T > cb;
#endif
};

struct Foo {
  float f;
};

#ifdef MANUAL_INST
Wrapper<Foo> wrappedFoo;
#endif

#ifdef WRAPCB
  WrappedCB<Foo> CB0 : register(b0);
#else
  ConstantBuffer< Wrapper<Foo> > CB0 : register(b0);
#endif

template<typename Component, int Size>
struct MyVec {
  vector<Component, Size> vec;
};

template<typename T>
void increment(inout T X) {
  X += 1;
}

#ifdef EPARTIAL
// EPARTIAL: error: function template partial specialization is not allowed
template<typename VComp, uint VSize>
void decrement(inout vector<VComp, VSize> X) {
  X.x -= 1;
}

template<typename VComp>
void decrement<VComp, 4>(inout vector<VComp, 4> X) {
  X.x -= 1;
}
#endif

template<uint VSize, typename T>
vector<T, VSize> make_vec(T X) {
  return (vector<T, VSize>)X;
}

float2 main(float4 a:A) : SV_Target {
  int X = 0;
  int3 Y = {0,0,0};
  float3 Z = {0.0,0.0,0.0};
  increment(X);
  increment(Y);
  increment(Z);

#if WRAPCB == 1
  float f = CB0.cb.value.f;
#elif WRAPCB == 2
  float f = CB0.cb.f;
#else
  float f = CB0.value.f;
#endif

  float3 M4 = make_vec<4>(f).xyz;

#ifdef ERANGE1
  // ERANGE1: error: no matching function for call to 'make_vec'
  // ERANGE1: note: candidate template ignored: substitution failure [with VSize = 5]: invalid value, valid range is between 1 and 4 inclusive
  float M5[5] = (float[5])make_vec<5>(f);
  f += M5[4];
#endif

  MyVec<float, 4> W4 = (MyVec<float, 4>)f;
  increment(W4.vec);

#ifdef ERANGE2
  // ERANGE2: invalid value, valid range is between 1 and 4 inclusive
  MyVec<float, 5> W5 = (MyVec<float, 5>)f;
  increment(W5.vec);
#endif

#ifdef ESCALAR
  // ESCALAR: error: 'Foo' cannot be used as a type parameter where a scalar is required
  MyVec<Foo, 4> Foo4 = (MyVec<Foo, 4>)f;
  increment(W4.vec);
#endif

  return Z.xy * f;
}
