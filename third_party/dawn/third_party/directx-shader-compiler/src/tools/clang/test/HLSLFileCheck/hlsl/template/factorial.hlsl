// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 1)

template<uint n>
struct factorial {
  enum { value = n * factorial<n -1>::value };
};

template<>
struct factorial<0> {
  enum { value = 1 };
};

bool main(int4 a:A) : SV_Target {
   return (factorial<0>::value == 1)
       && (factorial<1>::value == 1)
       && (factorial<3>::value == 6)
       && (factorial<4>::value == 24);
}
