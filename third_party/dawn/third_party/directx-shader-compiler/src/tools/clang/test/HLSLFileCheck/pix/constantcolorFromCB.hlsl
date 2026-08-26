// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-constantColor,mod-mode=1 | %FileCheck %s

// Check that the CB return type has been added:
// CHECK: %dx.types.CBufRet.f32 = type { float, float, float, float }

// Look for call to create handle at index 1: --------------------------------------------------------V
// CHECK: %PIX_Constant_Color_CB_Handle = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 1, i32 0, i1 false)

// Look for callto read from CB:
// CHECK: %PIX_Constant_Color_Value = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %PIX_Constant_Color_CB_Handle, i32 0)

// Calls to load elements:
// CHECK: %PIX_Constant_Color_Value0 = extractvalue %dx.types.CBufRet.f32 %PIX_Constant_Color_Value, 0
// CHECK: %PIX_Constant_Color_Value1 = extractvalue %dx.types.CBufRet.f32 %PIX_Constant_Color_Value, 1
// CHECK: %PIX_Constant_Color_Value2 = extractvalue %dx.types.CBufRet.f32 %PIX_Constant_Color_Value, 2
// CHECK: %PIX_Constant_Color_Value3 = extractvalue %dx.types.CBufRet.f32 %PIX_Constant_Color_Value, 3

// Check that the store-output has been modifed:
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %PIX_Constant_Color_Value0)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %PIX_Constant_Color_Value1)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %PIX_Constant_Color_Value2)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %PIX_Constant_Color_Value3)

cbuffer Constants : register(b0)
{
  float4 color;
}

[RootSignature("CBV(b0, space=0, visibility=SHADER_VISIBILITY_PIXEL)")]
float4 main() : SV_Target {
  return color;
}