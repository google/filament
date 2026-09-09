// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-constantColor | %FileCheck %s

// Check the write to the integer part was replaced (since it is RTV0):
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 1)

// Check color in RTV1 is unaffected:
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 0, float 0.000000e+00)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 1, float 0.000000e+00)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 2, float 0.000000e+00)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 3, float 0.000000e+00)

struct RTOut
{
  int i : SV_Target;
  float4 c : SV_Target1;
};

[RootSignature("")]
RTOut main()  {
  RTOut rtOut;
  rtOut.i = 8;
  rtOut.c = float4(0.f, 0.f, 0.f, 0.f);
  return rtOut;
}
