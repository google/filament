// RUN: %dxc -T lib_6_6 -force-zero-store-lifetimes %s  | FileCheck %s

//
// Same test as in lifetimes.hlsl, but expecting zeroinitializer store
// instead of lifetime intrinsics due to flag -force-zero-store-lifetimes.
//
// CHECK: define i32 @"\01?if_scoped_array{{[@$?.A-Za-z0-9_]+}}"
// CHECK: alloca
// CHECK: icmp
// CHECK: br i1
// CHECK: store [200 x i32] zeroinitializer
// CHECK: br label
// CHECK: load i32
// CHECK: store [200 x i32] zeroinitializer
// CHECK: br label
// CHECK: phi i32
// CHECK: load i32
// CHECK: store i32
// CHECK: br i1
// CHECK: phi i32
// CHECK: ret i32
export
int if_scoped_array(int n, int c)
{
  int res = c;

  if (n > 0) {
    int arr[200];

    // Fake some dynamic initialization so the array can't be optimzed away.
    for (int i = 0; i < n; ++i) {
        arr[i] = arr[c - i];
    }

    res = arr[c];
  }

  return res;
}
