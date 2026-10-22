// RUN: %dxilver 1.2 | %dxc -E main -T ps_6_2 -enable-16bit-types %s | FileCheck %s

// CHECK: Use native low precision
// CHECK: cbuffer Foo
// CHECK: {
// CHECK:   struct Foo
// CHECK:   {
// CHECK:       half f_h1;                                    ; Offset:    0
// CHECK:       float3 f_f3;                                  ; Offset:    4

// CHECK:       half2 f_h2;                                   ; Offset:   16
// CHECK:       float3 f_f3_1;                                ; Offset:   20

// CHECK:       float2 f_f2;                                  ; Offset:   32
// CHECK:       half4 f_h4;                                   ; Offset:   40

// CHECK:       half2 f_h2_1;                                 ; Offset:   48
// CHECK:       half3 f_h3;                                   ; Offset:   52

// CHECK:       double f_d1;                                  ; Offset:   64
// CHECK:       half3 f_h3_1;                                 ; Offset:   72

// CHECK:       int f_i1;                                     ; Offset:   80
// CHECK:       double f_d2;                                  ; Offset:   88
// CHECK:   } Foo;                                            ; Offset:    0 Size:    96
// CHECK: }

cbuffer Foo {
  half f_h1;
  float3 f_f3;

  half2 f_h2;
  float3 f_f3_1;

  float2 f_f2;
  half4 f_h4;

  half2 f_h2_1;
  half3 f_h3;

  double f_d1;
  half3 f_h3_1;

  int    f_i1;
  double f_d2;
}

// CHECK: cbuffer Bar
// CHECK: {
// CHECK:   struct Bar
// CHECK:   {
// CHECK:       half b_h1;                                    ; Offset:    0
// CHECK:       half b_h2;                                    ; Offset:    2
// CHECK:       half b_h3;                                    ; Offset:    4
// CHECK:       half2 b_h4;                                   ; Offset:    6
// CHECK:       half3 b_h5;                                   ; Offset:   10

// CHECK:       half3 b_h6;                                   ; Offset:   16
// CHECK:       half4 b_h7;                                   ; Offset:   22
// CHECK:       half b_h8;                                    ; Offset:   30

// CHECK:       half4 b_h9;                                   ; Offset:   32
// CHECK:       half3 b_h10;                                  ; Offset:   40

// CHECK:       half2 b_h11;                                  ; Offset:   48
// CHECK:       half3 b_h12;                                  ; Offset:   52
// CHECK:       half2 b_h13;                                  ; Offset:   58


// CHECK:       half b_h14;                                   ; Offset:   62
// CHECK:       half b_h16;                                   ; Offset:   64
// CHECK:       half b_h17;                                   ; Offset:   66
// CHECK:       half b_h18;                                   ; Offset:   68
// CHECK:       half b_h19;                                   ; Offset:   70
// CHECK:       half b_h20;                                   ; Offset:   72
// CHECK:       half b_h21;                                   ; Offset:   74
// CHECK:       half b_h22;                                   ; Offset:   76
// CHECK:       half b_h23;                                   ; Offset:   78

// CHECK:   } Bar;                                            ; Offset:    0 Size:    80
// CHECK: }

cbuffer Bar {
  half b_h1;
  half b_h2;
  half b_h3;
  half2 b_h4;
  half3 b_h5;

  half3 b_h6;
  half4 b_h7;
  half b_h8;

  half4 b_h9;
  half3 b_h10;

  half2 b_h11;
  half3 b_h12;
  half2 b_h13;
  half  b_h14;

  half b_h16;
  half b_h17;
  half b_h18;
  half b_h19;
  half b_h20;
  half b_h21;
  half b_h22;
  half b_h23;

}

// CHECK: %dx.types.CBufRet.f16.8 = type { half, half, half, half, half, half, half, half }

float4 main() : SV_Target  {
  return f_h1 + f_f3.x
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %Foo_cbuffer, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 0
  // CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %Foo_cbuffer, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f32 {{%[0-9]+}}, 1
  + f_h2.x + f_h2.y + f_f3_1.z
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %Foo_cbuffer, i32 1)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 1
  // CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %Foo_cbuffer, i32 1)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f32 {{%[0-9]+}}, 3
  + f_f2.x + f_h4.x + f_h4.y + f_h4.z + f_h4.w
  // CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %Foo_cbuffer, i32 2)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f32 {{%[0-9]+}}, 0
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %Foo_cbuffer, i32 2)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 6
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 7
  + f_h2_1.x + f_h2_1.y + f_h3.x + f_h3.y + f_h3.z
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %Foo_cbuffer, i32 3)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 4
  + f_d1 + f_h3_1.x
  // CHECK: call %dx.types.CBufRet.f64 @dx.op.cbufferLoadLegacy.f64(i32 59, %dx.types.Handle %Foo_cbuffer, i32 4)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f64 {{%[0-9]+}}, 0
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %Foo_cbuffer, i32 4)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 4
  + f_i1 + f_d2
  // CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Foo_cbuffer, i32 5)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.i32 {{%[0-9]+}}, 0
  // CHECK: call %dx.types.CBufRet.f64 @dx.op.cbufferLoadLegacy.f64(i32 59, %dx.types.Handle %Foo_cbuffer, i32 5)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f64 {{%[0-9]+}}, 1
  + b_h1 + b_h2 + b_h3 + b_h4.x + b_h5.y + b_h5.x + b_h5.y + b_h5.z +
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %Bar_cbuffer, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 6
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 7
  + b_h6.x + b_h6.y + b_h6.z + b_h7.x + b_h7.y + b_h7.z + b_h7.w + b_h8
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %Bar_cbuffer, i32 1)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 6
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 7
  + b_h9.x + b_h9.y + b_h9.z + b_h9.w + b_h10.x + b_h10.y + b_h10.z
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %Bar_cbuffer, i32 2)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 6
  + b_h11.x + b_h11.y + b_h12.x + b_h12.y + b_h12.z + b_h13.x + b_h13.y + b_h14
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %Bar_cbuffer, i32 3)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 6
  + b_h16 + b_h17 + b_h18 + b_h19 + b_h20 + b_h21 + b_h22 + b_h23;
  // CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %Bar_cbuffer, i32 4)  ; CBufferLoadLegacy(handle,regIndex)
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 0
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 1
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 2
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 3
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 4
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 5
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 6
  // CHECK: extractvalue %dx.types.CBufRet.f16.8 {{%[0-9]+}}, 7
}