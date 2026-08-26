// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-constantColor,mod-mode=1 | %FileCheck %s

// Check that the CB return type has been added:
// CHECK: %dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }

// Look for call to create handle:
// CHECK: %PIX_Constant_Color_CB_Handle = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 0, i1 false)

// Look for callto read from CB:
// CHECK: %PIX_Constant_Color_Value = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %PIX_Constant_Color_CB_Handle, i32 0)

// Calls to load elements:
// CHECK: %PIX_Constant_Color_Value0 = extractvalue %dx.types.CBufRet.i32 %PIX_Constant_Color_Value, 0
// CHECK: %PIX_Constant_Color_Value1 = extractvalue %dx.types.CBufRet.i32 %PIX_Constant_Color_Value, 1
// CHECK: %PIX_Constant_Color_Value2 = extractvalue %dx.types.CBufRet.i32 %PIX_Constant_Color_Value, 2
// CHECK: %PIX_Constant_Color_Value3 = extractvalue %dx.types.CBufRet.i32 %PIX_Constant_Color_Value, 3

// Check that the store-output has been modifed:
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 %PIX_Constant_Color_Value0)
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 %PIX_Constant_Color_Value1)
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 2, i32 %PIX_Constant_Color_Value2)
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 3, i32 %PIX_Constant_Color_Value3)

[RootSignature("")]
int4 main() : SV_Target {
    return int4(0,0,0,0);
}