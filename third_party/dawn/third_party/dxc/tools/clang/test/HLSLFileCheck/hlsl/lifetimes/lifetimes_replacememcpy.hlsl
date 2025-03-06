// RUN: %dxc -T lib_6_6 %s  | FileCheck %s
// RUN: %dxc -T lib_6_3 %s  | FileCheck %s

//
// Regression test for a case where a memcpy gets replaced. If lifetime
// intrinsics are not correctly removed or made conservative, this can
// lead to cases with invalid lifetimes.
//

// CHECK: @[[constname:.*]] = internal unnamed_addr constant [2 x float] [float 1.000000e+00, float 3.000000e+00]
// CHECK: define float @"\01?memcpy_replace{{[@$?.A-Za-z0-9_]+}}"(i32 %i)
// CHECK: getelementptr inbounds [2 x float], [2 x float]* @[[constname]], i32 0, i32 %i
// CHECK: load float
// CHECK: ret float
struct MyStruct2 {
 float x[2];
 float y;
};

MyStruct2 init() {
  // The layout of the struct and the way we initialize it has to be
  // complex enough that a memcpy is generated.
  MyStruct2 s;
  s.y = 3;
  s.x[0] = 1;
  s.x[1] = s.y;
  return s;
}

export
float memcpy_replace(int i) {
  MyStruct2 s = init();
  // Memcpy from inlined alloca to local alloca of s happens here.
  //   Memcpy replacement replaces s by the inlined one.
  // Lifetime of inlined alloca ends here

  // Access local variable here again.
  // If everything works correctly, this should be a GEP to a constant
  // that has 1, 3 as its elements. When lifetimes were not removed
  // conservatively during memcpy removal, they caused this to load
  // uninitialized memory, represented by a load from a zeroinitialized
  // constant.
  return s.x[i];
}

// This is a slightly more complex variant that exposes the same issue.
//
//float func3(in float x);
//
//export
//float memcpy_replace2() {
//  float res = 0;
//  MyStruct2 s = init();
//
//  [loop]
//  for (uint i = 0; i < 3; i++) {
//    res += func3(s.y);
//  }
//  res *= s.x[res];
//
//  return res;
//}
