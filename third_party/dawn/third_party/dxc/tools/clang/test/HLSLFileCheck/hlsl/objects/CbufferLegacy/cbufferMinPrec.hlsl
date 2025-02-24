// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: Minimum-precision data types
// CHECK: cbuffer Foo
// CHECK: {
// CHECK:   struct hostlayout.Foo
// CHECK:   {
// CHECK:       min16float h1;                                ; Offset:    0
// CHECK:       float3 f3;                                    ; Offset:   16
// CHECK:       min16float2 h2;                               ; Offset:   32
// CHECK:       float3 f3_1;                                  ; Offset:   48
// CHECK:       float2 f2;                                    ; Offset:   64
// CHECK:       min16float4 h4;                               ; Offset:   80
// CHECK:       min16float2 h2_1;                             ; Offset:   96
// CHECK:       min16float3 h3;                               ; Offset:  112
// CHECK:       double d1;                                    ; Offset:  128
// CHECK:   } Foo;                                            ; Offset:    0 Size:   136
// CHECK: }

// CHECK: %dx.types.CBufRet.f16 = type { half, half, half, half }

// CHECK: {{%[0-9]+}} = call %dx.types.CBufRet.f16 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %Foo_cbuffer, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
// CHECK: {{%[0-9]+}} = extractvalue %dx.types.CBufRet.f16 {{%[0-9]+}}, 0
// CHECK: {{%[0-9]+}} = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %Foo_cbuffer, i32 1)  ; CBufferLoadLegacy(handle,regIndex)
// CHECK: {{%[0-9]+}} = extractvalue %dx.types.CBufRet.f32 {{%[0-9]+}}, 0
// CHECK: {{%[0-9]+}} = call %dx.types.CBufRet.f16 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %Foo_cbuffer, i32 2)  ; CBufferLoadLegacy(handle,regIndex)
// CHECK: {{%[0-9]+}} = extractvalue %dx.types.CBufRet.f16 {{%[0-9]+}}, 0
// CHECK: {{%[0-9]+}} = extractvalue %dx.types.CBufRet.f16 {{%[0-9]+}}, 1
// CHECK: {{%[0-9]+}} = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %Foo_cbuffer, i32 3)  ; CBufferLoadLegacy(handle,regIndex)
// CHECK: {{%[0-9]+}} = extractvalue %dx.types.CBufRet.f32 {{%[0-9]+}}, 2
// CHECK: {{%[0-9]+}} = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %Foo_cbuffer, i32 4)  ; CBufferLoadLegacy(handle,regIndex)
// CHECK: {{%[0-9]+}} = extractvalue %dx.types.CBufRet.f32 {{%[0-9]+}}, 0
// CHECK: {{%[0-9]+}} = call %dx.types.CBufRet.f16 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %Foo_cbuffer, i32 5)  ; CBufferLoadLegacy(handle,regIndex)
// CHECK: {{%[0-9]+}} = extractvalue %dx.types.CBufRet.f16 {{%[0-9]+}}, 0
// CHECK: {{%[0-9]+}} = extractvalue %dx.types.CBufRet.f16 {{%[0-9]+}}, 1
// CHECK: {{%[0-9]+}} = extractvalue %dx.types.CBufRet.f16 {{%[0-9]+}}, 2
// CHECK: {{%[0-9]+}} = extractvalue %dx.types.CBufRet.f16 {{%[0-9]+}}, 3
// CHECK: {{%[0-9]+}} = call %dx.types.CBufRet.f16 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %Foo_cbuffer, i32 6)  ; CBufferLoadLegacy(handle,regIndex)
// CHECK: {{%[0-9]+}} = extractvalue %dx.types.CBufRet.f16 {{%[0-9]+}}, 0
// CHECK: {{%[0-9]+}} = extractvalue %dx.types.CBufRet.f16 {{%[0-9]+}}, 1
// CHECK: {{%[0-9]+}} = call %dx.types.CBufRet.f16 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %Foo_cbuffer, i32 7)  ; CBufferLoadLegacy(handle,regIndex)
// CHECK: {{%[0-9]+}} = extractvalue %dx.types.CBufRet.f16 {{%[0-9]+}}, 0
// CHECK: {{%[0-9]+}} = extractvalue %dx.types.CBufRet.f16 {{%[0-9]+}}, 1
// CHECK: {{%[0-9]+}} = extractvalue %dx.types.CBufRet.f16 {{%[0-9]+}}, 2
// CHECK: {{%[0-9]+}} = call %dx.types.CBufRet.f64 @dx.op.cbufferLoadLegacy.f64(i32 59, %dx.types.Handle %Foo_cbuffer, i32 8)  ; CBufferLoadLegacy(handle,regIndex)
// CHECK: {{%[0-9]+}} = extractvalue %dx.types.CBufRet.f64 {{%[0-9]+}}, 0

cbuffer Foo {
  min16float h1;
  float3 f3;
  min16float2 h2;
  float3 f3_1;
  float2 f2;
  min16float4 h4;
  min16float2 h2_1;
  min16float3 h3;
  double d1;
}

float4 main() : SV_Target {
  return h1 + f3.x + h2.x + h2.y + f3_1.z + f2.x + h4.x + h4.y + h4.z + h4.w + h2_1.x + h2_1.y + h3.x + h3.y + h3.z + d1;
}