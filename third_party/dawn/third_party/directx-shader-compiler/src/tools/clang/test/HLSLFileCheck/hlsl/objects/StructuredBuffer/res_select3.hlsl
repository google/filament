// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// Make sure no phi/select of handle in lib.
// CHECK: define i32 @"\01?test{{[@$?.A-Za-z0-9_]+}}"(i32 %i, i32 %j, i32 %m)
// CHECK-NOT: phi %"class.
// CHECK-NOT: phi %dx.types.Handle
// CHECK-NOT: select i1 %{{[^,]+}}, %"class.
// CHECK-NOT: select i1 %{{[^,]+}}, %dx.types.Handle

// Make sure get dimensions returns 24
// CHECK: ret i32 24

struct MyStruct {
  float2 a;
  int b;
  float3 c;
};

RWStructuredBuffer<MyStruct> BufArray[3];

uint test(int i, int j, int m) {
  RWStructuredBuffer<MyStruct> a = BufArray[0];
  RWStructuredBuffer<MyStruct> b = BufArray[1];
  RWStructuredBuffer<MyStruct> c = BufArray[2];

  RWStructuredBuffer<MyStruct> buf = c;
  while (i > 9) {
     while (j < 4) {
        if (i < m)
          buf = b;
        buf[j].b = i;
        j++;
     }
     if (m > j)
       buf = a;
     buf[m].b = i;
     i--;
  }
  buf[i].b = j;
  uint dim = 0;
  uint stride = 0;
  buf.GetDimensions(dim, stride);
  return stride;
}
