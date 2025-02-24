// RUN: %dxc -E main -T cs_6_0 -fcgl %s | FileCheck %s

// Validate that when swizzling requires an r-value (i.e. duplicate elements), that the result is stored to
// and loaded from a temporary.
// CHECK:      store <1 x i32> %splat.splat, <1 x i32>* %tmp
// CHECK-NEXT: %1 = load <1 x i32>, <1 x i32>* %tmp
// CHECK-NEXT: %2 = shufflevector <1 x i32> %1, <1 x i32> undef, <4 x i32> zeroinitializer
// CHECK-NEXT: store <4 x i32> %2, <4 x i32>* %x

groupshared int a;
[numthreads(64, 1, 1)]
void main() {
  a = 123;
  int4 x = (a).xxxx;
}
