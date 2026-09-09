// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK-NOT: "MyCBArray"

struct MyStruct {
int a;
};
ConstantBuffer<MyStruct> MyCBArray[5] : register(b2, space5);
float4 main() : SV_Target {
  return 0;
}
