// RUN: %dxc /Tps_6_0 /Emain %s | FileCheck %s
// github issue #1684
// Test explicit cast between structs of identical layouts.

// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 0.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 0.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 0.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 0.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
struct VSOut { float4 color : COLOR; };
struct VSOut2 { float4 color : COLOR; };

float4 main(VSOut psin) : SV_Target0
{
    (VSOut2)psin;
    return 0;
}